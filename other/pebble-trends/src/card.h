#include <pebble.h>

#ifndef CARD_H
#define CARD_H

#define CARD_BUFFER_SIZE 64

typedef struct {
  TextLayer *title_layer;
  TextLayer *timestamp_layer;

  char
    *title_buff,
    *timestamp_buff;
} Card;

/**
 * 1. Create a Card
 */
Card* card_create();

/**
 * 2. Set the Card's title
 */
void card_set_title(Card *this, char *title);

/**
 * 3. Set the Card's description
 */
void card_set_description(Card *this, char *desc);

/**
 * 4. Add the Card's ScrollLayer to a parent Window
 */
void card_add_to_window(Card *this, Window *parent);

/**
 * 5. Destroy a Card
 */
void card_destroy(Card *this);

/**
 * Hide a card
 */
void card_hide(Card *this);

/**
 * Show a card
 */
void card_show(Card *this);

#endif