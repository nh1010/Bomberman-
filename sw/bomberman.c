#include <unistd.h>
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
int vga_fd;
game_info_t global_info;
player_info_t player0_info;
player_info_t player1_info;
control_info_t control_info;
uint16_t *map;
struct controller_list controllers;
struct args_list c_args_list;

void set_player_status (uint16_t vxpos, uint16_t vypos, enum FACING facing, enum STAT status, enum POSE pose, uint16_t pos_tick, uint16_t vspeed, uint16_t bomb_range, uint16_t max_bombs, uint16_t bombs_left, uint16_t dead, enum PLAYER player)
{
    player_info_t *target_info;
    if (player == PLAYER0)
        target_info = &player0_info;
    else
        target_info = &player1_info;
    target_info->vxpos = vxpos;
    target_info->vypos = vypos;
    target_info->facing = facing;
    target_info->status = status;
    target_info->pose = pose;
    target_info->pos_tick = pos_tick;
    target_info->vspeed = vspeed;
    target_info->bomb_range = bomb_range;
    target_info->max_bombs = max_bombs;
    target_info->bombs_left = bombs_left;
    target_info->dead = dead;
}

void init_players()
{
    set_player_status (PLAYER0_INIT_X * 3, PLAYER0_INIT_Y * 3, DOWN, STATIC, IDLE, 0, 1, 1, 1, 1, 0, PLAYER0);
    set_player_status (PLAYER1_INIT_X * 3, PLAYER1_INIT_Y * 3, DOWN, STATIC, IDLE, 0, 1, 1, 1, 1, 0, PLAYER1);
}


void write_player_info()
{
    uint16_t pos = 0;
    global_info.playerinfo00 = player0_info.vxpos / 3;
    if (player0_info.status == MOVING)
        SET_BIT(global_info.playerinfo00, 10);
    if (player0_info.pose == SIDE1 || player0_info.pose == DOWN1 || player0_info.pose == UP1)
            pos = 1;
    else if (player0_info.pose == SIDE2)
            pos = 2;
    SET_BITS(global_info.playerinfo00, 11, pos);
    global_info.playerinfo01 = player0_info.vypos / 3;
    SET_BITS(global_info.playerinfo01, 9, player0_info.facing);


    global_info.playerinfo10 = player1_info.vxpos / 3;
    if (player1_info.status == MOVING)
        SET_BIT(global_info.playerinfo10, 10);
    pos = 0;
    if (player1_info.pose == SIDE1 || player1_info.pose == DOWN1 || player1_info.pose == UP1)
            pos = 1;
    else if (player1_info.pose == SIDE2)
            pos = 2;
    SET_BITS(global_info.playerinfo10, 11, pos);
    global_info.playerinfo11 = player1_info.vypos / 3;
    SET_BITS(global_info.playerinfo11, 9, player1_info.facing);
}

void write_tile(uint16_t tile_pos, uint16_t tile_type, unsigned short *pos)
{
    *pos = tile_pos;
    SET_BITS(*pos, 11, tile_type);
    SET_BIT(*pos, 15);
}

