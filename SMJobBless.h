//
//  SMJobBless.h
//

#ifndef SMJobBless_SMJobBless_h
#define SMJobBless_SMJobBless_h

#define kSocketPath "/var/run/com.apple.SMJobBlessHelper.socket"
#define kHelperIdentifier "com.apple.bsd.SMJobBlessHelper"
#define kVersionPart1 1
#define kVersionPart2 0
#define kVersionPart3 47

enum SMJobBlessCommand {
    SMJobBless_Error    = 0,
    SMJobBless_Version  = 1,
    SMJobBless_PID      = 2,
};

// This needs to change if the following structure changes
#define kMessageVersion 1

struct SMJobBlessMessage {
    unsigned char version;      // kMessageVersion
    unsigned char command;      // SMJobBlessCommand
    unsigned char dataSize;     // 0 to 252
    unsigned char data[252];    // command-specific data
};

#define messageSize(message_p) sizeof(*message_p) - sizeof((message_p)->data) + (message_p)->dataSize
#define initMessage(m, c) { m.version = kMessageVersion; m.command = c; m.dataSize = 0; }

int readMessage(int fd, struct SMJobBlessMessage * message);

#endif
