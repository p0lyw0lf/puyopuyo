#include "gameboard.h"
#include "contracts.h"

bool __gameboard_is_gb_data_t(gb_data_t* gbdata) {
  if (gbdata == NULL) {
    fprintf(stderr, "Error: NULL gbdata\n");
    return false;
  }

  if (gbdata->spritesheet == NULL) {
    fprintf(stderr, "Error: NULL gbdata->spritesheet\n");
    return false;
  }
  
  if (gbdata->clips == NULL) {
    fprintf(stderr, "Error: NULL gbdata->clips\n");
    return false;
  }
  
  if (gbdata->board == NULL) {
    fprintf(stderr, "Error: NULL gbdata->board\n");
    return false;
  }
  
  if (gbdata->gui == NULL) {
    fprintf(stderr, "Error: NULL gbdata->gui\n");
    return false;
  }

  return true;
}

gb_data_t* gameboard_load_media(ACGL_gui_t* gui, puyo_board_t* board) {
  if (gui == NULL) {
    fprintf(stderr, "Error: NULL gui passed in gameboard_load_media\n");
    return NULL;
  }

  if (board == NULL) {
    fprintf(stderr, "Error: NULL board pased in gameboard_load_media\n");
    return NULL;
  }

  gb_data_t* gbdata = (gb_data_t*)malloc(sizeof(gb_data_t));

  if (gbdata == NULL) {
    fprintf(stderr, "Error: could not malloc gameboard data!\n");
    return NULL;
  }

  gbdata->clips = ss_split();
  if (gbdata->clips == NULL) {
    fprintf(stderr, "Error: gameboard could not load spritesheet clips. More details above?\n");
    free(gbdata);
    return NULL;
  }

  gbdata->spritesheet = ss_load(gui->renderer);
  if (gbdata->spritesheet == NULL) {
    fprintf(stderr, "Error: gameboard could not load spritesheet image. More details above?\n");
    free(gbdata->clips);
    free(gbdata);
    return NULL;
  }

  gbdata->gui = gui;
  gbdata->board = board;

  gameboard_contruct_gui(gbdata);

  ENSURES(__gameboard_is_gb_data_t(gbdata));
  return gbdata;
}

