/*
 
    File: SMJobBlessHelper.c
Abstract: A helper tool that doesn't do anything event remotely interesting.
See the ssd sample for how to use GCD and launchd to set up an on-demand
server via sockets.
 Version: 1.2

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may
be used to endorse or promote products derived from the Apple Software
without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or
implied, are granted by Apple herein, including but not limited to any
patent rights that may be infringed by your derivative works or by other
works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2011 Apple Inc. All Rights Reserved.


*/

// Modified based on SampleD

#include <syslog.h>
#include <unistd.h>
#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <launch.h>

#import "../SMJobBless.h"

#define MAX_PATH_SIZE 128

/*
    returns -1 for error, else file descriptor for listener
*/
int get_listener_fd() {
    launch_data_t checkin_request = launch_data_new_string(LAUNCH_KEY_CHECKIN);
    if (!checkin_request) {
        syslog(LOG_NOTICE, "Unable to create checkin string!");
        return -1;
    }
    launch_data_t checkin_response = launch_msg(checkin_request);
    if (!checkin_response) {
        syslog(LOG_NOTICE, "Unable to do checkin!");
        return -1;
    }
    if (LAUNCH_DATA_ERRNO == launch_data_get_type(checkin_response)) {
        errno = launch_data_get_errno(checkin_response);
        syslog(LOG_NOTICE, "Error %d getting type of checkin response!", errno);
        return -1;
    }
    launch_data_t the_label = launch_data_dict_lookup(checkin_response, LAUNCH_JOBKEY_LABEL);
    if (!the_label) {
        syslog(LOG_NOTICE, "No Label for job!");
        return -1;
    }    
    launch_data_t sockets_dict = launch_data_dict_lookup(checkin_response, LAUNCH_JOBKEY_SOCKETS);
    if (!sockets_dict) {
        syslog(LOG_NOTICE, "No socket found to answer requests on!");
        return -1;
    }
    size_t count = launch_data_dict_get_count(sockets_dict);
    if (count < 1) {
        syslog(LOG_NOTICE, "No socket found to answer requests on!");
        return -1;
    }
    if (1 < count) {
        syslog(LOG_NOTICE, "Some socket(s) will be ignored!");
    }
    launch_data_t listening_fd_array = launch_data_dict_lookup(sockets_dict, "MasterSocket");
    if (!listening_fd_array) {
        syslog(LOG_NOTICE, "MasterSocket not found!");
        return -1;
    }
    count = launch_data_array_get_count(listening_fd_array);
    if (count < 1) {
        syslog(LOG_NOTICE, "No socket found to answer requests on!");
        return -1;
    }
    if (1 < count) {
        syslog(LOG_NOTICE, "Some socket(s) will be ignored!");
    }
    launch_data_t this_listening_fd = launch_data_array_get_index(listening_fd_array, 0);
    int listener_fd = launch_data_get_fd(this_listening_fd);
    if ( listener_fd == -1 ) {
        syslog(LOG_NOTICE, "launch_data_get_fd() failed!");
        return -1;
    }
    if (listen(listener_fd, 5)) {
        syslog(LOG_NOTICE, "listen() failed with %i", errno);
        return -1;
    }
    return listener_fd;
}

/*
    returns -2 for error, -1 for no connection, else file descriptor for connection
*/
int get_connection_fd(int listener_fd) {
    unsigned int size = sizeof(struct sockaddr) + MAX_PATH_SIZE;
    char address_data[size];
    struct sockaddr* address = (struct sockaddr*) &address_data;

    struct pollfd fds;
    fds.fd = listener_fd;
    fds.events = POLLIN;
    int readyCount = poll(&fds, 1, 10000);  // wait ten seconds for a connection
    if (readyCount == -1) {
        syslog(LOG_NOTICE, "poll() error = %d\n", errno);
        return -2;
    }
    if (!readyCount) return -1;
    
    int connection_fd = accept(listener_fd, address, &size);
    if (connection_fd < 0) {
        syslog(LOG_NOTICE, "accept() returned %i; error = %i!", connection_fd, errno);
        return -2;
    }
    return connection_fd;
}

int respondToRequests() {
    int listener_fd = get_listener_fd();
    if (listener_fd == -1) return 1;
    int connection_fd;
    while (0 <= (connection_fd = get_connection_fd(listener_fd))) {
        struct SMJobBlessMessage messageIn, messageOut;
        if (readMessage(connection_fd, &messageIn)) break;
        initMessage(messageOut, messageIn.command);
        switch (messageIn.command) {
            case SMJobBless_Version:
                messageOut.dataSize = 3;
                messageOut.data[0] = kVersionPart1;
                messageOut.data[1] = kVersionPart2;
                messageOut.data[2] = kVersionPart3;
                break;
                
            case SMJobBless_PID: {
                int pid = getpid();
                messageOut.dataSize = sizeof(pid);
                memcpy(messageOut.data, &pid, messageOut.dataSize);
                break;
            }
            default:
                syslog(LOG_NOTICE, "Unknown command: %hhd\n", messageIn.command);
                char* message = "Unknown command!";
                messageOut.command = SMJobBless_Error;
                messageOut.dataSize = strlen(message) + 1;    // add trailing \0
                strcpy((char *) messageOut.data, message);
                break;
        }
        int count = messageSize(&messageOut);
        int written = write(connection_fd, &messageOut, count);
        if (written != count) {
            syslog(LOG_NOTICE, "tried to write %i, but wrote %i", count, written);
            break;
        }
        close(connection_fd);
    }
    close(listener_fd);
    if (0 < connection_fd) close(connection_fd);
	return connection_fd == -1 ? 0 : 1;
}

int main(int argc, const char * argv[]) {
    syslog(LOG_NOTICE, "SMJobBlessHelper: uid = %d, euid = %d, pid = %d\n", getuid(), geteuid(), getpid());
    return respondToRequests();
}

