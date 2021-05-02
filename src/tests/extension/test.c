#include "reb-host.h"
#include "host-lib.h"
//#include <stdio.h> 

RL_LIB *RL; // Link back to reb-lib from embedded extensions

const char *init_block =
    "REBOL [\n"
        "Title: {Add two integers}\n"
        "Name: extension-test"
        "Type: module\n"
        "Exports: [addi]\n" // exporting just the first command!
    "]\n"
    "addi:  command [i1 [integer!] i2 [integer!]]\n"
    "addi2: command [i1 [integer!] i2 [integer!]]\n" // this one is not exported
    "bad-args: command [{generic error}]\n"
    "error: command [{throws an error with text}]\n"
;

RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
    RL = lib;
    //printf("RXinit %llu %llu\n", sizeof(REBREQ) , sizeof(REBEVT));
    if (!CHECK_STRUCT_ALIGN) return 0;
    return init_block;
}

RXIEXT int RX_Quit(int opts) {
    return 0;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
    //printf("RX_Call cmd: %i\n", cmd);
    switch(cmd) {
        case 0: 
            RXA_INT64(frm,1) = RXA_INT64(frm, 1) + RXA_INT64(frm, 2);
            break;
        case 1:
            RXA_INT64(frm,1) = 2 * (RXA_INT64(frm, 1) + RXA_INT64(frm, 2));
            break;
        case 2:
            return RXR_BAD_ARGS; //https://github.com/Oldes/Rebol-issues/issues/1867
        case 3:
            RXA_SERIES(frm,1) = "test error!";
            return RXR_ERROR;

    }
    return RXR_VALUE;
}