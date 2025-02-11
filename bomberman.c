
# include "bomberman.h"
int main()
{
    while (1) {
        handle_bomber_explode();
        handle_box_destruction();
        receive_from_user(0);
        receive_from_user(1);
        handle_bomb_placement(0);
        handle_bomb_placement(1);
        handle_player_movement(0);
        handle_player_movement(1);
        if (player1.dead == 0) {
            handle_player_dead(0);
            break;
        }
        if (player2.dead == 0) {
            handle_player_dead(1);
            break;
        }


    }
    return 0;

}