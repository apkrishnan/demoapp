#include "gpio.h"
#include "json.h"
#include "proton/message.h"
#include "proton/messenger.h"
#include "pncompat/misc_funcs.inc"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char* azurePathStr = (char*)"amqps://";

pn_message_t* msgPtr;
pn_messenger_t* messengerPtr;

#define check(messengerPtr)                                                       \
  do {                                                                            \
     if(pn_messenger_errno(messengerPtr))                                         \
        die(__FILE__, __LINE__, pn_error_text(pn_messenger_error(messengerPtr))); \
  } while(0)                                                                      \

void die(const char* f_fileName, int f_line, const char* f_message) {
    fprintf(stderr, "%s:%i: %s\n", f_fileName, f_line, f_message);
    exit(1);
}

int main(void) {

	// Initiatlze message data pointer.
	msgPtr = pn_message();

	// Initialize and set messager to be non-blocking mode.
	messengerPtr = pn_messenger(NULL);
	pn_messenger_set_blocking(messengerPtr, false);

	// Start the messenger.
	pn_messenger_start(messengerPtr);
	check(messengerPtr);

	// Subscribe messenger to the Azure Service bus path.
	pn_messenger_subscribe(messengerPtr, azurePathStr);
	check(messengerPtr);

	// Set to receive as many messages as messenger can buffer.
	pn_messenger_recv(messengerPtr, -1);

	// Main application loop.
	while(1) {
		// Block indefinitely until there has been socket activity.
		pn_messenger_work(messengerPtr, -1);

		// TODO: process the message.
	}

	return 0;
}
