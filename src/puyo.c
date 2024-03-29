#include "puyo.h"

#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif
#ifndef max
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif

const score_t PUYO_CHAIN_MULTIPLIERS[PUYO_MAX_CHAIN + 1] = {
  0,     // 0
  4,     // 1
  20,    // 2
  24,    // 3
  32,    // 4
  48,    // 5
  96,    // 6
  160,   // 7
  240,   // 8
  320,   // 9
  480,   // 10
  600,   // 11
  700,   // 12
  800,   // 13
  900,   // 14
  999,   // 15
};

const score_t PUYO_COLOR_MULTIPLIERS[PUYO_NUM_COLORS + 1] = {
  0,
  0, // 1
  3, // 2
  6, // 3
  12 // 4
};

const score_t PUYO_GROUP_MULTIPLIERS[PUYO_MAX_GROUP + 1] = {
  0,
  0,
  0,
  0,
  0, // 4
  2, // 5
  3, // 6
  4, // 7
  5, // 8
  6, // 9
  7, // 10
  10 // 11
};

puyo_board_t* puyo_create_board() {
    // Seed random number generator
    srand((unsigned)time(NULL));

    puyo_board_t* board = (puyo_board_t*)malloc(sizeof(puyo_board_t));
    if (board == NULL) {
        goto puyo_create_board_fail1;
    }
    board->mutex = SDL_CreateMutex();
    if (board->mutex == NULL) {
        fprintf(stderr, "Error: could not create mutex. SDL Error: %s\n", SDL_GetError());
        goto puyo_create_board_fail2;
    }
    board->area = (puyo_t*)malloc(sizeof(puyo_t) * (PUYO_WIDTH * PUYO_HEIGHT_ACT));
    if (board->area == NULL) {
        fprintf(stderr, "Could not create board area!\n");
        goto puyo_create_board_fail3;
    }
    for (char x = 0; x < PUYO_WIDTH; x++) {
        for (char y = 0; y < PUYO_HEIGHT_ACT; y++) {
            board->area[x * PUYO_HEIGHT_ACT + y] = PUYO_COLOR_NONE;
        }
    }
    board->popping_area = (bool*)malloc(sizeof(bool) * (PUYO_WIDTH * PUYO_HEIGHT_ACT));
    if (board->popping_area == NULL) {
        fprintf(stderr, "Could not create board popping buffer!\n");
        goto puyo_create_board_fail4;
    }

    board->score = 0;
    board->incoming_garbage = 0;
    board->chain = 0;

    board->current_pair = puyo_get_random_pair();
    board->rot_state = ROT_DOWN;
    board->puyo1_x = 3;
    board->puyo1_y = PUYO_HEIGHT;
    board->puyo2_x = 3;
    board->puyo2_y = PUYO_HEIGHT - 1;

    board->next_pair = puyo_get_random_pair();
    board->next_next_pair = puyo_get_random_pair();

    board->is_popping = false;
    board->popping_counter = 0;

    board->board_has_changed = true;
    board->pairs_have_changed = true;

    board->board_object = NULL;
    board->pairs_object = NULL;

    // Later: randomly initialize this
    board->color_to_sprite[PUYO_COLOR_1] = PUYO_RED;
    board->color_to_sprite[PUYO_COLOR_2] = PUYO_GREEN;
    board->color_to_sprite[PUYO_COLOR_3] = PUYO_BLUE;
    board->color_to_sprite[PUYO_COLOR_4] = PUYO_YELLOW;

    return board;

puyo_create_board_fail4:
    free(board->area);
puyo_create_board_fail3:
    SDL_DestroyMutex(board->mutex);
puyo_create_board_fail2:
    free(board);
puyo_create_board_fail1:
    return NULL;
}

bool puyo_mark_board_changed(puyo_board_t* board) {
    // Assuming caller has locked mutex already. Checking for NULL just in case
    if (board == NULL) {
        return false;
    }

    board->board_has_changed = true;

    if (board->board_object != NULL) {
        if (SDL_LockMutex(board->board_object->mutex) != 0) {
            fprintf(stderr, "Could not lock ACGL mutex in puyo_mark_board_changed! SDL_Error %s\n", SDL_GetError());
            return false;
        }
        board->board_object->needs_update = true;
        SDL_UnlockMutex(board->board_object->mutex);
    }

    return true;
}

