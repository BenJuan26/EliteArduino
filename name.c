// Based on examples\Teensy\USB_MIDI\MIDI_name

#include "usb_names.h"

#define DEVICE_NAME   {'E','l','i','t','e',' ','D','a','n','g','e','r','o','u','s',' ','C','o','c','k','p','i','t'}
#define DEVICE_NAME_LEN  23

// Do not change this part.  This exact format is required by USB.
struct usb_string_descriptor_struct usb_string_product_name = {
        2 + DEVICE_NAME_LEN * 2,
        3,
        DEVICE_NAME
};
