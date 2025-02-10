#include "SevenSegment.h"

SevenSegment* seven_segment_create(GPoint origin, GSize segment_dimensions, int anim_duration, int seg_delay, Window *parent)
{
  SevenSegment *this = malloc(sizeof(SevenSegment));
  this->origin = origin;
  this->seg_dims = segment_dimensions;
  this->anim_duration = anim_duration;
  this->seg_delay = seg_delay;

  for(int i = 0; i < 7; i++)
  {
    this->layer_array[i] = inverter_layer_create(GRect(0, 0, 0, 0));

    layer_add_child(window_get_root_layer(parent), inverter_layer_get_layer(this->layer_array[i]));
  }

  //Init to 0
  seven_segment_set_value(this, 0);
  this->last_state[0] = 1;
  this->last_state[1] = 1;
  this->last_state[2] = 1;
  this->last_state[3] = 1;
  this->last_state[4] = 1;
  this->last_state[5] = 1;
  this->last_state[6] = 0;

  return this;
}

void seven_segment_destroy(SevenSegment *this)
{
  for(int i = 0; i < 7; i++)
  {
    inverter_layer_destroy(this->layer_array[i]);
  }

  free(this);
}

static void animation_stopped_handler(Animation *animation, bool finished, void *context)
{
  property_animation_destroy((PropertyAnimation*)animation);
}

static void animate(Layer *layer, SevenSegment *seg, GRect start, GRect finish, int position)
{
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*)anim, seg->anim_duration);
  animation_set_curve((Animation*)anim, AnimationCurveEaseInOut);
  animation_set_delay((Animation*)anim, position * seg->seg_delay); //Staggered animation effect
  animation_set_handlers((Animation*)anim, (AnimationHandlers) {
    .stopped = animation_stopped_handler
  }, NULL);
  animation_schedule((Animation*)anim);
}

static void animate_segments(SevenSegment *this, int arr[7])
{
  GPoint origin = this->origin;

  for(int i = 0; i < 7; i++)
  {
    // Only animate a change of state!
    if(this->last_state[i] != arr[i])
    {
      switch(arr[i])
      {
      case 1:
        // ON
        switch(i)
        {
        case 0:
          animate(inverter_layer_get_layer(this->layer_array[0]), this,
            GRect(origin.x + this->seg_dims.h, origin.y, 0, this->seg_dims.h), 
            GRect(origin.x + this->seg_dims.h, origin.y, this->seg_dims.w, this->seg_dims.h),
          0);
          break;
        case 1:
          animate(inverter_layer_get_layer(this->layer_array[1]), this,
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + this->seg_dims.h, this->seg_dims.h, 0), 
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + this->seg_dims.h, this->seg_dims.h, this->seg_dims.w),
          1);
          break;
        case 2:
          animate(inverter_layer_get_layer(this->layer_array[2]), this,
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, 0), 
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, this->seg_dims.w),
          2);
          break;
        case 3:
          animate(inverter_layer_get_layer(this->layer_array[3]), this,
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.w) + (2 * this->seg_dims.h), 0, this->seg_dims.h), 
            GRect(origin.x + this->seg_dims.h, origin.y + (2 * this->seg_dims.w) + (2 * this->seg_dims.h), this->seg_dims.w, this->seg_dims.h),
          3);
          break;
        case 4:
          animate(inverter_layer_get_layer(this->layer_array[4]), this,
            GRect(origin.x, origin.y + (2 * this->seg_dims.h) + (2 * this->seg_dims.w), this->seg_dims.h, 0), 
            GRect(origin.x, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, this->seg_dims.w),
          4);
          break;
        case 5:
          animate(inverter_layer_get_layer(this->layer_array[5]), this,
            GRect(origin.x, origin.y + this->seg_dims.h + this->seg_dims.w, this->seg_dims.h, 0), 
            GRect(origin.x, origin.y + this->seg_dims.h, this->seg_dims.h, this->seg_dims.w),
          5);
          break;
        case 6:
          animate(inverter_layer_get_layer(this->layer_array[6]), this,
            GRect(origin.x + this->seg_dims.h, origin.y + this->seg_dims.w + this->seg_dims.h, 0, this->seg_dims.h), 
            GRect(origin.x + this->seg_dims.h, origin.y + this->seg_dims.w + this->seg_dims.h, this->seg_dims.w, this->seg_dims.h),
          6);
          break;
        }
        break;
      case 0:
        //OFF
        switch(i)
        {
        case 0:
          animate(inverter_layer_get_layer(this->layer_array[0]), this,
            GRect(origin.x + this->seg_dims.h, origin.y, this->seg_dims.w, this->seg_dims.h),
            GRect(origin.x + this->seg_dims.h, origin.y, 0, this->seg_dims.h), 
          0);
          break;
        case 1:
          animate(inverter_layer_get_layer(this->layer_array[1]), this,
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + this->seg_dims.h, this->seg_dims.h, this->seg_dims.w),
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + this->seg_dims.h, this->seg_dims.h, 0), 
          1);
          break;
        case 2:
          animate(inverter_layer_get_layer(this->layer_array[2]), this,
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, this->seg_dims.w),
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, 0), 
          2);
          break;
        case 3:
          animate(inverter_layer_get_layer(this->layer_array[3]), this,
            GRect(origin.x + this->seg_dims.h, origin.y + (2 * this->seg_dims.w) + (2 * this->seg_dims.h), this->seg_dims.w, this->seg_dims.h),
            GRect(origin.x + this->seg_dims.h + this->seg_dims.w, origin.y + (2 * this->seg_dims.w) + (2 * this->seg_dims.h), 0, this->seg_dims.h), 
          3);
          break;
        case 4:
          animate(inverter_layer_get_layer(this->layer_array[4]), this,
            GRect(origin.x, origin.y + (2 * this->seg_dims.h) + this->seg_dims.w, this->seg_dims.h, this->seg_dims.w),
            GRect(origin.x, origin.y + (2 * this->seg_dims.h) + (2 * this->seg_dims.w), this->seg_dims.h, 0), 
          4);
          break;
        case 5:
          animate(inverter_layer_get_layer(this->layer_array[5]), this,
            GRect(origin.x, origin.y + this->seg_dims.h, this->seg_dims.h, this->seg_dims.w),
            GRect(origin.x, origin.y + this->seg_dims.h + this->seg_dims.w, this->seg_dims.h, 0), 
          5);
          break;
        case 6:
          animate(inverter_layer_get_layer(this->layer_array[6]), this,
            GRect(origin.x + this->seg_dims.h, origin.y + this->seg_dims.w + this->seg_dims.h, this->seg_dims.w, this->seg_dims.h),
            GRect(origin.x + this->seg_dims.h, origin.y + this->seg_dims.w + this->seg_dims.h, 0, this->seg_dims.h), 
          6);
          break;
        }
        break;
      }
    }
  }

  //Store last stage as new state
  for(int i = 0; i < 7; i++)
  {
    this->last_state[i] = arr[i];
  }
}

