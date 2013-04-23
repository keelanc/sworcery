/*
 
 Sworcery
 
 https://github.com/keelanc/sworcery
 
 Inspired by the excellent game Sword & Sworcery
 
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "hobbit_meals.h"
#include "mini-printf.h"


#define MY_UUID { 0x13, 0xEF, 0x56, 0xB6, 0x40, 0x2E, 0x43, 0x10, 0x81, 0x9B, 0x1F, 0x98, 0x33, 0x36, 0x70, 0x92 }
PBL_APP_INFO(MY_UUID,
             "Sworcery", "keelanchufor.com",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer arch_layer;
TextLayer text_debug_layer;
TextLayer text_debug2_layer;
TextLayer text_hobbit_layer;
BmpContainer moon;
BmpContainer bg_default;
BmpContainer arch_image;

const int ARCH_IMAGE_RESOURCE_IDS[] = {
	RESOURCE_ID_ARCH_NORM,
	RESOURCE_ID_ARCH_SMOKE_ONE,
	RESOURCE_ID_ARCH_SMOKE_TWO,
	RESOURCE_ID_ARCH_SMOKE_THREE,
	RESOURCE_ID_ARCH_SMOKE_FOUR,
	RESOURCE_ID_ARCH_SMOKE_FIVE,
	RESOURCE_ID_ARCH_SMOKE_SIX,
	RESOURCE_ID_ARCH_TURN,
	RESOURCE_ID_ARCH_RAISED
};



// Can be used to cancel timer via `app_timer_cancel_event()`
AppTimerHandle timer_handle;

// Can be used to distinguish between multiple timers in your app
#define SMOKE_TIMER 1
#define FPERS 8	/* frames per second */
#define SMOKE_LOOP 20	/* loop every x seconds */

#define SPERF (1000/FPERS)
#define FPERL (SMOKE_LOOP * FPERS)

#define ARCH_POS GRect(86, 87, 144-100, 168-87)

static int animation_frame = 0;
bool animateNow = false;

static char debug_text[] = "02:55:02 pm";
static char debug2_text[] = "frame: XX";
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


//
// Main image setting routine.
// Used for every image swap in the app
//

void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin, Layer *targetLayer) {
	/*
	 Main image setting routine.
	 Used for every image swap in the app
	 Borrowed from https://github.com/xretsim/pebble_steampebble_mark1
	 */
	
	layer_remove_from_parent(&bmp_container->layer.layer);            //remove it from layer so it can be safely deinited
	bmp_deinit_container(bmp_container);                              //deinit the old image.
	
	bmp_init_container(resource_id, bmp_container);                   //init the container with the new image
	
	GRect frame = layer_get_frame(&bmp_container->layer.layer);       //posiiton the new image with the supplied coordinates.
	frame.origin.x = origin.x;
	frame.origin.y = origin.y;
	layer_set_frame(&bmp_container->layer.layer, frame);
	
	layer_add_child(targetLayer, &bmp_container->layer.layer);        //add the new image to the target layer.
}


void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	(void)ctx;
	(void)handle;
	
	if (cookie == SMOKE_TIMER) {
		// animation sequence
		if (animation_frame == 1) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[1], GPoint(86, 87), &arch_layer);
		}
		else if (animation_frame == 27) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[2], GPoint(86, 87), &arch_layer);
		}
		else if (animation_frame == 3) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[3], GPoint(86, 87), &arch_layer);
		}
		else if (animation_frame == 4) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[4], GPoint(86, 87), &arch_layer);
		}
		else if (animation_frame == 5 || animation_frame == 26) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[5], GPoint(86, 87), &arch_layer);
		}
		else if (animation_frame >= 6 && animation_frame <= 25) {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[6], GPoint(86, 87), &arch_layer);
		}
		else {
			set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(86, 87), &arch_layer);
			
			if (animation_frame >= 29) {
				animation_frame = 0;
				animateNow = false;
			}
		}
		animation_frame++;
		
	}
	if (animateNow) {
		timer_handle = app_timer_send_event(ctx, SPERF /* milliseconds */, SMOKE_TIMER);
	}
	
}


void update_watchface(PblTm* t) {
	
	string_format_time(debug_text, sizeof(debug_text), "%r", t);
	text_layer_set_text(&text_debug_layer, debug_text);
	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", animation_frame);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", t->tm_sec + 10);
	text_layer_set_text(&text_debug2_layer, debug2_text);
	
	hobbit_time(t->tm_hour, hobbit_hour);
//	text_layer_set_text(&text_hobbit_layer, hobbit_hour);
	text_layer_set_text(&text_hobbit_layer, "ELEVEN\nTWENTY SEVEN");
	
}


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
	
	(void)ctx;
	
	unsigned short display_second = t->tick_time->tm_sec;
	
	// play smoke animation every SMOKE_LOOP seconds
	if ((display_second % SMOKE_LOOP) == 0) {
		animateNow = true;
		timer_handle = app_timer_send_event(ctx, SPERF, SMOKE_TIMER);
	}
	
	// arch_turn every half SMOKE_LOOP for 2 seconds
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[7], GPoint(86, 87), &arch_layer);
	}
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 + 2) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(86, 87), &arch_layer);
	}
	
	update_watchface(t->tick_time);
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
	(void)ctx;
	
	window_init(&window, "Sworcery watch");
	window_stack_push(&window, true /* Animated */);
//	window_set_background_color(&window, GColorBlack);
	
	// init the archetype layer
	layer_init(&arch_layer, window.layer.frame);
	layer_add_child(&window.layer, &arch_layer);
	
	
	resource_init_current_app(&APP_RESOURCES);
//	bmp_init_container(RESOURCE_ID_BG_DEFAULT, &bg_default);
	bmp_init_container(RESOURCE_ID_MOON, &moon);
	
	// default background includes arch_norm
//	layer_add_child(&arch_layer, &bg_default.layer.layer);
	layer_add_child(&window.layer, &moon.layer.layer);
	
	// init the debug text layer
	text_layer_init(&text_debug_layer, GRect(0, 0, 144, 30));
	text_layer_set_text_alignment(&text_debug_layer, GTextAlignmentRight);
//	text_layer_set_text_color(&text_debug_layer, GColorWhite);
	text_layer_set_background_color(&text_debug_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug_layer.layer);
	
	// init the 2nd debug text layer
	text_layer_init(&text_debug2_layer, GRect(0, 20, 144, 30));
	text_layer_set_text_alignment(&text_debug2_layer, GTextAlignmentRight);
//	text_layer_set_text_color(&text_debug2_layer, GColorWhite);
	text_layer_set_background_color(&text_debug2_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug2_layer.layer);
	
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
	
	timer_handle = app_timer_send_event(ctx, SPERF, SMOKE_TIMER);
}


void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	
	bmp_deinit_container(&arch_image);
	bmp_deinit_container(&moon);
	bmp_deinit_container(&bg_default);
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,
		.timer_handler = &handle_timer,
		.tick_info = {
			.tick_handler = &handle_second_tick,
			.tick_units = SECOND_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