bool puyo_mark_pairs_changed(puyo_board_t* board) {
    // Assuming caller has locked mutex already. Checking for NULL just in case
    if (board == NULL) {
        return false;
    }

    board->pairs_have_changed = true;

    if (board->pairs_object != NULL) {
        if (SDL_LockMutex(board->pairs_object->mutex) != 0) {
            fprintf(stderr, "Could not lock ACGL mutex in puyo_mark_pairs_changed! SDL_Error %s\n", SDL_GetError());
            return false;
        }
        board->pairs_object->needs_update = true;
        SDL_UnlockMutex(board->pairs_object->mutex);
    }

    return true;
}
void puyo_free_board(puyo_board_t* board) {
    // REQUIRES: no thread contention, because this destroys the mutex
    if (board != NULL) {
        if (board->area != NULL) {
            free(board->area);
            board->area = NULL;
        }
        if (board->popping_area != NULL) {
            free(board->popping_area);
            board->popping_area = NULL;
        }
        if (board->mutex != NULL) {
            SDL_DestroyMutex(board->mutex);
            board->mutex = NULL;
        }
        // Only zeroing out b/c these are ACGL objects, handled elsewhere
        board->board_object = NULL;
        board->pairs_object = NULL;
        free(board);
        // Don't forget to set board to NULL on the outside too!
    }
}

puyo_t puyo_get_random() {
    return rand() % PUYO_NUM_COLORS;
}

puyo_pair_t puyo_get_random_pair() {
    puyo_pair_t pair = {0, 0};
    pair.top = puyo_get_random();
    pair.bot = puyo_get_random();
    return pair;
}

Uint8 __puyo_count_group_size(puyo_board_t* board, char x, char y, bool* marked) {
    Uint8 count = 1;
    int index = x * PUYO_HEIGHT_ACT + y;
    if (marked[index]) {
        // We've been here already
        return 0;
    }
    marked[index] = true;
    puyo_t puyo_color = board->area[index];
    if (!is_playable_puyo(puyo_color)) {
        return 0;
    }
    if (x > 0 && board->area[(x - 1) * PUYO_HEIGHT_ACT + y] == puyo_color) {
        // expand left
        count += __puyo_count_group_size(board, x - 1, y, marked);
    }
    if (x < PUYO_WIDTH - 1 && board->area[(x + 1) * PUYO_HEIGHT_ACT + y] == puyo_color) {
        // expand right
        count += __puyo_count_group_size(board, x + 1, y, marked);
    }
    if (y > 0 && board->area[x * PUYO_HEIGHT_ACT + y - 1] == puyo_color) {
        // expand down
        count += __puyo_count_group_size(board, x, y - 1, marked);
    }
    if (y < PUYO_HEIGHT_ACT - 1 && board->area[x * PUYO_HEIGHT_ACT + y + 1] == puyo_color) {
        // expand up
        count += __puyo_count_group_size(board, x, y + 1, marked);
    }
    return count;
}

void __puyo_group_popping(puyo_board_t* board, char x, char y, bool* marked) {
    int index = x * PUYO_HEIGHT_ACT + y;
    if (marked[index]) {
        // We've been here already
        return;
    }
    marked[index] = true;
    board->popping_area[index] = true;

    puyo_t puyo_color = board->area[index];
    if (x > 0) {
        int left_index = (x - 1) * PUYO_HEIGHT_ACT + y;
        puyo_t left = board->area[left_index];
        if (left == puyo_color) {
            // expand left
            __puyo_group_popping(board, x - 1, y, marked);
        }
        else if (left == PUYO_COLOR_GARBAGE) {
            board->popping_area[left_index] = true;
        }
    }
    if (x < PUYO_WIDTH - 1) {
        int right_index = (x + 1) * PUYO_HEIGHT_ACT + y;
        puyo_t right = board->area[right_index];
        if (right == puyo_color) {
            // expand right
            __puyo_group_popping(board, x + 1, y, marked);
        }
        else if (right == PUYO_COLOR_GARBAGE) {
            board->popping_area[right_index] = true;
        }
    }
    if (y > 0) {
        int down_index = x * PUYO_HEIGHT_ACT + y - 1;
        puyo_t down = board->area[down_index];
        if (down == puyo_color) {
            // expand down
            __puyo_group_popping(board, x, y - 1, marked);
        }
        else if (down == PUYO_COLOR_GARBAGE) {
            board->popping_area[down_index] = true;
        }
    }
    if (y < PUYO_HEIGHT - 1) {
        int up_index = x * PUYO_HEIGHT_ACT + y + 1;
        puyo_t up = board->area[up_index];
        if (up == puyo_color) {
            // expand up
            __puyo_group_popping(board, x, y + 1, marked);
        }
        else if (up == PUYO_COLOR_GARBAGE) {
            board->popping_area[up_index] = true;
        }
    }
}