void seven_segment_set_value(SevenSegment *this, int value)
{
  this->value = value;

  //Which digit to show?
  switch(value)
  {
  case 0:
    animate_segments(this, (int[7]) {1, 1, 1, 1, 1, 1, 0});
    break;
  case 1:
    animate_segments(this, (int[7]) {0, 1, 1, 0, 0, 0, 0});
    break;
  case 2:
    animate_segments(this, (int[7]) {1, 1, 0, 1, 1, 0, 1});
    break;
  case 3:
    animate_segments(this, (int[7]) {1, 1, 1, 1, 0, 0, 1});
    break;
  case 4:
    animate_segments(this, (int[7]) {0, 1, 1, 0, 0, 1, 1});
    break;
  case 5:
    animate_segments(this, (int[7]) {1, 0, 1, 1, 0, 1, 1});
    break;
  case 6:
    animate_segments(this, (int[7]) {1, 0, 1, 1, 1, 1, 1});
    break;
  case 7:
    animate_segments(this, (int[7]) {1, 1, 1, 0, 0, 0, 0});
    break;
  case 8:
    animate_segments(this, (int[7]) {1, 1, 1, 1, 1, 1, 1});
    break;
  case 9:
    animate_segments(this, (int[7]) {1, 1, 1, 0, 0, 1, 1});
    break;
  case 10:
    animate_segments(this, (int[7]) {1, 1, 1, 0, 1, 1, 1});
    break;
  case 11:
    animate_segments(this, (int[7]) {0, 0, 1, 1, 1, 1, 1});
    break;
  case 12:
    animate_segments(this, (int[7]) {1, 0, 0, 1, 1, 1, 0});
    break;
  default:
    //Blank display
    animate_segments(this, (int[7]) {0, 0, 0, 0, 0, 0, 0});
    break;
  }
}

int seven_segment_get_value(SevenSegment *this)
{
  return this->value;
}