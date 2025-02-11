#include "controller.h"
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct controller_list open_controllers() {

        printf("Searching for USB connections...\n");

        uint8_t endpoint_address = 0;
        struct controller_list devices;
        libusb_device **devs;
        struct libusb_device_descriptor desc;
        struct libusb_device_handle *controller = NULL;
        ssize_t num_devs;



        // Boot libusb library
        if (libusb_init(NULL) != 0) {
                printf("\nERROR: libusb failed to boot");
                exit(1);
        }

        if  ((num_devs = libusb_get_device_list(NULL, &devs)) < 0) {
                printf("\nERROR: no controllers found");
                exit(1);
        }

        //printf("Detected %d devices...\n", num_devs);
        int connection_count = 0;
        for (int i = 0; i < num_devs; i++) {

                libusb_device *dev = devs[i];

                if (libusb_get_device_descriptor(dev, &desc) < 0) {
                        printf("\nERROR: bad device descriptor.");
                        exit(1);
                }

                // Our controllers have idProduct of 17
                if (desc.idProduct == 0xe401) {

                        //printf("FOUND: idProduct-%d ", desc.idProduct);
                        struct libusb_config_descriptor *config;
                        if ((libusb_get_config_descriptor(dev, 0, &config)) < 0) {
                                printf("\nERROR: bad config descriptor.");
                                exit(1);
                        }
                        //printf("interfaces-%d\n", config->bNumInterfaces);

                        // Our controllers only have a single interface, no need for looping
                        // This interface also only has one .num_altsetting, no need for looping

                        int r;
                        const struct libusb_interface_descriptor *inter = config->interface[0].altsetting;
                        if ((r = libusb_open(dev, &controller)) != 0) {
                                printf("\nERROR: couldn't open controller");
                                exit(1);
                        }
                        if (libusb_kernel_driver_active(controller, 0)) {
                                libusb_detach_kernel_driver(controller, 0);
                        }
                        libusb_set_auto_detach_kernel_driver(controller, 0);
                        if ((r = libusb_claim_interface(controller, 0)) != 0) {
                                printf("\nERROR: couldn't claim controller.");
                                exit(1);
                        }

                        endpoint_address = inter->endpoint[0].bEndpointAddress;
                        connection_count++;

                        if (connection_count == 1) {
                                devices.device1 = controller;
                                devices.device1_addr = endpoint_address;
                        } else {
                                devices.device2 = controller;
                                devices.device2_addr = endpoint_address;
                                //printf("%d:%d,%d:%d\n",devices.device1,devices.device1_addr,devices.device2,devices.device2_addr)                                goto found;
                        }
                }
        }

        if (connection_count < 2) {
                printf("ERROR: couldn't find 2 controllers.");
                exit(1);
        }

        found:
                printf("Connected %d controllers!\n", connection_count);
                libusb_free_device_list(devs, 1);

        return devices;
}

