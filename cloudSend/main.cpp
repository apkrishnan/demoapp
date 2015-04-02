#include "gpio.h"
#include "json.h"
#include "proton/message.h"
#include "proton/messenger.h"
#include "pncompat/misc_funcs.inc"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

pn_message_t* message;
pn_messenger_t* messenger;

#define check(messenger)                                                       \
  do {                                                                         \
     if(pn_messenger_errno(messenger))                                         \
        die(__FILE__, __LINE__, pn_error_text(pn_messenger_error(messenger))); \
  } while(0)                                                                   \

void die(const char* file, int line, const char* message) {
    fprintf(stderr, "%s:%i: %s\n", file, line, message);
    exit(1);
}

int main(void) {

    return 0;
}
