# PGE Sprite Documentation

## Creating Game Sprites

The `PGESprite` structure allows creation of a bitmap sprite-based game entity
and a set of convenience functions to manage it.

 * Include `pge/modules/pge_sprite.h` in your main C file:

        #include "pge/modules/pge_sprite.h"

 * Create a structure containing a `PGESprite` pointer member:

        typedef struct {
          PGESprite *sprite;
          int direction;
          bool moving;
        } Robot;

 * Create the `PGESprite` when the entity is created:

        this->sprite = pge_sprite_create(start_position, RESOURCE_ID_ROBOT_UP);

 * Update the sprite's bitmap as the player plays the game:

        // Set the sprite bitmap
        switch(this->direction) {
          case DIRECTION_UP:
            pge_sprite_set_frame(this->sprite, RESOURCE_ID_ROBOT_UP);
            break;
          case DIRECTION_RIGHT:
            pge_sprite_set_frame(this->sprite, RESOURCE_ID_ROBOT_RIGHT);
            break;
          case DIRECTION_DOWN:
            pge_sprite_set_frame(this->sprite, RESOURCE_ID_ROBOT_DOWN);
            break;
          case DIRECTION_LEFT:
            pge_sprite_set_frame(this->sprite, RESOURCE_ID_ROBOT_LEFT);
            break;
        }

 * Update the entity's position:

        GPoint pos = GPoint(25 + dx, 25 + dy);
        pge_sprite_set_position(this->sprite, pos);

or to move by a certain number of pixels:

        // Move by (10, 0)
        pge_sprite_move(this->sprite, 10, 0);

 * Draw the entity's bitmap:

        pge_sprite_draw(this->sprite, ctx);

 * Destroy the entity's `PGESprite` when destroying the app:

        void robot_destroy(Robot *this) {
          pge_sprite_destroy(this->sprite);
          free(this);
        }