score_t puyo_pop_groups(puyo_board_t* board) {
    if (board == NULL) {
        // TODO: make a method that checks all fields of board for malformedness
        fprintf(stderr, "Called puyo_pop_grous on a NULL board\n");
        return 0;
    }

    bool marked[PUYO_WIDTH * PUYO_HEIGHT_ACT];
    SDL_zero(marked);
    Uint8 group_sizes[PUYO_WIDTH * PUYO_HEIGHT] = {0};

    for (char y = 0; y < PUYO_HEIGHT; y++) {
        for (char x = 0; x < PUYO_WIDTH; x++) {
            group_sizes[x * PUYO_HEIGHT + y] = __puyo_count_group_size(board, x, y, marked);
        }
    }

    SDL_zero(marked);
    if (!(board->is_popping)) {
        SDL_memset(board->popping_area, 0, sizeof(bool) * PUYO_WIDTH * PUYO_HEIGHT_ACT);
    }

    Uint8 puyo_cleared = 0;
    Uint8 groups_cleared = 0;
    bool color_cleared[PUYO_NUM_COLORS] = { false, false, false, false };

    for (char y = 0; y < PUYO_HEIGHT; y++) {
        for (char x = 0; x < PUYO_WIDTH; x++) {
            Uint8 group_size = group_sizes[x * PUYO_HEIGHT + y];
            if (group_size >= PUYO_GROUP_POP) {
                __puyo_group_popping(board, x, y, marked);

                puyo_cleared += group_size;
                groups_cleared += 1;
                puyo_t puyo_color = board->area[x * PUYO_HEIGHT_ACT + y];
                printf("Popping group of %d puyos, color %d\n", group_size, puyo_color);
                assert(0 <= puyo_color && puyo_color < PUYO_NUM_COLORS);
                color_cleared[puyo_color] = true;
            }
        }
    }
    groups_cleared = min(groups_cleared, PUYO_MAX_GROUP);

    Uint8 num_color_cleared = 0;
    if (color_cleared[PUYO_COLOR_1]) { num_color_cleared++; }
    if (color_cleared[PUYO_COLOR_2]) { num_color_cleared++; }
    if (color_cleared[PUYO_COLOR_3]) { num_color_cleared++; }
    if (color_cleared[PUYO_COLOR_4]) { num_color_cleared++; }
    score_t color_bonus = PUYO_COLOR_MULTIPLIERS[num_color_cleared];
    score_t group_bonus = PUYO_GROUP_MULTIPLIERS[groups_cleared];
    if (puyo_cleared) {
        board->chain++;
        board->is_popping = true;
        printf("Chain count: %d\n", board->chain);
    }
    // get chain from board state somewhere
    score_t chain_bonus = PUYO_CHAIN_MULTIPLIERS[board->chain];

    score_t final_score = (10 * (score_t)puyo_cleared) * (color_bonus + group_bonus + chain_bonus);

    return final_score;
}

bool puyo_apply_pops(puyo_board_t* board) {
    if (board == NULL) {
        fprintf(stderr, "puyo_apply_pops called on a NULL board\n");
        return false;
    }

    if (!board->is_popping) {
        // If we should not pop puyos, don't
        return false;
    }

    bool has_popped = false;

    for (char x = 0; x < PUYO_WIDTH; x++) {
        for (char y = 0; y < PUYO_HEIGHT_ACT; y++) {
            int index = x * PUYO_HEIGHT_ACT + y;
            if (board->popping_area[index]) {
                board->area[index] = PUYO_COLOR_NONE;
                has_popped = true;
            }
        }
    }

    board->is_popping = false;
    return has_popped;
}

bool puyo_apply_gravity(puyo_board_t* board) {
    if (board == NULL) {
        fprintf(stderr, "puyo_apply_gravity called on a NULL board\n");
        return false;
    }

    puyo_t new_col[PUYO_HEIGHT_ACT] = {0};
    Uint8 i;
    bool seen_nonempty;
    bool did_shift = false;


    for (char x = 0; x < PUYO_WIDTH; x++) {
        i = 0;
        seen_nonempty = false;

        for (char y = 0; y < PUYO_HEIGHT_ACT; y++) {
            int index = x * PUYO_HEIGHT_ACT + y;
            puyo_t puyo_color = board->area[index];
            if (puyo_color != PUYO_COLOR_NONE) {
                seen_nonempty = true;
                new_col[i] = puyo_color;
                i++;
            }
            else if (seen_nonempty) {
                did_shift = true;
            }
        }

        for (; i < PUYO_HEIGHT_ACT; i++) {
            new_col[i] = PUYO_COLOR_NONE;
        }

        memcpy(
            &(board->area[x * PUYO_HEIGHT_ACT]), // dest
            &new_col,                            // src
            PUYO_HEIGHT_ACT * sizeof(puyo_t)     // size
        );
    }

    return did_shift;
}
