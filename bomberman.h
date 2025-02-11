struct user_command_struct {
    int command;
};
struct player {
    int x;
    int y;
    int dead;
    int facing;
    int remaining_bombs;
};
void handle_bomber_explode(void);
void handle_player_movement(int);
void handle_bomb_placement(int);

struct user_command_struct uc1, uc2;
void receive_from_user (int);
struct player player1, player2;

