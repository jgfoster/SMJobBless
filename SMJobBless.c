//
//  SMJobBless.c
//

#include "SMJobBless.h"
#include <errno.h>
#include <poll.h>
#include <syslog.h>
#include <unistd.h>

/*
 return 0 for success, 1 for failure
 */
int readBytes(int size, int fd, unsigned char * buffer) {
    int left = size;
    unsigned char* pointer = buffer;
    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    
    while (0 < left) {
        int readyCount = poll(&fds, 1, 1000);  // wait one second for data
        if (readyCount == -1) {
            syslog(LOG_NOTICE, "poll() error = %d\n", errno);
            return 1;
        }
        if (readyCount == 0) {
            syslog(LOG_NOTICE, "no bytes available on socket!");
            return 1;
        }
        int numberRead = read(fd, pointer, left);
        if (numberRead == 0) {
            syslog(LOG_NOTICE, "poll() said that data was available but we didn't get any!");
            return 1;
        }
        left -= numberRead;
        pointer += numberRead;
    }
    return 0;
}

int readMessage(int fd, unsigned char * buffer) {
    unsigned char version;
    if (readBytes(1, fd, &version)) return 1;
    if (version != kMessageVersion) {
        syslog(LOG_NOTICE, "expected message format version %i but got %i", kMessageVersion, version);
        return 1;
    }
    unsigned char size;
    if (readBytes(1, fd, &size)) return 1;
    return readBytes(size, fd, buffer);
}



