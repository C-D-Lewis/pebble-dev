/*********************************************************************/
/* Simple alert mechanism for Pebble apps                            */
/* Author: Chris Lewis                                               */
/* Modified: 21/02/14                                                */
/* Freely available to use with link to https://github.com/C-D-Lewis */
/*********************************************************************/

#include "alert.h"

//Library constants
#define MAX_ALERT_STRLEN_LENGTH 64	//For safety - largest string allowed in buffers

//Library buffers
static char alert_lib_title_buffer[32];
static char alert_lib_body_buffer[64];

//State
static bool alert_lib_is_visible;

//Library objects
static AppTimer *alert_lib_timer;
static TextLayer *alert_lib_title_layer, *alert_lib_body_layer;
static Layer *alert_lib_background_layer;

//Function prototypes
static int alert_strlen(const char *string);
static void alert_end(void *data);
static void alert_bg_update_proc(Layer *layer, GContext *ctx);

/*
 * Get length of a string
 *
 * Arguments:
 * const char *string: The string to measure
 *
 * Returns:
 * Length of the string
 */
static int alert_strlen(const char *string)
{
	int length = 0;
	for(int i = 0; i < MAX_ALERT_STRLEN_LENGTH; i++)
	{
		if(!string[i])
		{
			break;
		}
		else
		{
			length++;
		}
	}

	return length;
}

/*
 * Cancel an alert currently showing
 *
 * Arguments:
 * void *data: Nothing. A callback requirement
 */
static void alert_end(void *data)
{
	//Dissapear
	layer_remove_from_parent(alert_lib_background_layer);
	layer_remove_from_parent(text_layer_get_layer(alert_lib_title_layer));
	layer_remove_from_parent(text_layer_get_layer(alert_lib_body_layer));
	
	layer_destroy(alert_lib_background_layer);
	text_layer_destroy(alert_lib_title_layer);
	text_layer_destroy(alert_lib_body_layer);

	//Incase of repeat
	app_timer_cancel(alert_lib_timer);

	alert_lib_is_visible = false;
}

/*
 * Alert background drawing procedure
 *
 * Arguments:
 * Layer    *layer: The layer to draw to
 * GContext   *ctx: Graphics context provided by Pebble OS
 */
static void alert_bg_update_proc(Layer *layer, GContext *ctx)
{
	//Setup context
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorBlack);

	graphics_fill_rect(ctx, GRect(5, 5, 129, 132), 5, GCornersAll);
	graphics_draw_round_rect(ctx, GRect(10, 10, 114, 117), 5);

	GPoint start = GPoint(10, 35);
	GPoint finish = GPoint(123, 35);
	graphics_draw_line(ctx, start, finish);
}

/*
 * Create and show an alert 
 *
 * Arguments:
 * Window     *window: The parent window from the Pebble application
 * const char  *title: The alert title
 * const char   *body: The alert body message
 * const int duration: The duration until the message dissapears. Use alert_update to manage this.
 */
void alert_show(Window *window, const char *title, const char *body, const int duration)
{	
	//Prevent dereferencing on multiple spawns
	if(alert_lib_is_visible == false)
	{
		//Show background
		alert_lib_background_layer = layer_create(GRect(5, 5, 129, 132));
		layer_set_update_proc(alert_lib_background_layer, alert_bg_update_proc);
		layer_add_child(window_get_root_layer(window), alert_lib_background_layer);

		//Show title
		alert_lib_title_layer = text_layer_create(GRect(10, 15, 124, 30));
		text_layer_set_text_color(alert_lib_title_layer, GColorBlack);
		text_layer_set_background_color(alert_lib_title_layer, GColorClear);
		text_layer_set_font(alert_lib_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
		text_layer_set_text_alignment(alert_lib_title_layer, GTextAlignmentCenter);
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(alert_lib_title_layer));

		//Show body
		alert_lib_body_layer = text_layer_create(GRect(20, 40, 104, 112));
		text_layer_set_text_color(alert_lib_body_layer, GColorBlack);
		text_layer_set_background_color(alert_lib_body_layer, GColorClear);
		text_layer_set_text_alignment(alert_lib_body_layer, GTextAlignmentLeft);
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(alert_lib_body_layer));

		//Set text
		snprintf(alert_lib_title_buffer, (alert_strlen(title) * sizeof(char) + 1), "%s", title);
		text_layer_set_text(alert_lib_title_layer, alert_lib_title_buffer);

		snprintf(alert_lib_body_buffer, (alert_strlen(body) * sizeof(char) + 1), "%s", body);
		text_layer_set_text(alert_lib_body_layer, alert_lib_body_buffer);

		if (duration >= 0) {
			//Register timer
			alert_lib_timer = app_timer_register(duration, alert_end, NULL);
		}

		alert_lib_is_visible = true;
	}
}

/*
 * Create and show an alert that does not disappear automatically
 *
 * Arguments:
 * Window     *window: The parent window from the Pebble application
 * const char  *title: The alert title
 * const char   *body: The alert body message
 */
void alert_show_sticky(Window *window, const char *title, const char *body) {
	alert_show(window, title, body, -1);
}

/*
 * Update an existing alert with more details
 *
 * Arguments:
 * const char    *new_title: Updated alert title
 * const char     *new_body: Updated alert body message
 * const  int *new_duration: The new duration until the alert dissapears
 */
void alert_update(const char *new_title, const char *new_body, const int new_duration)
{
	if(alert_lib_is_visible == true)
	{
		//Set text
		snprintf(alert_lib_title_buffer, (alert_strlen(new_title) * sizeof(char) + 1), "%s", new_title);
		text_layer_set_text(alert_lib_title_layer, alert_lib_title_buffer);

		snprintf(alert_lib_body_buffer, (alert_strlen(new_body) * sizeof(char) + 1), "%s", new_body);
		text_layer_set_text(alert_lib_body_layer, alert_lib_body_buffer);

		//New duration
		app_timer_cancel(alert_lib_timer);
		alert_lib_timer = app_timer_register(new_duration, alert_end, NULL);
	}
}

/*
 * Cancel an existing alert. Convenience due to callback argument
 */
void alert_cancel()
{
	if(alert_lib_is_visible == true)
	{
		alert_end(NULL);	//NULL - signature requirement
	}
}

/*
 * Tell the parent app if the alert is currently visible
 *
 * Returns
 * bool: True if the alert is still visible
 */
bool alert_is_visible()
{
	return alert_lib_is_visible;
}