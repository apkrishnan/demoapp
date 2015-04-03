#include "gpio.h"
#include "json.h"
#include "proton/message.h"
#include "proton/messenger.h"
#include "pncompat/misc_funcs.inc"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

// GPIO used for Button.
#define BTN_GPIO    49

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

    // Initialize and start the messenger.
    // Set to be non-blocking mode with outgoing window size of 1024.
    messengerPtr = pn_messenger(NULL);
    pn_messenger_set_blocking(messengerPtr, false);
    pn_messenger_set_outgoing_window(messengerPtr, 1024); 
    pn_messenger_start(messengerPtr);

    // Configure Button GPIO to be input pin.
    gpio_export(BTN_GPIO);
    gpio_set_dir(BTN_GPIO, INPUT_PIN);

    while(1) {

#ifndef USE_HW_PUSHBTN
        // Wait for user key press.
        getchar();
#else
        unsigned int pinStat = LOW;
        do {
            gpio_get_value(BTN_GPIO, &pinStat);
            usleep(1000);
        } while(pinStat != HIGH);
#endif // USE_HW_PUSHBTN

        // Create JSON message string.
        Json::Value valObj;
        static const Json::StaticString sourceKey("Source");
        valObj[sourceKey] = std::string("Beagle");

        static const Json::StaticString timeKey("Time");
        time_t t = time(NULL);
        valObj[timeKey] = std::string(asctime(gmtime(&t)));

        Json::FastWriter jsonWriter;
        std::string jsonFile = jsonWriter.write(valObj);

        // Initiatlze message data pointer.
        pn_message_t* msgPtr = pn_message();
        pn_message_set_address(msgPtr, argv[1]);
        pn_data_t* msgBodyPtr = pn_message_body(msgPtr);
        pn_data_put_string(msgBodyPtr, pn_bytes(jsonFile.length(), jsonFile.c_str()));
    
        // Put new message into messenger.
        pn_messenger_put(messengerPtr, msgPtr);
        check(messengerPtr);

        // Track the message send status.
        pn_tracker_t msgTracker = pn_messenger_outgoing_tracker(messengerPtr);
        pn_status_t msgStatus;
        do {
            pn_messenger_work(messengerPtr, -1);
            msgStatus = pn_messenger_status(messengerPtr, msgTracker);
        } while(msgStatus == PN_STATUS_PENDING);

        // Free up message pointer.
        pn_message_free(msgPtr);

        printf("Sent new message to %s:\n%s", argv[1], jsonFile.c_str());
    }

    return 0;
}
