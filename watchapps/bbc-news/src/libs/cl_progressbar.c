#include "cl_progressbar.h"

static void update_proc(Layer *layer, GContext *ctx)
{
  //Draw border
  GRect outer = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, outer, 0, GCornerNone);

  //Fill background of bar
  GRect inner = GRect(
    outer.origin.x + 1,
    outer.origin.y + 1,
    outer.size.w - 2,
    outer.size.h - 2
  );
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, inner, 0, GCornerNone);

  //Draw progress as bar
  void *ptr = layer_get_data(layer);
  int progress = *((int*)ptr);
  int width = (int)(((float)progress / 100.0F) * (float)inner.size.w);
  GRect bar = GRect(
    outer.origin.x + 1,
    outer.origin.y + 1,
    width,
    outer.size.h - 2
  );
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bar, 0, GCornerNone);
}

ProgressBarLayer* progressbar_layer_create(GRect bounds)
{
  ProgressBarLayer *this = malloc(sizeof(ProgressBarLayer));
  this->layer = layer_create_with_data(bounds, sizeof(int));
  layer_set_update_proc(this->layer, (LayerUpdateProc)update_proc);
  
  void *progress = layer_get_data(this->layer);
  *((int*)progress) = 0;

  return this;
}

void progressbar_layer_destroy(ProgressBarLayer *this)
{
  layer_destroy(this->layer);
  free(this);
}

void progressbar_layer_set_progress(ProgressBarLayer *this, int new_progress)
{
  //Limit value to possible range
  if(new_progress < 0)
  {
    new_progress = 0;
  }
  else if(new_progress > 100)
  {
    new_progress = 100;
  }

  //Store value
  void *progress = layer_get_data(this->layer);
  *((int*)progress) = new_progress;

  //Re-draw
  layer_mark_dirty(this->layer);
}

int progressbar_layer_get_progress(ProgressBarLayer *this)
{
  void *progress = layer_get_data(this->layer);
  
  return *((int*)progress);
}

Layer* progressbar_layer_get_layer(ProgressBarLayer *this)
{
  return this->layer;
}