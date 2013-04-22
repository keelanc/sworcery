/*
 
 Sworcery
 
 https://github.com/keelanc/sworcery
 
 Inspired by the excellent game Sword & Sworcery
 
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "hobbit_meals.h"


#define MY_UUID { 0x13, 0xEF, 0x56, 0xB6, 0x40, 0x2E, 0x43, 0x10, 0x81, 0x9B, 0x1F, 0x98, 0x33, 0x36, 0x70, 0x92 }
PBL_APP_INFO(MY_UUID,
             "Sworcery", "keelanchufor.com",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer arch_layer;
Layer animation_layer;
TextLayer text_debug_layer;
TextLayer text_hobbit_layer;
BmpContainer moon;
BmpContainer bg_default;
BmpContainer arch_norm;
BmpContainer arch_turn;
BmpContainer arch_smoke1;
BmpContainer arch_smoke2;
BmpContainer arch_smoke3;
BmpContainer arch_smoke4;
BmpContainer arch_smoke5;
BmpContainer arch_smoke6;
BmpContainer arch_raised;


// Can be used to cancel timer via `app_timer_cancel_event()`
AppTimerHandle timer_handle;

// Can be used to distinguish between multiple timers in your app
#define SMOKE_TIMER 1
#define FPERS 5	/* frames per second */
#define SMOKE_LOOP 15	/* loop every x seconds */

#define SPERF (1000/FPERS)
#define FPERL (SMOKE_LOOP * FPERS)

#define ARCH_POS GRect(86, 87, 144-100, 168-87)

static int animation_frame = 0;


static char debug_text[] = "02:55:02 pm";
static char hobbit_hour[] = "something quite long";

// unused thus far
unsigned short get_display_hour(unsigned short hour) {
	
	if (clock_is_24h_style()) {
		return hour;
	}
	
	// convert 24hr to 12hr
	unsigned short display_hour = hour % 12;
	// Converts "0" to "12"
	return display_hour ? display_hour : 12;
	
}


void animationlayer_update_callback(Layer *me, GContext* ctx) {
	(void)me;
	
	PblTm t;
	get_time(&t);
	
	// unused thus far
	unsigned short display_hour = get_display_hour(t.tm_hour);
	
	
	// sync animation with the time
	if (t.tm_sec % SMOKE_LOOP == 0) {
//	if (t.tm_sec == 0) {
		animation_frame = 0;
	}
	
/*	switch (animation_frame) {
		case 1:
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke1.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 17:
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke2.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 3:
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke3.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 4:
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke4.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 5: case 16:
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke5.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: 
			graphics_draw_bitmap_in_rect(ctx, &arch_smoke6.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		case 33: case 34: case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42:
			graphics_draw_bitmap_in_rect(ctx, &arch_turn.bmp, GRect(86, 87, 144-100, 168-87));
			break;
		default:
			graphics_draw_bitmap_in_rect(ctx, &arch_norm.bmp, GRect(86, 87, 144-100, 168-87));
			break;
	}
*/
	if (animation_frame == 1) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke1.bmp, ARCH_POS);
	}
	else if (animation_frame == 17) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke2.bmp, ARCH_POS);
	}
	else if (animation_frame == 3) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke3.bmp, ARCH_POS);
	}
	else if (animation_frame == 4) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke4.bmp, ARCH_POS);
	}
	else if (animation_frame == 5 || animation_frame == 16) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke5.bmp, ARCH_POS);
	}
	else if (animation_frame >= 6 && animation_frame <= 15) {
		graphics_draw_bitmap_in_rect(ctx, &arch_smoke6.bmp, ARCH_POS);
	}
	else if (animation_frame >= 33 && animation_frame <= 42) {
		graphics_draw_bitmap_in_rect(ctx, &arch_turn.bmp, ARCH_POS);
	}
	else {
		graphics_draw_bitmap_in_rect(ctx, &arch_norm.bmp, ARCH_POS);
	}