void pass_game_info()
{
    vga_ball_arg_t vla;
    vla.background= global_info;
    if (ioctl(vga_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
        perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
        return;
    }
}

uint16_t manhattan_distance(uint16_t x_0, uint16_t y_0, uint16_t x_1, uint16_t y_1)
{
    uint16_t x_diff;
    if (x_0 < x_1)
        x_diff = x_1 - x_0;
    else
        x_diff = x_0 - x_1;
    uint16_t y_diff;
    if (y_0 < y_1)
        y_diff = y_1 - y_0;
    else
        y_diff = y_0 - y_1;
    return x_diff + y_diff;

}

void generate_software_map()
{
    time_t t;
    srand((unsigned) time(&t));
    write_player_info();
    pass_game_info();
    map = (uint16_t *) malloc (MAP_SIZE * sizeof(uint16_t));
    uint16_t row_p0 = 10;
    uint16_t col_p0 = 10;
    uint16_t row_p1 = 20;
    uint16_t col_p1 = 30;
    for (uint16_t i = 0; i < MAP_SIZE; i ++) {
        uint16_t row = i / 40;
        uint16_t col = i % 40;
        if (row % 2 == 1 && col % 2 == 1) {
            map[i] = 1;
            continue;
        }
        if (rand() % 10 < 2 && manhattan_distance(row, col, row_p0, col_p0) > 10 && manhattan_distance(row, col, row_p1, col_p1) > 10) {
            map[i] = 2;
            continue;
        }
        map[i] = 0;
    }
    for (int i = 0; i < 100; i++) {
        write_tile(i * 12, map[i * 12], &global_info.map_change_0);
        write_tile(i * 12 + 1, map[i * 12 + 1], &global_info.map_change_1);
        write_tile(i * 12 + 2, map[i * 12 + 2], &global_info.map_change_2);
        write_tile(i * 12 + 3, map[i * 12 + 3], &global_info.map_change_3);
        write_tile(i * 12 + 4, map[i * 12 + 4], &global_info.map_change_4);
        write_tile(i * 12 + 5, map[i * 12 + 5], &global_info.map_change_5);
        write_tile(i * 12 + 6, map[i * 12 + 6], &global_info.map_change_6);
        write_tile(i * 12 + 7, map[i * 12 + 7], &global_info.map_change_7);
        write_tile(i * 12 + 8, map[i * 12 + 8], &global_info.map_change_8);
        write_tile(i * 12 + 9, map[i * 12 + 9], &global_info.map_change_9);
        write_tile(i * 12 + 10, map[i * 12 + 10], &global_info.map_change_10);
        write_tile(i * 12 + 11, map[i * 12 + 11], &global_info.map_change_11);
        pass_game_info();
        usleep(2000);
    }
}

int is_player_moving(player_info_t *info)
{
    if (info -> status == STATIC)
        return 0;
    else
        return 1;
}

enum FACING get_player_facing(player_info_t *info)
{
    return info -> facing; 
}

void get_player_vpos(uint16_t *pos, player_info_t *info)
{
    pos[0] = info -> vxpos;
    pos[1] = info -> vypos;
}

uint16_t get_player_vspeed(player_info_t *info)
{
    return info -> vspeed;
}

void handle_player_movement()
{
    uint16_t player0_curr_vpos[2];
    uint16_t player1_curr_vpos[2];
    uint16_t player0_attempt_vpos[2];
    uint16_t player1_attempt_vpos[2];
    get_player_vpos(player0_curr_vpos, &player0_info);
    get_player_vpos(player1_curr_vpos, &player1_info);
    memcpy((void *) player0_attempt_vpos, (void *) player0_curr_vpos, 2 * sizeof(uint16_t));
    memcpy((void *) player1_attempt_vpos, (void *) player1_curr_vpos, 2 * sizeof(uint16_t));
    if (control_info.direction0 == get_player_facing(&player0_info) && control_info.press_tick0 > 20) {
        uint16_t player0_vs = get_player_vspeed(&player0_info);
        switch (control_info.direction0) {
            case DOWN:
                player0_attempt_vpos[1] -= player0_vs;
                break;
            case LEFT:
                player0_attempt_vpos[0] -= player0_vs;
                break;
            case UP:
                player0_attempt_vpos[1] += player0_vs;
                break;
            case RIGHT:
                player0_attempt_vpos[0] += player0_vs;
                break;
        }
        uint16_t player0_attempt_xpos = player0_attempt_vpos[0] / 3;
        uint16_t player0_attempt_ypos = player0_attempt_vpos[1] / 3;
        if (player0_attempt_xpos > PLAYER_X_UPPER_LIM || player0_attempt_ypos < PLAYER_X_LOWER_LIM || player0_attempt_ypos > PLAYER_Y_UPPER_LIM || player0_attempt_ypos < PLAYER_Y_LOWER_LIM)
            memcpy((void *) player0_attempt_vpos, (void *) player0_curr_vpos, 2 * sizeof(uint16_t));
        
    }
    else {
        
    }

}