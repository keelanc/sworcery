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
Layer lbrace_layer;
Layer rbrace_layer;
TextLayer brace_hider_layer;
Layer moon_layer;
TextLayer text_debug_layer;
TextLayer text_debug2_layer;
TextLayer text_hobbit_layer;
BmpContainer moon;
BmpContainer lbrace;
BmpContainer rbrace;
BmpContainer bg_default;	/*its use is deprecated but removing the line crashes the app :S */
BmpContainer arch_image;
static PropertyAnimation braces_animation[2];

const int ARCH_IMAGE_RESOURCE_IDS[] = {
	RESOURCE_ID_ARCH_NORM,
	RESOURCE_ID_ARCH_SMOKE_1,
	RESOURCE_ID_ARCH_SMOKE_2,
	RESOURCE_ID_ARCH_SMOKE_3,
	RESOURCE_ID_ARCH_SMOKE_4,
	RESOURCE_ID_ARCH_SMOKE_5,
	RESOURCE_ID_ARCH_SMOKE_6,	/* pause here */
	RESOURCE_ID_ARCH_SMOKE_7,
	RESOURCE_ID_ARCH_SMOKE_8,
	RESOURCE_ID_ARCH_SMOKE_9,
	RESOURCE_ID_ARCH_TURN,
	RESOURCE_ID_ARCH_RAISED_1,
	RESOURCE_ID_ARCH_RAISED_2,
	RESOURCE_ID_ARCH_RAISED_3,
	RESOURCE_ID_ARCH_RAISED_4,
	RESOURCE_ID_ARCH_RAISED_5,
	RESOURCE_ID_ARCH_RAISED_6,
	RESOURCE_ID_ARCH_RAISED_7,
	RESOURCE_ID_ARCH_RAISED_8	/* pause here */
};

static int smoke_ani[] = {
	0,1,2,3,4,5,6,
	6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,
	7,8,9,0
};

static int smoke_ani_length = sizeof(smoke_ani) / sizeof(smoke_ani[0]);

static int raised_ani[] = {
	0,11,12,13,14,15,16,17,18,
	18,18,18,18,18,18,18,18,18,18,
	18,18,18,18,18,18,18,18,18,18,
	17,16,15,14,13,12,11,0
};

static int raised_ani_length = sizeof(raised_ani) / sizeof(raised_ani[0]);


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
bool bracesOpen = true;			// needs to start as 'true' or goofy stuff happens

static char debug_text[] = "02:55:02 pm";
//static char debug2_text[] = "frame: XX";
//static char debug2_text[] = "smoke_ani_length: XX";
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


void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin, Layer *targetLayer) {
	
	layer_remove_from_parent(&bmp_container->layer.layer);
	bmp_deinit_container(bmp_container);
	
	bmp_init_container(resource_id, bmp_container);
	
	GRect frame = layer_get_frame(&bmp_container->layer.layer);
	frame.origin.x = origin.x;
	frame.origin.y = origin.y;
	layer_set_frame(&bmp_container->layer.layer, frame);
	
	layer_add_child(targetLayer, &bmp_container->layer.layer);
}


void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	(void)ctx;
	(void)handle;
	
	if (cookie == SMOKE_TIMER) {
		// animation sequence	
		
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[smoke_ani[animation_frame]], GPoint(86, 87), &arch_layer);
		animation_frame++;
		if (animation_frame >= smoke_ani_length) {
			animation_frame = 0;
			animateNow = false;
		}
		
	}
	if (animateNow) {
		timer_handle = app_timer_send_event(ctx, SPERF /* milliseconds */, SMOKE_TIMER);
	}
	
}


void update_watchface(PblTm* t) {
	/*
	 update text layers
	 */
	
	string_format_time(debug_text, sizeof(debug_text), "%r", t);
	text_layer_set_text(&text_debug_layer, debug_text);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", animation_frame);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", t->tm_sec + 10);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "smoke_ani_length: %d", smoke_ani_length);
//	text_layer_set_text(&text_debug2_layer, debug2_text);
	text_layer_set_text(&text_debug2_layer, (bracesOpen ? "bracesOpen:  true" : "bracesOpen: false"));
	
	hobbit_time(t->tm_hour, hobbit_hour);