//	animation_frame = (animation_frame + 1) % FPERL;
	animation_frame ++;
	
}


void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	(void)ctx;
	(void)handle;
	
	if (cookie == SMOKE_TIMER) {
		layer_mark_dirty(&animation_layer);
		timer_handle = app_timer_send_event(ctx, SPERF /* milliseconds */, SMOKE_TIMER);
	}
	
}


void update_watchface(PblTm* t) {
	
	string_format_time(debug_text, sizeof(debug_text), "%r", t);
	text_layer_set_text(&text_debug_layer, debug_text);
	
	hobbit_time(t->tm_hour, hobbit_hour);
//	text_layer_set_text(&text_hobbit_layer, hobbit_hour);
	text_layer_set_text(&text_hobbit_layer, "ELEVEN\nTWENTY SEVEN");
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
//	(void)ctx;
	
	window_init(&window, "Sworcery watch");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	// init the archetype layer
	layer_init(&arch_layer, window.layer.frame);
//	arch_layer.update_proc = &arch_layer_update_callback; // REF: .update_proc points to a function that draws the layer
	layer_add_child(&window.layer, &arch_layer);
	
	
	resource_init_current_app(&APP_RESOURCES);
	bmp_init_container(RESOURCE_ID_BG_DEFAULT, &bg_default);
	bmp_init_container(RESOURCE_ID_MOON, &moon);
	bmp_init_container(RESOURCE_ID_ARCH_NORM, &arch_norm);
	bmp_init_container(RESOURCE_ID_ARCH_TURN, &arch_turn);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_ONE, &arch_smoke1);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_TWO, &arch_smoke2);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_THREE, &arch_smoke3);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_FOUR, &arch_smoke4);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_FIVE, &arch_smoke5);
	bmp_init_container(RESOURCE_ID_ARCH_SMOKE_SIX, &arch_smoke6);
	bmp_init_container(RESOURCE_ID_ARCH_RAISED, &arch_raised);
	
	// default background includes arch_norm
	layer_add_child(&arch_layer, &bg_default.layer.layer);
	layer_add_child(&arch_layer, &moon.layer.layer);
	
	// animation stuff on arch_layer
	layer_add_child(&arch_layer, &animation_layer);
	animation_layer.update_proc = &animationlayer_update_callback;
	
	// init the debug text layer
	text_layer_init(&text_debug_layer, GRect(0, 0, 144, 30));
	text_layer_set_text_alignment(&text_debug_layer, GTextAlignmentRight);
	text_layer_set_text_color(&text_debug_layer, GColorWhite);
	text_layer_set_background_color(&text_debug_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug_layer.layer);
	
	// init the hobbit text layer
	text_layer_init(&text_hobbit_layer, GRect(0, 50, 144, 60));
	text_layer_set_font(&text_hobbit_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(&text_hobbit_layer, GTextAlignmentCenter);
	text_layer_set_text_color(&text_hobbit_layer, GColorWhite);
	text_layer_set_background_color(&text_hobbit_layer, GColorClear);
	layer_add_child(&window.layer, &text_hobbit_layer.layer);
	
	// load watchface immediately
	PblTm t;
	get_time(&t);
	update_watchface(&t);
	
	timer_handle = app_timer_send_event(ctx, SPERF /* milliseconds */, SMOKE_TIMER);
}


void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	
	bmp_deinit_container(&arch_norm);
	bmp_deinit_container(&arch_turn);
	bmp_deinit_container(&arch_smoke1);
	bmp_deinit_container(&arch_smoke2);
	bmp_deinit_container(&arch_raised);
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
	// doing something on the second
	
	(void)ctx;
	
//	layer_mark_dirty(&animation_layer);
	
	update_watchface(t->tick_time);
	
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,
		.timer_handler = &handle_timer,
		.tick_info = {
			.tick_handler = &handle_tick,
			.tick_units = SECOND_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
