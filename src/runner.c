#include "runner.h"

bool runner_setup(void* data) {
    runnerData* grdata = (runnerData*)data;
    if (SDL_LockMutex(grdata->board->mutex) != 0) {
        fprintf(stderr, "Could not lock board mutex in runner_setup! SDL_Error %s\n", SDL_GetError());
        return false;
    }

    for (char y = 0; y < PUYO_HEIGHT; y++) {
        for (char x = 0; x < PUYO_WIDTH; x++) {
            grdata->board->area[x * PUYO_HEIGHT_ACT + y] = puyo_get_random();
        }
    }

    SDL_UnlockMutex(grdata->board->mutex);
    return true;
}

bool runner_loop(void* data) {
    runnerData* grdata = (runnerData*)data;
    if (SDL_LockMutex(grdata->board->mutex) != 0) {
        fprintf(stderr, "Could not lock board mutex in runner_mainloop! SDL_Error %s\n", SDL_GetError());
        return 1;
    }
    score_t new_score;

    switch (grdata->state) {
    case READY:
        new_score = puyo_pop_groups(grdata->board);
        if (new_score) {
            grdata->board->score += new_score;
            printf("Got score %d\n", new_score);
            grdata->state = POP_FLASHING;
        }
        else {
            grdata->board->chain = 0;
        }
        break;
    case POP_FLASHING:
        puyo_apply_pops(grdata->board);
        puyo_mark_board_changed(grdata->board);
        grdata->state = POP_FALLING;
        break;
    case POP_FALLING:
        puyo_apply_gravity(grdata->board);
        puyo_mark_board_changed(grdata->board);
        grdata->state = READY;
        break;
    default:
        grdata->state = READY;
        break;
    }

    SDL_UnlockMutex(grdata->board->mutex);

    // TODO: Eventually I'd like this to be handled by ACGL as well
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = REFRESH_REQUEST;
    event.user.data1 = NULL;
    event.user.data2 = NULL;
    SDL_PushEvent(&event);

    return true;
}

runnerData* runner_create(puyo_board_t* board) {
    runnerData* data = (runnerData*)malloc(sizeof(runnerData));
    data->board = board;
    data->state = READY;
    return data;
}

void runner_destroy(void* data) {
    runnerData* grdata = (runnerData*)data;
    if (data != NULL) {
        // We are not in charge of data->board
        free(data);
    }
}