//	text_layer_set_text(&text_hobbit_layer, hobbit_hour);
//	text_layer_set_text(&text_hobbit_layer, "ELEVEN\nTWENTY SEVEN");

	if (!bracesOpen) {
		text_layer_set_text(&text_hobbit_layer, "");
		text_layer_set_background_color(&brace_hider_layer, GColorBlack);
	}
	else if (t->tm_sec % 15 == 2) {
		text_layer_set_text(&text_hobbit_layer, "ELEVEN\nTWENTY SEVEN");
		text_layer_set_background_color(&brace_hider_layer, GColorClear);
	}
/*	
	if (t->tm_sec % 15 == 0 || t->tm_sec % 15 == 1) {
		text_layer_set_text(&text_hobbit_layer, "");
		
		text_layer_set_background_color(&brace_hider_layer, GColorBlack);
	}
	else if (t->tm_sec % 15 == 2) {
		text_layer_set_background_color(&brace_hider_layer, GColorClear);
	}*/
}

void animation_stopped(Animation *animation, void *data) {
	(void)animation;
	(void)data;
	
	bracesOpen = true;
	
	text_layer_set_text(&text_hobbit_layer, "ELEVEN\nTWENTY SEVEN");
	text_layer_set_background_color(&brace_hider_layer, GColorClear);
}


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
	/*
	 timer-based arch animation. timer_handle
	 braces animation. animation_schedule
	 */
	
	(void)ctx;
	
	unsigned short display_second = t->tick_time->tm_sec;
	
	// Play smoke animation every SMOKE_LOOP seconds except on the minute mark when the braces animate.
	// If you do that, you're gonna have a bad time.
	if ((display_second % SMOKE_LOOP) == 0 && display_second != 0 && bracesOpen) {
		animateNow = true;
		timer_handle = app_timer_send_event(ctx, SPERF, SMOKE_TIMER);
	}
	
	// arch_turn every half SMOKE_LOOP for 2 seconds
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[10], GPoint(86, 87), &arch_layer);
	}
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 + 2) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(86, 87), &arch_layer);
	}
	
	// Animate braces
	// Don't schedule the animations at the same time as the timer-based!
	if (display_second % 15 == 0 && bracesOpen) {
		bracesOpen = false;
//		animation_unschedule_all();
		property_animation_init_layer_frame(&braces_animation[1], &lbrace_layer, NULL, &GRect((144-16)/2,40,16,61));
		property_animation_init_layer_frame(&braces_animation[2], &rbrace_layer, NULL, &GRect((144-16)/2,40,16,61));
		animation_set_duration(&braces_animation[1].animation, 300);
		animation_set_duration(&braces_animation[2].animation, 300);
		animation_set_curve(&braces_animation[1].animation,AnimationCurveLinear);
		animation_set_curve(&braces_animation[2].animation,AnimationCurveLinear);		
		animation_schedule(&braces_animation[1].animation);	
		animation_schedule(&braces_animation[2].animation);
	}
	// Tried the animation_stopped callback but it messes with the timer-based animation
	if (display_second % 15 == 1 && !bracesOpen) {
		
//		animation_unschedule_all();
		property_animation_init_layer_frame(&braces_animation[1], &lbrace_layer, NULL, &GRect(0,40,16,61));
		property_animation_init_layer_frame(&braces_animation[2], &rbrace_layer, NULL, &GRect(144-16,40,16,61));
		animation_set_delay(&braces_animation[1].animation, 400);
		animation_set_delay(&braces_animation[2].animation, 400);
		animation_set_duration(&braces_animation[1].animation, 300);
		animation_set_duration(&braces_animation[2].animation, 300);
		animation_set_curve(&braces_animation[1].animation,AnimationCurveLinear);
		animation_set_curve(&braces_animation[2].animation,AnimationCurveLinear);
		
		animation_set_handlers(&braces_animation[1].animation, (AnimationHandlers) {
			.stopped = (AnimationStoppedHandler) animation_stopped
		}, &ctx);
		
		animation_schedule(&braces_animation[1].animation);		
		animation_schedule(&braces_animation[2].animation);
	}
	
	// update text layers
	update_watchface(t->tick_time);
	
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
	(void)ctx;
	
	window_init(&window, "Sworcery watch");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	// init the archetype layer
	layer_init(&arch_layer, window.layer.frame);
	layer_add_child(&window.layer, &arch_layer);
	
	
	resource_init_current_app(&APP_RESOURCES);
	bmp_init_container(RESOURCE_ID_MOON, &moon);
	bmp_init_container(RESOURCE_ID_LBRACE, &lbrace);
	bmp_init_container(RESOURCE_ID_RBRACE, &rbrace);
	
	// init moon layer and graphic
	layer_init(&moon_layer, GRect(0, 0, 25, 25));
	layer_add_child(&window.layer, &moon_layer);
	layer_add_child(&moon_layer, &moon.layer.layer);
	
	//init the braces layers
