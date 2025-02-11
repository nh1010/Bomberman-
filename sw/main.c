#include "controller.h"
#include <pthread.h>

char buttons[11];

int main() {
    struct controller_list devices = open_controllers();

    struct args_list args;
    args.devices = devices;
    args.mode = 0; // Set mode to 0 for human-readable output
    args.print = 1; // Set print to 1 to print button presses

    // Create a thread for listening to controller input
    pthread_t listener_thread;
    pthread_create(&listener_thread, NULL, listen_controllers, (void*)&args);

    pthread_join(listener_thread, NULL);

    return 0;
}
