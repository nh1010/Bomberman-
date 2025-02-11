#ifndef _VGA_BALL_H
#define _VGA_BALL_H

#include <linux/ioctl.h>

typedef struct {
	unsigned short playerinfo00, playerinfo01, playerinfo10, playerinfo11,
    map_change_0, map_change_1, map_change_2, map_change_3, map_change_4, map_change_5, map_change_6,map_change_7, map_change_8, map_change_9, map_change_10, map_change_11, reserved_0, reserved_1;
} game_info_t;
  

typedef struct {
    game_info_t background;
} vga_ball_arg_t;

#define VGA_BALL_MAGIC 'q'

/* ioctls and their arguments */
#define VGA_BALL_WRITE_BACKGROUND _IOW(VGA_BALL_MAGIC, 1, vga_ball_arg_t *)
#define VGA_BALL_READ_BACKGROUND  _IOR(VGA_BALL_MAGIC, 2, vga_ball_arg_t *)

#endif