//	layer_init(&lbrace_layer, GRect(0, 40, 16, 61));
	layer_init(&lbrace_layer, GRect((144-16)/2,40,16,61));				// hide braces
	layer_add_child(&window.layer, &lbrace_layer);
	layer_add_child(&lbrace_layer, &lbrace.layer.layer);
//	layer_init(&rbrace_layer, GRect(144-16, 40, 16, 61));
	layer_init(&rbrace_layer, GRect((144-16)/2,40,16,61));				// hide braces
	layer_add_child(&window.layer, &rbrace_layer);
	layer_add_child(&rbrace_layer, &rbrace.layer.layer);
	text_layer_init(&brace_hider_layer, GRect((144-32)/2, 40, 32, 61));
//	text_layer_set_background_color(&brace_hider_layer, GColorClear);
	text_layer_set_background_color(&brace_hider_layer, GColorBlack);	// hide braces
	layer_add_child(&window.layer, &brace_hider_layer.layer);
	
	// init the debug text layer
	text_layer_init(&text_debug_layer, GRect(0, 0, 144, 30));
	text_layer_set_text_alignment(&text_debug_layer, GTextAlignmentRight);
	text_layer_set_text_color(&text_debug_layer, GColorWhite);
	text_layer_set_background_color(&text_debug_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug_layer.layer);
	
	// init the 2nd debug text layer
	text_layer_init(&text_debug2_layer, GRect(0, 20, 144, 30));
	text_layer_set_text_alignment(&text_debug2_layer, GTextAlignmentRight);
	text_layer_set_text_color(&text_debug2_layer, GColorWhite);
	text_layer_set_background_color(&text_debug2_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug2_layer.layer);
	
	// init the hobbit text layer
	text_layer_init(&text_hobbit_layer, GRect(0, 50, 144, 60));
	text_layer_set_font(&text_hobbit_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(&text_hobbit_layer, GTextAlignmentCenter);
	text_layer_set_text_color(&text_hobbit_layer, GColorWhite);
	text_layer_set_background_color(&text_hobbit_layer, GColorClear);
//	text_layer_set_text(&text_hobbit_layer, "");						// hide text
	layer_add_child(&window.layer, &text_hobbit_layer.layer);
	
	// load watchface immediately
//	PblTm t;
//	get_time(&t);
//	update_watchface(&t);
	
	bracesOpen = false;
	
	animation_unschedule_all();
	property_animation_init_layer_frame(&braces_animation[1], &lbrace_layer, NULL, &GRect(0,40,16,61));
	property_animation_init_layer_frame(&braces_animation[2], &rbrace_layer, NULL, &GRect(144-16,40,16,61));
	animation_set_delay(&braces_animation[1].animation, 600);
	animation_set_delay(&braces_animation[2].animation, 600);
	animation_set_duration(&braces_animation[1].animation, 300);
	animation_set_duration(&braces_animation[2].animation, 300);
	animation_set_curve(&braces_animation[1].animation,AnimationCurveLinear);
	animation_set_curve(&braces_animation[2].animation,AnimationCurveLinear);
	
	animation_set_handlers(&braces_animation[1].animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler) animation_stopped
	}, &ctx);
	
	animation_schedule(&braces_animation[1].animation);		
	animation_schedule(&braces_animation[2].animation);
	
//	timer_handle = app_timer_send_event(ctx, SPERF, SMOKE_TIMER);
	set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(86, 87), &arch_layer); // place default arch image
}


void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	
	bmp_deinit_container(&arch_image);
	bmp_deinit_container(&moon);
	bmp_deinit_container(&bg_default);
	bmp_deinit_container(&lbrace);
	bmp_deinit_container(&rbrace);
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