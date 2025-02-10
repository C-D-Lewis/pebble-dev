#include "digit.h"

static GPoint i2xy(int i) {
  return GPoint(
    i % 3,
    i / 3
  );
}

Digit* digit_create(GPoint origin, int initial_value) {
  Digit *this = (Digit*)malloc(sizeof(Digit));

  for(int i = 0; i < 15; i++) {
    GPoint p = i2xy(i);
    Vec3 vec = Vec3(p.x, p.y, BASE_Y);
    vec.x *= BLOCK_SIZE;
    vec.y *= BLOCK_SIZE;
    vec.x += origin.x;
    vec.y += origin.y;
    this->segments[i] = segment_create(vec);
  }

  this->origin = origin;
  this->value = initial_value;
  digit_set_value(this, 0);

  return this;
}

void digit_destroy(Digit *this) {
  for(int i = 0; i < 15; i++) {
    segment_destroy(this->segments[i]);
  }
  free(this);
}

static void set_digit_states(Digit *this, int arr[]) {
  for(int i = 0; i < 15; i++) {
    segment_set_visible(this->segments[i], (arr[i] == 1), i);
  }
}

void digit_set_value(Digit *this, int new_value) {
  this->value = new_value;

  // Update segments
  switch(this->value) {
    case -1:
      set_digit_states(this, (int[]){
        0, 0, 0, 
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0});
      break;
    case 0: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1});
      break;
    case 1: 
      set_digit_states(this, (int[]){
        0, 0, 1, 
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1});
      break;
    case 2: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1});
      break;
    case 3: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        0, 0, 1,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1});
      break;
    case 4: 
      set_digit_states(this, (int[]){
        1, 0, 1, 
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1});
      break;
    case 5: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1});
      break;
    case 6: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1});
      break;
    case 7: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1});
      break;
    case 8: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1});
      break;
    case 9: 
      set_digit_states(this, (int[]){
        1, 1, 1, 
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1});
      break;
  }
}

void digit_logic(Digit *this) {
  for(int i = 0; i < 15; i++) {
    segment_logic(this->segments[i]);
  }
}

void digit_render(Digit *this) {
  for(int i = 0; i < 15; i++) {
    segment_render(this->segments[i]);
  }
}

void digit_set_colors(Digit *this, GColor side_color, GColor face_color) {
  for(int i = 0; i < 15; i++) {
    segment_set_colors(this->segments[i], side_color, face_color);
  }
}

bool digit_is_animating(Digit *this) {
  for(int i = 0; i < 15; i++) {
    if(!segment_is_at_rest(this->segments[i])) {
      return true;
    }
  }
  return false;
}
