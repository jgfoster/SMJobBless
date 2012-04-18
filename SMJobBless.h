//
//  SMJobBless.h
//

#ifndef SMJobBless_SMJobBless_h
#define SMJobBless_SMJobBless_h

#define kSocketPath "/var/run/com.apple.SMJobBlessHelper.socket"
#define kHelperIdentifier "com.apple.bsd.SMJobBlessHelper"
#define kVersionPart1 1
#define kVersionPart2 0
#define kVersionPart3 40

// message format (both directions)
#define kMessageVersion 1
// byte 0: kMessageVersion
// byte 1: remaining size (0-253)
// byte 2: SMJobBlessCommand
// bytes 3-255: command-specific data (if any)

enum SMJobBlessCommand {
    SMJobBless_Error    = 0,
    SMJobBless_Version  = 1,
    SMJobBless_PID      = 2,
};

int readMessage(int fd, unsigned char * buffer);

#endif