bool color_rectangle_callback(SDL_Renderer* renderer, SDL_Rect location, void* data) {
  REQUIRES(data != NULL);
  SDL_Color old_color, *new_color;
  new_color = (SDL_Color*)data;
  if (SDL_GetRenderDrawColor(renderer, &old_color.r, &old_color.g, &old_color.b, &old_color.a) != 0) {
    fprintf(stderr, "Error: could not determine previous renderer draw color! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (SDL_SetRenderDrawColor(renderer, new_color->r, new_color->g, new_color->b, new_color->a) != 0) {
    fprintf(stderr, "Error: could not set draw color! SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  if (SDL_RenderFillRect(renderer, &location) != 0) {
    fprintf(stderr, "Error: could not fill rectangle! SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  if (SDL_SetRenderDrawColor(renderer, old_color.r, old_color.g, old_color.b, old_color.a) != 0) {
    fprintf(stderr, "Error: could not re-set draw color! SDL_Error %s\n", SDL_GetError());
    return false;
  }

  return true;
}

void gameboard_contruct_gui(gb_data_t* gbdata) {
  if (gbdata == NULL) {
    fprintf(stderr, "Error: NULL gbdata passed in gameboard_contruct_gui\n");
    return;
  }

  if (gbdata->gui == NULL) {
    fprintf(stderr, "Error: gbdata has NULL gui in gameboard_contruct_gui\n");
    return;
  }

  // The gui really should be well constructed, idk
  SDL_Renderer* renderer = gbdata->gui->renderer;
  //ACGL_gui_node_remove_all_children(gbdata->gui->root);


  ACGL_gui_object_t* board_base = ACGL_gui_node_init(renderer, &gameboard_render, (void*)gbdata);
  if (board_base == NULL) {
    fprintf(stderr, "Error: could not create board gui node!\n");
    return;
  }

  board_base->node_type = ACGL_GUI_NODE_FILL_H + ACGL_GUI_NODE_PRESERVE_ASPECT;
  board_base->anchor = ACGL_GUI_ANCHOR_CENTER;
  board_base->x = 0;
  board_base->y = 0;
  board_base->w = 6;
  board_base->h = 12;
  board_base->min_w = 60;
  board_base->max_w = 500;

  ACGL_gui_node_add_child_front(gbdata->gui->root, board_base);
  gbdata->board->board_object = board_base;

  // wow 4 bytes leaked soo scaary
  SDL_Color* blue = (SDL_Color*)malloc(sizeof(SDL_Color));
  blue->r = 0; blue->g = 0; blue->b = 255; blue->a = 255;
  ACGL_gui_object_t* background = ACGL_gui_node_init(renderer, &color_rectangle_callback, (void*)blue);
  if (background == NULL) {
    fprintf(stderr, "Error: could not create background gui node!\n");
    ACGL_gui_node_remove_child(gbdata->gui->root, board_base);
    ACGL_gui_node_destroy(board_base);
    return;
  }

  // default settings are enough for the background
  ACGL_gui_node_add_child_back(gbdata->gui->root, background);
}

void gameboard_destroy(gb_data_t* gbdata) {
  REQUIRES(gbdata != NULL);
  if (gbdata->spritesheet != NULL) {
    SDL_DestroyTexture(gbdata->spritesheet);
    gbdata->spritesheet = NULL;
  }

  if (gbdata->clips != NULL) {
    free(gbdata->clips);
    gbdata->clips = NULL;
  }

  // board and gui should be managed further up, don't delete them
  gbdata->gui = NULL;
  gbdata->board = NULL;
  //if (gbdata->gui != NULL) {
  //  ACGL_gui_node_remove_all_children(gbdata->gui->root);
  //}
  free(gbdata);
}

bool gameboard_background_render(SDL_Renderer* renderer, SDL_Rect location, void* data) {
  // I need to find a suitable background first lol
  SDL_Color* green = (SDL_Color*)malloc(sizeof(SDL_Color));
  green->r = 0; green->g = 255; green->b = 0; green->a = 255;
  color_rectangle_callback(renderer, location, green);
  free(green);
  return true;
}

void gameboard_render_board(SDL_Renderer* renderer, SDL_Rect location, gb_data_t* gbdata) {
  REQUIRES(renderer != NULL);
  REQUIRES(gbdata != NULL);

  printf("x: %d y: %d, w: %d h: %d\n", location.x, location.y, location.w, location.h);

  int col_width = location.w / PUYO_WIDTH;
  int row_height = location.h / PUYO_HEIGHT;
  puyo_board_t* board = gbdata->board;

  // Board background has already been drawn, place existing puyos on top
  SDL_Rect rect;
  rect.w = col_width;
  rect.h = row_height;
  for (char y=0; y<PUYO_HEIGHT; y++) {
    rect.y = location.y + location.h - (y+1)*row_height; // board is drawn bottom up
    for (char x=0; x<PUYO_WIDTH; x++) {
      rect.x = location.x + x*col_width;

      enum PUYO_COLOR_IDS puyo_color = board->area[x*PUYO_HEIGHT_ACT+y];
      if (puyo_color == PUYO_COLOR_1 || puyo_color == PUYO_COLOR_2 || puyo_color == PUYO_COLOR_3 || puyo_color == PUYO_COLOR_4) {
        int puyo_id = board->color_to_sprite[puyo_color];
        // make connections with surrounding puyos
        if (x > 0 && board->area[(x-1)*PUYO_HEIGHT_ACT+y] == puyo_color) {
          puyo_id += PUYO_LEFT;
        }
        if (x < PUYO_WIDTH-1 && board->area[(x+1)*PUYO_HEIGHT_ACT+y] == puyo_color) {
          puyo_id += PUYO_RIGHT;
        }
        if (y > 0 && board->area[x*PUYO_HEIGHT_ACT+y-1] == puyo_color) {
          puyo_id += PUYO_DOWN;
        }
        if (y < PUYO_HEIGHT_ACT-1 && board->area[x*PUYO_HEIGHT_ACT+y+1] == puyo_color) {
          puyo_id += PUYO_UP;
        }
        // and then render
        ss_render(renderer, gbdata->spritesheet, gbdata->clips, &rect, puyo_id);
      } else if (puyo_color == PUYO_COLOR_GARBAGE) {
        ss_render(renderer, gbdata->spritesheet, gbdata->clips, &rect, PUYO_GARBAGE_BLOB);
      }
    }
  }
}

void gameboard_render_falling(SDL_Renderer* renderer, SDL_Rect* location, puyo_board_t* board, SDL_Texture* spritesheet, SDL_Rect* clips) {
  int col_width = location->w / PUYO_WIDTH;
  int row_height = location->h / PUYO_HEIGHT;
  // Then draw in the current piece
  // Assume there are no overlaps or out-of-bounds (hopefully that's taken care of elsewhere)
  SDL_Rect rect;
  rect.x = board->current_x*col_width + location->x;
  rect.y = location->y + location->h - (board->current_y+1)*row_height; // board is drawn bottom up
  rect.w = col_width;
  rect.h = row_height;

  // top is the place we rotate around
  //ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->current_pair.top]);

  switch (board->current_rot_state) {
    case ROT_LEFT:
      rect.x -= 1;
      break;
    case ROT_UP:
      rect.y -= 1;
      break;
    case ROT_RIGHT:
      rect.x += 1;
      break;
    default: // ROT_DOwN
      rect.y += 1;
      break;
  }

  // bot(tom) part gets rotated
  //ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->current_pair.bot]);
}

void gameboard_render_upcoming(SDL_Renderer* renderer, SDL_Rect* location, puyo_board_t* board, SDL_Texture* spritesheet, SDL_Rect* clips) {
  int col_width = location->w / PUYO_WIDTH;
  int row_height = location->h / PUYO_HEIGHT;
  if (board->pairs_have_changed) {
    // clear area surrouding current piece in a 5x5 grid to account for
    // every possible previous location/rotation
    SDL_Rect rect;
    /*for (int x=board->current_x-2; x<board->current_x+3; x++) {
      if (x < 0) { continue; }
      if (x >= PUYO_WIDTH) { break; }
      for (int y=board->current_y-2; y<board->current_y+3; y++) {
        if (y < 0) { continue; }
        if (y >= PUYO_HEIGHT) { break; }

        rect.x = x*col_width + location->x;
        rect.y = location->y + location->h - (y+1)*row_height; // board is drawn bottom up
        rect.w = col_width;
        rect.h = row_height;
        // later on, actually fill space with appropriate background tile
        // for now tho, just this is fine
        SDL_RenderFillRect(renderer, &rect);
      }
    }*/



    // Now to render the next pairs off to the side
    rect.x = location->x + location->w;
    rect.y = location->y;
    rect.w = col_width * 2 / 3;
    rect.h = row_height * 2 / 3;
    SDL_RenderFillRect(renderer, &rect); // always clear before re-drawing
    ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->next_pair.top]);

    rect.y += rect.h;
    SDL_RenderFillRect(renderer, &rect);
    ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->next_pair.bot]);

    // add a little space in between the pairs
    rect.y += row_height;
    SDL_RenderFillRect(renderer, &rect);
    ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->next_next_pair.top]);

    rect.y += rect.h;
    SDL_RenderFillRect(renderer, &rect);
    ss_render(renderer, spritesheet, clips, &rect, board->color_to_sprite[board->next_next_pair.bot]);

    // Finally, make it so we don't update the next time around
    board->pairs_have_changed = false;
    SDL_RenderPresent(renderer);
  }
}

bool gameboard_render(SDL_Renderer* renderer, SDL_Rect location, void* data) {
  REQUIRES(renderer != NULL);
  REQUIRES(data != NULL);

  gb_data_t* gbdata = (gb_data_t*)data;

  if (SDL_LockMutex(gbdata->board->mutex) != 0) {
    fprintf(stderr, "Could not lock mutex in gameboard_render! SDL Error: %s\n", SDL_GetError());
  }

  gameboard_background_render(renderer, location, gbdata);
  gameboard_render_board(renderer, location, gbdata);

  SDL_UnlockMutex(gbdata->board->mutex);
  return true;
}
