#ifndef _BOMBERMAN_H
#define _BOMBERMAN_H
#include <stdint.h>
#include "hello.h"
#define PLAYER_X_UPPER_LIM 631
#define PLAYER_X_LOWER_LIM 8
#define PLAYER_Y_UPPER_LIM 471
#define PLAYER_Y_LOWER_LIM 8
#define SET_BIT(x,y) x |= ((uint16_t) 1 << y)
#define SET_BITS(x,y,z) x |= ((uint16_t) z << y)
#define CLEAR_BIT(x,y) x &= ~((uint16_t) 1 << y)
#define MAP_SIZE 1200
#define PLAYER0_INIT_X 168
#define PLAYER0_INIT_Y 167
#define PLAYER1_INIT_X 488
#define PLAYER1_INIT_Y 327


enum FACING {DOWN, LEFT, UP, RIGHT};
enum STAT {STATIC, MOVING};
enum POSE {IDLE, SIDE0, SIDE1, SIDE2, DOWN0, DOWN1, UP0, UP1};
enum PLAYER {PLAYER0, PLAYER1};

typedef struct {
    uint16_t vxpos;
    uint16_t vypos;
    enum FACING facing;
    enum STAT status;
    enum POSE pose;
    uint16_t pos_tick;
    uint16_t vspeed;
    uint16_t bomb_range;
    uint16_t max_bombs;
    uint16_t bombs_left;
    uint16_t dead;
} player_info_t;

typedef struct {
    enum FACING direction0;
    enum FACING direction1;
    unsigned long long press_tick0;
    unsigned long long press_tick1;
    int attempt_place_bomb_0;
    int attempt_place_bomb_1;
    int idle0;
    int idle1;
} control_info_t;



void set_player_status (uint16_t vxpos, uint16_t vypos, enum FACING facing, enum STAT status, enum POSE pose, uint16_t pos_tick, uint16_t vspeed, uint16_t bomb_range, uint16_t max_bombs, uint16_t bombs_left, uint16_t dead, enum PLAYER player);
void write_player_info(void);
void pass_game_info(void);
void generate_software_map(void);
void init_players(void);
int is_player_moving(player_info_t *info);
enum FACING get_player_facing(player_info_t *info);
void get_player_vpos(uint16_t *pos, player_info_t *info);
uint16_t get_player_vspeed(player_info_t *info);
void handle_player_movement();
#endif
