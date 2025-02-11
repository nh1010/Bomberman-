#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "controller.h"
#include "bomberman.h"
extern int vga_fd;
extern game_info_t global_info;
extern player_info_t player0_info;
extern player_info_t player1_info;
extern control_info_t control_info;
extern uint16_t *map;
extern struct controller_list controllers;
extern struct args_list c_args_list;

int main()
{
    controllers = open_controllers();
    pthread_t control_thread;
    memset((void *) &control_info, 0, sizeof(control_info_t));
    c_args_list.mode = 0;
    c_args_list.print = 1;
    c_args_list.devices = controllers;
    c_args_list.control_info = &control_info;
    
    
    if (pthread_create(&control_thread, NULL, &listen_controllers, (void *) &c_args_list)) {
        fprintf(stderr, "Could not create controller thread\n");
        return -1;
    }
    static const char filename[] = "/dev/vga_ball";
    printf("VGA ball Userspace program started\n");
    
    if ( (vga_fd = open(filename, O_RDWR)) == -1) {
        fprintf(stderr, "could not open %s\n", filename);
        return -1;
    }
    memset((void *) &global_info, 0, sizeof(game_info_t));
    generate_software_map();
    init_players();
    /*for (int i = 0; i < 10000; i ++){
        xpos0 += v0;
        xpos1 += v1;
        pos_counter ++;
        pos_counter = pos_counter % 18;
        if (xpos0 > PLAYER_X_UPPER_LIM) {
            xpos0 = PLAYER_X_UPPER_LIM;
            facing0 = LEFT;
            v0 = -1;
        }
        if (xpos0 < PLAYER_X_LOWER_LIM) {
            xpos0 = PLAYER_X_LOWER_LIM;
            facing0 = RIGHT;
            v0 = 1;
        }
        if (xpos1 > PLAYER_X_UPPER_LIM) {
            xpos1 = PLAYER_X_UPPER_LIM;
            facing1 = LEFT;
            v1 = -1;
        }
        if (xpos1 < PLAYER_X_LOWER_LIM) {
            xpos1 = PLAYER_X_LOWER_LIM;
            facing1 = RIGHT;
            v1 = 1;
        }
        if (pos_counter / 6 == 0)
            pos = SIDE0;
        else if (pos_counter / 6 == 1)
            pos = SIDE1;
        else pos = SIDE2;
            
        set_player_status(xpos0, 136, facing0, MOVING, pos, PLAYER0);
        set_player_status(xpos1, 400, facing1, MOVING, pos, PLAYER1);
        write_player_info();
        pass_game_info();
        usleep(20000);
    }*/
    pthread_join(control_thread, NULL);
    free(map);
    return 0;
}
