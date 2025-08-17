/**
 * Optional sprite object add-on for PGE
 * 
 * 1. Create your game entity and include a PGESprite member;
 * 2. Easy to manage entity position and frame animation
 */

#pragma once
 
#include <pebble.h>

// Sprite base object
typedef struct {
  GBitmap *bitmap;
  GPoint position;
} PGESprite;

/**
 * Create a sprite object
 */
PGESprite* pge_sprite_create(GPoint position, int initial_resource_id);

/**
 * Destroy a sprite object
 */
void pge_sprite_destroy(PGESprite *this);

/**
 * Set the current animation frame
 */
void pge_sprite_set_anim_frame(PGESprite *this, int resource_id);

/**
 * Draw the sprite's bitmap to the graphics context
 */
void pge_sprite_draw(PGESprite *this, GContext *ctx);

/**
 * Set the position of the sprite
 */
void pge_sprite_set_position(PGESprite *this, GPoint new_position);

/**
 * Get the position of the sprite
 */
GPoint pge_sprite_get_position(PGESprite *this);

/**
 * Convenience function to move a sprite instead of explicitly setting its position
 */
void pge_sprite_move(PGESprite *this, int dx, int dy);

/**
 * Test to see if two entities are colliding
 */
bool pge_check_collision(PGESprite* sprite1, PGESprite *sprite2);

/**
 * Get the on-screen bounds of the PGESprite
 */
GRect pge_sprite_get_bounds(PGESprite *this);
