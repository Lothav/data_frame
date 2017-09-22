#include <iostream>
#include <cstring>
#include <vector>
#include "HandleErrors.h"

struct Frame {
    char sync1[8];
    char sync2[8];
    char checksum[4];
};

int main(int argc, char **argv) {

    if(argc != 6) {

        return EXIT_FAILURE;
    }
    struct Frame *frame = (Frame *) malloc(sizeof(Frame));

    memcpy(frame->sync1, "dcc023c2", sizeof(frame->sync1));
    memcpy(frame->sync2, "dcc023c2", sizeof(frame->sync2));

    return 0;
}