void detect_presses(struct controller_pkt pkt1, struct controller_pkt pkt2, control_info_t *control_info, int print) {

        // Choose whether you want human-readable or binary output
        if (pkt1.codes[0] == 0x7f && pkt1.codes[1] == 0xff && pkt1.codes[2] == 0x00) { //Player0 press Down direction
                control_info -> idle0 = 0;
                if (control_info -> direction0 == DOWN)
                        control_info -> press_tick0 ++;
                        
                else {
                        control_info -> direction0 = DOWN;
                        control_info -> press_tick0 = 0;
                }
                if (print)
                        printf("Player 0: Down\n");
        } else if (pkt1.codes[0] == 0x00 && pkt1.codes[1] == 0x7f && pkt1.codes[2] == 0x00) {
                control_info -> idle0 = 0;
                if (control_info -> direction0 == LEFT)
                        control_info -> press_tick0 ++;
                else {
                        control_info -> direction0 = LEFT;
                        control_info -> press_tick0 = 0;
                }
                if (print)
                        printf("Player 0: Left\n");
        } else if (pkt1.codes[0] == 0x7f && pkt1.codes[1] == 0x00 && pkt1.codes[2] == 0x00) {
                control_info -> idle0 = 0;
                if (control_info -> direction0 == UP)
                        control_info -> press_tick0 ++;
                else {
                        control_info -> direction0 = UP;
                        control_info -> press_tick0 = 0;
                }
                if (print)
                        printf("Player 0: Up\n");
        } else if (pkt1.codes[0] == 0xff && pkt1.codes[1] == 0x7f && pkt1.codes[2] == 0x00) {
                control_info -> idle0 = 0;
                if (control_info -> direction0 == RIGHT)
                        control_info -> press_tick0 ++;
                else {
                        control_info -> direction0 = RIGHT;
                        control_info -> press_tick0 = 0;
                }
                if (print)
                        printf("Player 0: Right\n");
        } else {
                control_info -> press_tick0 = 0;
                control_info -> idle0 = 1;
        }
        if (pkt2.codes[0] == 0x7f && pkt2.codes[1] == 0xff && pkt2.codes[2] == 0x00) { //Player1 press Down direction
                control_info -> idle1 = 0;
                if (control_info -> direction1 == DOWN)
                        control_info -> press_tick1 ++;
                        
                else {
                        control_info -> direction1 = DOWN;
                        control_info -> press_tick1 = 0;
                }
                if (print)
                        printf("Player 1: Down\n");
        } else if (pkt2.codes[0] == 0x00 && pkt2.codes[1] == 0x7f && pkt2.codes[2] == 0x00) {
                control_info -> idle1 = 0;
                if (control_info -> direction1 == LEFT)
                        control_info -> press_tick1 ++;
                else {
                        control_info -> direction1 = LEFT;
                        control_info -> press_tick1 = 0;
                }
                if (print)
                        printf("Player 1: Left\n");
        } else if (pkt2.codes[0] == 0x7f && pkt2.codes[1] == 0x00 && pkt2.codes[2] == 0x00) {
                control_info -> idle1 = 0;
                if (control_info -> direction1 == UP)
                        control_info -> press_tick1 ++;
                else {
                        control_info -> direction1 = UP;
                        control_info -> press_tick1 = 0;
                }
                if (print)
                        printf("Player 1: Up\n");
        } else if (pkt2.codes[0] == 0xff && pkt2.codes[1] == 0x7f && pkt2.codes[2] == 0x00) {
                control_info -> idle1 = 0;
                if (control_info -> direction1 == RIGHT)
                        control_info -> press_tick1 ++;
                else {
                        control_info -> direction1 = RIGHT;
                        control_info -> press_tick1 = 0;
                }
                if (print)
                        printf("Player 1: Right\n");
        } else {
                control_info -> press_tick1 = 0;
                control_info -> idle1 = 1;
        }
        if (pkt1.codes[5] == 0x2f) {
                control_info -> attempt_place_bomb_0 = 1;
                if (print)
                        printf("Player 0: Place bomb!\n");
        } else
                control_info -> attempt_place_bomb_0 = 0;
        if (pkt2.codes[5] == 0x2f) {
                control_info -> attempt_place_bomb_1 = 1;
                if (print)
                        printf("Player 1: Place bomb!\n");
        } else
                control_info -> attempt_place_bomb_1 = 0;
        

}

void *listen_controllers(void *arg) {

        struct args_list *args_p = arg;
        struct args_list args = *args_p;
        struct controller_list devices = args.devices;

        struct controller_pkt pkt1, pkt2;
        int fields1, fields2;
        int size1 = sizeof(pkt1);
        int size2 = sizeof(pkt2);
        for (;;) {

                libusb_interrupt_transfer(devices.device1, devices.device1_addr, (unsigned char *) &pkt1, size1, &fields1, 0);
                libusb_interrupt_transfer(devices.device2, devices.device2_addr, (unsigned char *) &pkt2, size2, &fields2, 0);

                // 7 fields should be transferred for each packet
                if (fields1 == 7 && fields2 == 7) {
                        detect_presses(pkt1, pkt2, args.control_info, args.print);
                        usleep(20000);

                }
                
        }
}
