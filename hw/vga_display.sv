module vga_display(input logic clk,
						 output logic [7:0] VGA_R, VGA_G, VGA_B,
						 output logic VGA_CLK,
						 output logic VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n,
						 input logic [15:0]  writedata,
						 input logic write,
						 input logic chipselect,
						 input logic [4:0] address,
						 output logic isplayer,
						 output logic [15:0] data0,
						 output logic [15:0] data1,
						 output logic [10:0] playeraddr,
						 output logic [7:0] playercolor);

	logic [7:0] local_pixel;
	logic [7:0] idle_down_sprite_raw;
	logic [7:0] background_sprite_raw;
	logic [7:0] stone_sprite_raw;
	logic [7:0] box_sprite_raw;
	
	logic [7:0] map_sig_select_raw;
	logic [3:0] map_sig_select;
	logic [1:0] idle_down_sprite;
	logic [1:0] background_sprite;
	logic [1:0] stone_sprite;
	logic [1:0] box_sprite;
	
	logic [1:0] map_sprite;
	logic [10:0] hcount;
	logic [9:0] vcount;
	logic [1:0] sprite_offset_temp; //Decides which 2 bits in the 1 byte sprite_info
	logic [1:0] sprite_offset;
	logic [7:0] map_local_color_addr;
	logic [7:0] global_color_addr;
	logic [10:0] map_read_addr;
	
	/*Registers for player0*/
	logic [15:0] player_info_00; // moving[10] + xinfo[9:0]
	logic [15:0] player_info_01; // facing[10:9] + yinfo[8:0]
	/*Registers for player1*/
	logic [15:0] player_info_10; // same format
	logic [15:0] player_info_11;
	logic [15:0] map_change_0;
	logic [15:0] map_change_1;
	logic [15:0] map_change_2;
	logic [15:0] map_change_3;
	logic [15:0] map_change_4;
	logic [15:0] map_change_5;
	logic [15:0] map_change_6;
	logic [15:0] map_change_7;
	logic [15:0] map_change_8;
	logic [15:0] map_change_9;
	logic [15:0] map_change_10;
	logic [15:0] map_change_11;
	
	
	/*The variable to determine whether the pixel belongs to player 0 and corresponding shift register*/
	logic is_player_0;
	logic is_player_1;
	logic is_player;
	logic [3:0] is_player_sr;
	
	/*Whether the player is moving*/
	logic moving;
	
	/*shift register for facing*/
	logic [1:0] facing;
	
	logic [10:0] player_addr_player;
	logic [10:0] player_addr_moving;
	logic [10:0] player_addr_facing;
	logic [10:0] player_addr_sprite;
	logic [10:0] player_addr_local;
	logic [10:0] player_addr;
	
	logic [7:0] player_sprite_raw;
	logic [1:0] player_sprite;
	logic [3:0] player_offset_sr;
	logic [7:0] player_color_addr;
	logic [15:0] player_color_addr_sr;
	logic [7:0] player_local_color_addr;
	
	logic [7:0] upper_layer_local_color_addr;
	logic [7:0] lower_layer_local_color_addr;
	logic [7:0] upper_layer_global_color_addr;
	logic [7:0] lower_layer_global_color_addr;
	
	logic map_write;
	logic [10:0] map_write_addr;
	logic [7:0] map_write_data;
	
	parameter PLAYER_OFFSET = 11'd640,
				 MOVING_OFFSET = 11'd192,
				 IDLE_SIDE_OFFSET = 11'd64,
				 IDLE_UP_OFFSET = 11'd128,
				 MOVING_SIDE_OFFSET = 11'd128,
				 MOVING_UP_OFFSET = 11'd320,
				 PLAYER_COLOR_OFFSET = 8'd12;
				 
	
	assign player_addr = player_addr_player + player_addr_moving + player_addr_facing + player_addr_sprite + player_addr_local[7:2];
	
	
	assign is_player_0 = (player_info_00[9:0] != 10'd0) && (hcount[10:1] >= (player_info_00[9:0] - 10'd7)) && (hcount[10:1] <= (player_info_00[9:0] + 10'd8))
						  && (vcount[8:0] >= (player_info_01[8:0] - 9'd7)) && (vcount[8:0] <= (player_info_01[8:0] + 9'd8));
	
	assign is_player_1 = (player_info_10[9:0] != 10'd0) && (hcount[10:1] >= (player_info_10[9:0] - 10'd7)) && (hcount[10:1] <= (player_info_10[9:0] + 10'd8)) 
						  && (vcount[8:0] >= (player_info_11[8:0] - 9'd7)) && (vcount[8:0] <= (player_info_11[8:0] + 9'd8));
	
	assign is_player = is_player_0 || is_player_1;
	assign upper_layer_local_color_addr[7:0] = player_local_color_addr[7:0];
	assign lower_layer_local_color_addr[7:0] = map_local_color_addr[7:0];
	assign isplayer = is_player_0;
	assign data0[15:0] = player_info_00[15:0];
	assign data1[15:0] = player_info_01[15:0];
	assign playeraddr[10:0] = player_addr[10:0];
	assign playercolor[7:0] = player_local_color_addr[7:0];
	
	assign map_write_data[7:4] = 4'd0;
	
	
	vga_counter count(.clk50(clk),
							.hcount(hcount),
							.vcount(vcount),
							.VGA_CLK(VGA_CLK),
							.VGA_HS(VGA_HS),
							.VGA_VS(VGA_VS),
							.VGA_BLANK_n(VGA_BLANK_n),
							.VGA_SYNC_n(VGA_SYNC_n));

	//Decide which pixel to look at
	assign local_pixel[7:4] = vcount[3:0];
	assign local_pixel[3:0] = hcount[4:1];
	
	ram_empty_map empty_map_ram(.clock(clk),
						 .data(map_write_data),
						 .rdaddress(map_read_addr),
						 .wraddress(map_write_addr),
						 .wren(map_write),
						 .q(map_sig_select_raw));
	
	
	
	rom_players players_rom(.address(player_addr),
									.clock(clk),
									.q(player_sprite_raw));
	
	rom_background background_rom(.address(local_pixel[7:2]),
										 .clock(clk),
										 .q(background_sprite_raw));
	
	rom_stone stone_rom(.address(local_pixel[7:2]),
							  .clock(clk),
							  .q(stone_sprite_raw));
							  
	rom_box box_rom(.address(local_pixel[7:2]),
						 .clock(clk),
						 .q(box_sprite_raw));
						 
	assign map_sig_select[3:0] = map_sig_select_raw[3:0];
										 
	always_comb begin
		idle_down_sprite = (idle_down_sprite_raw >> (2 * sprite_offset[1:0])) % 4;
		background_sprite = (background_sprite_raw >> (2 * sprite_offset[1:0])) % 4;
		stone_sprite = (stone_sprite_raw >> (2 * sprite_offset[1:0])) % 4;
		box_sprite = (box_sprite_raw >> (2 * sprite_offset[1:0])) % 4;
		map_read_addr = (vcount[9:4] * 40 + hcount[10:5]);
		player_sprite = (player_sprite_raw >> (2 * player_offset_sr[3:2])) % 4;
		player_color_addr = player_addr[10:6] * 4 + PLAYER_COLOR_OFFSET;
		player_local_color_addr[7:0] = player_color_addr_sr[15:8] + player_sprite;
		
		case (map_sig_select)
			4'd0 : map_sprite = background_sprite;
			4'd1 : map_sprite = stone_sprite;
			4'd2 : map_sprite = box_sprite;
			default : map_sprite = background_sprite;
		endcase
		map_local_color_addr = map_sig_select * 4 + map_sprite;
		if (is_player_1) begin
			player_addr_player = PLAYER_OFFSET;
			facing = player_info_11[10:9];
			moving = player_info_10[10];
			player_addr_sprite = player_info_10 [12:11] * 64;
			if (facing == 2'd1)
				player_addr_local = (vcount - player_info_11[8:0] + 7) * 16 + 15 - (hcount[10:1] - player_info_10[9:0] + 7);
			else
				player_addr_local = (vcount - player_info_11[8:0] + 7) * 16 + (hcount[10:1] - player_info_10[9:0] + 7);
		end else if (is_player_0) begin
			player_addr_player = 11'd0;
			facing = player_info_01[10:9];
			moving = player_info_00[10];
			player_addr_sprite = player_info_00 [12:11] * 64;
			if (facing == 2'd1)
				player_addr_local = (vcount - player_info_01[8:0] + 7) * 16 + 15 - (hcount[10:1] - player_info_00[9:0] + 7);
			else
				player_addr_local = (vcount - player_info_01[8:0] + 7) * 16 + (hcount[10:1] - player_info_00[9:0] + 7);
		end else begin
			player_addr_player = 11'd0;
			facing = 2'd0;
			moving = 1'd0;
			player_addr_sprite = 11'd0;
			player_addr_local = 11'd0;
		end
		if (moving) begin
			player_addr_moving = MOVING_OFFSET;
			case (facing)
				2'd0 : player_addr_facing = 11'd0;
				2'd1 : player_addr_facing = MOVING_SIDE_OFFSET;
				2'd2 : player_addr_facing = MOVING_UP_OFFSET;
				2'd3 : player_addr_facing = MOVING_SIDE_OFFSET;
			endcase
		end else begin
			player_addr_moving = 11'd0;
			case (facing)
				2'd0 : player_addr_facing = 11'd0;
				2'd1 : player_addr_facing = IDLE_SIDE_OFFSET;
				2'd2 : player_addr_facing = IDLE_UP_OFFSET;
				2'd3 : player_addr_facing = IDLE_SIDE_OFFSET;
			endcase
		end
		if (is_player_sr[3]) begin
			if (upper_layer_global_color_addr == 8'd0)
				global_color_addr = lower_layer_global_color_addr;
			else
				global_color_addr = upper_layer_global_color_addr;
		end else begin
			global_color_addr = lower_layer_global_color_addr;
		end
		case(address)
			5'd4 : begin
				map_write_addr[10:0] = map_change_0[10:0];
				map_write_data[3:0] = map_change_0[14:11];
				map_write = map_change_0[15];
			end
			5'd5 : begin
				map_write_addr[10:0] = map_change_1[10:0];
				map_write_data[3:0] = map_change_1[14:11];
				map_write = map_change_1[15];
			end
			5'd6 : begin
				map_write_addr[10:0] = map_change_2[10:0];
				map_write_data[3:0] = map_change_2[14:11];
				map_write = map_change_2[15];
			end
			5'd7 : begin
				map_write_addr[10:0] = map_change_3[10:0];
				map_write_data[3:0] = map_change_3[14:11];
				map_write = map_change_3[15];
			end
			5'd8 : begin
				map_write_addr[10:0] = map_change_4[10:0];
				map_write_data[3:0] = map_change_4[14:11];
				map_write = map_change_4[15];
			end
			5'd9 : begin
				map_write_addr[10:0] = map_change_5[10:0];
				map_write_data[3:0] = map_change_5[14:11];
				map_write = map_change_5[15];
			end
			5'd10 : begin
				map_write_addr[10:0] = map_change_6[10:0];
				map_write_data[3:0] = map_change_6[14:11];
				map_write = map_change_6[15];
			end
			5'd11 : begin
				map_write_addr[10:0] = map_change_7[10:0];
				map_write_data[3:0] = map_change_7[14:11];
				map_write = map_change_7[15];
			end
			5'd12 : begin
				map_write_addr[10:0] = map_change_8[10:0];
				map_write_data[3:0] = map_change_8[14:11];
				map_write = map_change_8[15];
			end
			5'd13 : begin
				map_write_addr[10:0] = map_change_9[10:0];
				map_write_data[3:0] = map_change_9[14:11];
				map_write = map_change_9[15];
			end
			5'd14 : begin
				map_write_addr[10:0] = map_change_10[10:0];
				map_write_data[3:0] = map_change_10[14:11];
				map_write = map_change_10[15];
			end
			5'd15 : begin
				map_write_addr[10:0] = map_change_11[10:0];
				map_write_data[3:0] = map_change_11[14:11];
				map_write = map_change_11[15];
			end
			default : begin
				map_write_addr[10:0] = 11'd0;
				map_write_data[3:0] = 4'd0;
				map_write = 0;
			end
		endcase
		
	end
	
	
	
	rom_local_color local_colors_rom(.address_a(upper_layer_local_color_addr[6:0]),
												.address_b(lower_layer_local_color_addr[6:0]),
												.clock(clk),
												.q_a(upper_layer_global_color_addr),
												.q_b(lower_layer_global_color_addr));
												 
	rom_global_color_r global_color_r(.address(global_color_addr[4:0]),
												 .clock(clk),
												 .q(VGA_R));
	
	rom_global_color_g global_color_g(.address(global_color_addr[4:0]),
												 .clock(clk),
												 .q(VGA_G));
	
	
	rom_global_color_b global_color_b(.address(global_color_addr[4:0]),
												 .clock(clk),
												 .q(VGA_B));
	
	
	
	always_ff @(posedge clk) begin
		sprite_offset[1:0] <= sprite_offset_temp[1:0];
		sprite_offset_temp[1:0] <= local_pixel[1:0];
		player_offset_sr[1:0] <= player_addr_local[1:0];
		player_offset_sr[3:2] <= player_offset_sr[1:0];
		player_color_addr_sr[7:0] <= player_color_addr[7:0];
		player_color_addr_sr[15:8] <= player_color_addr_sr[7:0];
		is_player_sr[0] <= is_player;
		is_player_sr[1] <= is_player_sr[0];
		is_player_sr[2] <= is_player_sr[1];
		is_player_sr[3] <= is_player_sr[2];
		
		if (chipselect && write) begin
			case(address)
				5'd0 : player_info_00[15:0] <= writedata[15:0];
				5'd1 : player_info_01[15:0] <= writedata[15:0];
				5'd2 : player_info_10[15:0] <= writedata[15:0];
				5'd3 : player_info_11[15:0] <= writedata[15:0];
				5'd4 : map_change_0 [15:0] <= writedata[15:0];
				5'd5 : map_change_1 [15:0] <= writedata[15:0];
				5'd6 : map_change_2 [15:0] <= writedata[15:0];
				5'd7 : map_change_3 [15:0] <= writedata[15:0];
				5'd8 : map_change_4 [15:0] <= writedata[15:0];
				5'd9 : map_change_5 [15:0] <= writedata[15:0];
				5'd10 : map_change_6 [15:0] <= writedata[15:0];
				5'd11 : map_change_7 [15:0] <= writedata[15:0];
				5'd12 : map_change_8 [15:0] <= writedata[15:0];
				5'd13 : map_change_9 [15:0] <= writedata[15:0];
				5'd14 : map_change_10 [15:0] <= writedata[15:0];
				5'd15 : map_change_11 [15:0] <= writedata[15:0];
			endcase
		end
	end
	


	
endmodule

module vga_counter(input logic clk50,
						 output logic [10:0] hcount,
						 output logic [9:0]  vcount,
						 output logic VGA_CLK,
						 output logic VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);
						 
	parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

   logic endOfLine;
	logic [10:0] effective_hcount;
	logic [9:0]  effective_vcount;
	
	count_sr sr_count_ram(.clock(clk50),
								 .shiftin({hcount[10:0], vcount[9:0]}),
								 .shiftout({effective_hcount[10:0], effective_vcount[9:0]}));
   
   always_ff @(posedge clk50)
		if (endOfLine) hcount <= 0;
		else  	      hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;
       
   logic endOfField;
   
   always_ff @(posedge clk50)
		if (endOfLine)
			if (endOfField)   vcount <= 0;
			else					vcount <= vcount + 10'd 1;
		 
	assign endOfField = vcount == VTOTAL - 1;
	assign VGA_CLK = hcount[0];
	assign VGA_HS = !( (effective_hcount[10:8] == 3'b101) &
		      !(effective_hcount[7:5] == 3'b111));
   assign VGA_VS = !( effective_vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1'b0; // For putting sync on the green signal; unused
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to 479
   // 101 0000 0000  1280	       01 1110 0000  480
   // 110 0011 1111  1599	       10 0000 1100  524
   assign VGA_BLANK_n = !( effective_hcount[10] & (effective_hcount[9] | effective_hcount[8]) ) &
			!( effective_vcount[9] | (effective_vcount[8:5] == 4'b1111) );

endmodule

