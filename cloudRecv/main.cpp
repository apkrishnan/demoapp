#include "gpio.h"
#include "json.h"
#include "proton/message.h"
#include "proton/messenger.h"
#include "pncompat/misc_funcs.inc"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// GPIO used for LED.
#define LED_GPIO    115

// Qpid-proton Messenger.
pn_messenger_t* messengerPtr;

#define check(ptr)                                                       \
  do {                                                                   \
     if(pn_messenger_errno(ptr))                                         \
        die(__FILE__, __LINE__, pn_error_text(pn_messenger_error(ptr))); \
  } while(0)

void die(const char* f_fileName, int f_line, const char* f_message) {
    fprintf(stderr, "%s:%i: %s\n", f_fileName, f_line, f_message);
    exit(1);
}

int main(int argc, char** argv) {

    if(argc < 2) 
        die(__FILE__, __LINE__, "Specify the Azure queue path");

    // Configure LED GPIO to be output pin.
    gpio_export(LED_GPIO);
    gpio_set_dir(LED_GPIO, OUTPUT_PIN);
    gpio_set_value(LED_GPIO, LOW);

    while(1) {

        // Initialize and set the messager incoming window to be 1.
        messengerPtr = pn_messenger(NULL);
        pn_messenger_set_incoming_window(messengerPtr, 1);
        check(messengerPtr);

        // Start the messenger.
        pn_messenger_start(messengerPtr);
        check(messengerPtr);

        // Subscribe messenger to the Azure Service bus path.
        pn_messenger_subscribe(messengerPtr, argv[1]);
        check(messengerPtr);

        // Prepare message data.
        pn_message_t* msgPtr = pn_message();

        // Loop until message receive timeout.
        while(1) {

            // Wait to receive at least one message.
            pn_messenger_recv(messengerPtr, 1);
            if(pn_messenger_errno(messengerPtr))
                break;

            // Process all new incoming messages.
            while(pn_messenger_incoming(messengerPtr)) {

                // Get new message out from messager.
                pn_messenger_get(messengerPtr, msgPtr);
                check(messengerPtr);

                // Retrieve message string.
                const char* subjStr = pn_message_get_subject(msgPtr);

                // Retrieve data packet.
                char datBuf[2048];
                size_t datBufSz = sizeof(datBuf);
                pn_data_t* msgBodyPtr = pn_message_body(msgPtr);
                pn_data_format(msgBodyPtr, datBuf, &datBufSz);

                // Mark message is accepted on tracker.
                pn_tracker_t msgTracker = pn_messenger_incoming_tracker(messengerPtr);
                pn_messenger_accept(messengerPtr, msgTracker, PN_CUMULATIVE);

                // Printout for diagnostics.
                printf("New message recieved from %s\n", argv[1]);
                printf("Address: %s\n", pn_message_get_address(msgPtr));
                printf("Suject: %s\n", subjStr ? subjStr : "(no subject)");
                printf("Content: %s\n\n", datBuf);

                // Blink LED twice.
                gpio_set_value(LED_GPIO, HIGH);
                usleep(200000);
                gpio_set_value(LED_GPIO, LOW);
                usleep(200000);
                gpio_set_value(LED_GPIO, HIGH);
                usleep(200000);
                gpio_set_value(LED_GPIO, LOW);
            }
        }

        // Free up resources to restart.
        pn_messenger_stop(messengerPtr);
        pn_messenger_free(messengerPtr);
        pn_message_free(msgPtr); 
    }

    return 0;
}
