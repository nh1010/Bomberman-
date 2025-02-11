#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include "bomberman.h"

struct controller_list {

        struct libusb_device_handle *device1;
        struct libusb_device_handle *device2;
        uint8_t device1_addr;
        uint8_t device2_addr;

};

struct controller_pkt {

        uint8_t codes[7];

};

struct args_list {

        struct controller_list devices;
        char buttons[11];
        int mode;
        int print;
        control_info_t *control_info;
};

struct controller_list open_controllers();
void *listen_controllers(void *arg);

#endif
