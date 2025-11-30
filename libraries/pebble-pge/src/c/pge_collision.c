#include "pge_collision.h"

#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)

// Uses separated axis theorem where rectangles are aligned with the x and y axis
bool pge_collision_rectangle_rectangle(GRect *rect_a, GRect *rect_b) {

  struct overlap_rect{  
    GPoint ul;  // upper-left
    GPoint ur;  // upper-right
    GPoint bl;  // bottom-left
    GPoint br;  // bottom-right
  };

  struct overlap_rect overlap_a = (struct overlap_rect){
    {rect_a->origin.x, rect_a->origin.y},
    {rect_a->origin.x + rect_a->size.w, rect_a->origin.y},
    {rect_a->origin.x, rect_a->origin.y + rect_a->size.h},
    {rect_a->origin.x + rect_a->size.w, rect_a->origin.y + rect_a->size.h}};

  struct overlap_rect overlap_b = (struct overlap_rect){
    {rect_b->origin.x, rect_b->origin.y},
    {rect_b->origin.x + rect_b->size.w, rect_b->origin.y},
    {rect_b->origin.x, rect_b->origin.y + rect_b->size.h},
    {rect_b->origin.x + rect_b->size.w, rect_b->origin.y + rect_b->size.h}};

   GPoint a_min = GPoint(
      min( min( overlap_a.ul.x,overlap_a.ur.x),min( overlap_a.bl.x,overlap_a.br.x ) ),
      min( min( overlap_a.ul.y,overlap_a.ur.y),min( overlap_a.bl.y,overlap_a.br.y ) ) );
   GPoint a_max = GPoint(
      max( max( overlap_a.ul.x,overlap_a.ur.x),max( overlap_a.bl.x,overlap_a.br.x ) ),
      max( max( overlap_a.ul.y,overlap_a.ur.y),max( overlap_a.bl.y,overlap_a.br.y ) ) );
   GPoint b_min = GPoint(
      min( min( overlap_b.ul.x,overlap_b.ur.x),min( overlap_b.bl.x,overlap_b.br.x ) ),
      min( min( overlap_b.ul.y,overlap_b.ur.y),min( overlap_b.bl.y,overlap_b.br.y ) ) );
   GPoint b_max = GPoint(
      max( max( overlap_b.ul.x,overlap_b.ur.x),max( overlap_b.bl.x,overlap_b.br.x ) ),
      max( max( overlap_b.ul.y,overlap_b.ur.y),max( overlap_b.bl.y,overlap_b.br.y ) ) );
   
   //checks if cannot overlap, returns true if overlapped
   return !( a_min.x > b_max.x || a_max.x < b_min.x || 
      a_min.y > b_max.y || a_max.y < b_min.y );
}

bool pge_collision_line_rectangle(GLine *line, GRect *rect) {
  bool retval = false;

  retval |= pge_collision_line_line( line, &(GLine){
        {rect->origin.x, rect->origin.y}, 
        {rect->origin.x + rect->size.w, rect->origin.y}});

  retval |= pge_collision_line_line( line, &(GLine){
        {rect->origin.x + rect->size.w, rect->origin.y}, 
        {rect->origin.x + rect->size.w, rect->origin.y + rect->size.h}});

  retval |= pge_collision_line_line( line, &(GLine){
        {rect->origin.x + rect->size.w, rect->origin.y + rect->size.h}, 
        {rect->origin.x, rect->origin.y + rect->size.h}});

  retval |= pge_collision_line_line( line, &(GLine){
        {rect->origin.x, rect->origin.y + rect->size.h}, 
        {rect->origin.x, rect->origin.y}});

  return retval;
}

bool pge_collision_line_line(GLine *line_a, GLine *line_b) {
  // Dot product, positive results mean point is on one side of a line,
  // checking both ranges shows if intersection is within segments.
  return 
    (((line_b->p1.x-line_a->p1.x)*(line_a->p2.y-line_a->p1.y) + (line_b->p1.y-line_a->p1.y)*(line_a->p2.x-line_a->p1.x)) * ((line_b->p2.x-line_a->p1.x)*(line_a->p2.y-line_a->p1.y) + (line_b->p2.y-line_a->p1.y)*(line_a->p2.x-line_a->p1.x)) < 0) &&
    (((line_a->p1.x-line_b->p1.x)*(line_b->p2.y-line_b->p1.y) + (line_a->p1.y-line_b->p1.y)*(line_b->p2.x-line_b->p1.x)) * ((line_a->p2.x-line_b->p1.x)*(line_b->p2.y-line_b->p1.y) + (line_a->p2.y-line_b->p1.y)*(line_b->p2.x-line_b->p1.x)) < 0);
}


bool pge_collision_point_rectangle(GPoint *point, GRect *rect){
  return 
      (point->x >= rect->origin.x && point->x <= (rect->origin.x + rect->size.w)) && 
      (point->y >= rect->origin.y && point->y <= (rect->origin.y + rect->size.h));
}

