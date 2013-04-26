/*
 
 Sworcery
 
 https://github.com/keelanc/sworcery
 
 Inspired by the excellent game Sword & Sworcery
 
 
 
 Copyright (C) 2013 Keelan Chu For
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 You may contact the author at kkchufor@gmail.com
 
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <math.h>
#include "time_as_words.h"
//#include "mini-printf.h"


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
//TextLayer text_debug_layer;
//TextLayer text_debug2_layer;
TextLayer current_time_layer;
BmpContainer moon_image;
BmpContainer lbrace;
BmpContainer rbrace;
BmpContainer bg_default;	/*its use is deprecated but removing the line crashes the app :S */
BmpContainer arch_image;
static PropertyAnimation braces_animation[2];

const int MOON_IMAGE_RESOURCE_IDS[] = {
	RESOURCE_ID_MOON_0,
	RESOURCE_ID_MOON_1,
	RESOURCE_ID_MOON_2,
	RESOURCE_ID_MOON_3,
	RESOURCE_ID_MOON_4,
	RESOURCE_ID_MOON_5,
	RESOURCE_ID_MOON_6,
	RESOURCE_ID_MOON_7,
	RESOURCE_ID_MOON_8,
	RESOURCE_ID_MOON_9,
	RESOURCE_ID_MOON_10,
	RESOURCE_ID_MOON_11
};

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


AppTimerHandle timer_handle;


#define SMOKE_TIMER 1
#define RAISED_TIMER 2
#define INTRO_TIMER 3
#define FPERS 8	/* frames per second */
#define SMOKE_LOOP 15	/* loop every x seconds */

#define SPERF (1000/FPERS)
//#define FPERL (SMOKE_LOOP * FPERS)

static int animation_frame = 0;

bool animateNow = false;
bool bracesOpen = true;			// needs to start as 'true' or goofy stuff happens
bool introComplete = true;		// same deal
bool raisedNotPlaying = true;	// so that other timer-base animations don't play the same time

//static char debug_text[] = "02:55:02 pm";
//static char debug2_text[] = "frame: XX";
//static char debug2_text[] = "smoke_ani_length: XX";
static char current_time[] = "ELEVEN\nTWENTY SEVEN";







int moon_phase(int year, int yday) {
	
	// last new moon reference: Jan 11, 2013
	double days_since = (year - 2013) * 365.25 + yday - 11;
//	int phase = round(fmod(days_since, 29.53)/29.53 * 11);
	int phase = round( (days_since - 29.53 * floor(days_since/29.53)) / 29.53 * 11 );
	return phase;
}


void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin, Layer *targetLayer) {
	/*
	 swap images
	 */
	
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
	/*
	 handle timer-based animations
	 */
	
	(void)ctx;
	(void)handle;
	
	if (cookie == SMOKE_TIMER) {
		// animation sequence	
		
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[smoke_ani[animation_frame]], GPoint(0, 0), &arch_layer);
		animation_frame++;
		if (animation_frame >= smoke_ani_length) {
			animation_frame = 0;
			animateNow = false;
		}
		
	}
	
	if (cookie == RAISED_TIMER) {
		// animation sequence	
		
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[raised_ani[animation_frame]], GPoint(0, 0), &arch_layer);
		animation_frame++;
		if (animation_frame >= raised_ani_length) {
			animation_frame = 0;
			animateNow = false;
			raisedNotPlaying = true;
		}
/*		
	if (cookie == INTRO_TIMER) {
		text_layer_set_text(&text_debug2_layer, "intro_timer called");
		if (introComplete) {
			text_layer_set_text(&text_debug2_layer, "starting intro raise");
			timer_handle = app_timer_send_event(ctx, SPERF, RAISED_TIMER);
		}
		else {
			timer_handle = app_timer_send_event(ctx, 250, INTRO_TIMER);
		}

	}
*/
	
	}
	if (animateNow) {
		timer_handle = app_timer_send_event(ctx, SPERF, cookie);
	}
	
}

/*
void update_debug(PblTm* t) {
	
	string_format_time(debug_text, sizeof(debug_text), "%r", t);
	text_layer_set_text(&text_debug_layer, debug_text);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", animation_frame);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "frame: %d", t->tm_sec + 10);
//	mini_snprintf(debug2_text, sizeof(debug2_text), "smoke_ani_length: %d", smoke_ani_length);
//	text_layer_set_text(&text_debug2_layer, debug2_text);
//	text_layer_set_text(&text_debug2_layer, (bracesOpen ? "bracesOpen:  true" : "bracesOpen: false"));
//	text_layer_set_text(&text_debug2_layer, (introComplete ? "intro:  true" : "intro: false"));
//	text_layer_set_text(&text_debug2_layer, (raisedNotPlaying ? "raising:  false" : "raising: true"));
		
}
*/

void animation_stopped(Animation *animation, void *data) {
	/*
	 callback for brace-opening animation
	 */
	
	(void)animation;
	(void)data;
	
	bracesOpen = true;
	
	PblTm tick_time;
	get_time(&tick_time);
	time_as_words(tick_time.tm_hour, tick_time.tm_min, current_time);
	text_layer_set_text(&current_time_layer, current_time);
	text_layer_set_background_color(&brace_hider_layer, GColorClear);
}


void intro_animation_stopped(Animation *animation, void *data) {
	/*
	 callback for the intro-brace-opening animation
	 */
	
	(void)animation;
	(void)data;
	
	bracesOpen = true;
	
//	text_layer_set_text(&current_time_layer, "THE INTRO\nIS COMPLETE");
	PblTm tick_time;
	get_time(&tick_time);
	time_as_words(tick_time.tm_hour, tick_time.tm_min, current_time);
	text_layer_set_text(&current_time_layer, current_time);
	text_layer_set_background_color(&brace_hider_layer, GColorClear);
	
	introComplete = true;
//	text_layer_set_text(&text_debug2_layer, "intro: true");
}


void handle_hour_tick(AppContextRef ctx, PebbleTickEvent *t) {
	(void)ctx;
	
//	text_layer_set_text(&current_time_layer, "HOUR CHANGE\nHAPPENED");
	// TODO: celebrate with raise animation
	
	// on every change in day
	if ((t->units_changed & DAY_UNIT) == DAY_UNIT) {
		// change moon image
		set_container_image(&moon_image, MOON_IMAGE_RESOURCE_IDS[moon_phase(t->tick_time->tm_year+1900, t->tick_time->tm_yday)], GPoint(0, 0), &moon_layer);
	}
}


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
	/*
	 handles all triggers by the second
	 - handle_hour_tick
	 - timer-based arch animation. timer_handle
	 - braces animation. animation_schedule
	 */
	
	(void)ctx;
	
	unsigned short display_second = t->tick_time->tm_sec;
	
	
	// on every hour change
	if ((t->units_changed & HOUR_UNIT) == HOUR_UNIT) {
		handle_hour_tick(ctx,t);
	}
	
//	update_debug(t->tick_time);
// test moon_image
//	set_container_image(&moon_image, MOON_IMAGE_RESOURCE_IDS[display_second % 12], GPoint(0, 0), &moon_layer);
// test time_as_words
//	time_as_words(t->tick_time->tm_hour, display_second, current_time);
//	text_layer_set_text(&current_time_layer, current_time);
	
	
	// trigger raise animation (once ever) after intro is complete
	if (introComplete && bracesOpen) {	// Redundant? Not really! Avoids the scenario where the intro occurs right before the minute mark
		introComplete = false;	// never again!
		animateNow = true;
		raisedNotPlaying = false;
		timer_handle = app_timer_send_event(ctx, SPERF, RAISED_TIMER);
	}
	
	
	// Play smoke animation every SMOKE_LOOP seconds except on the minute mark when the braces animate.
	// If you do that, you're gonna have a bad time.
	if ((display_second % SMOKE_LOOP) == 0 && display_second != 0 && bracesOpen && raisedNotPlaying) {
		animateNow = true;
		timer_handle = app_timer_send_event(ctx, SPERF, SMOKE_TIMER);
	}
	
	
	// arch_turn every half SMOKE_LOOP for 2 seconds
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 && bracesOpen && raisedNotPlaying) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[10], GPoint(0, 0), &arch_layer);
	}
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 + 2 && bracesOpen && raisedNotPlaying) {
		set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(0, 0), &arch_layer);
	}
	
	
	
	// Animate braces every minute
	// Don't schedule the animations at the same time as the timer-based!
	
	// close braces
	if (display_second % 60 == 59 && bracesOpen && raisedNotPlaying) {
		bracesOpen = false;
		
		text_layer_set_text(&current_time_layer, "");
		text_layer_set_background_color(&brace_hider_layer, GColorBlack);
		
		property_animation_init_layer_frame(&braces_animation[1], &lbrace_layer, NULL, &GRect((144-16)/2,40,16,61));
		property_animation_init_layer_frame(&braces_animation[2], &rbrace_layer, NULL, &GRect((144-16)/2,40,16,61));
		animation_set_duration(&braces_animation[1].animation, 300);
		animation_set_duration(&braces_animation[2].animation, 300);
		animation_set_curve(&braces_animation[1].animation,AnimationCurveLinear);
		animation_set_curve(&braces_animation[2].animation,AnimationCurveLinear);		
		animation_schedule(&braces_animation[1].animation);	
		animation_schedule(&braces_animation[2].animation);
	}
	
	// open braces
	if (display_second % 60 == 0 && !bracesOpen && raisedNotPlaying) {
		
		property_animation_init_layer_frame(&braces_animation[1], &lbrace_layer, NULL, &GRect(0,40,16,61));
		property_animation_init_layer_frame(&braces_animation[2], &rbrace_layer, NULL, &GRect(144-16,40,16,61));
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
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
	(void)ctx;
	
	window_init(&window, "Sworcery watch");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	// init the archetype layer
//	layer_init(&arch_layer, window.layer.frame);
	layer_init(&arch_layer, GRect(86, 87, 40, 81));
	layer_add_child(&window.layer, &arch_layer);
	
	resource_init_current_app(&APP_RESOURCES);
//	moon_image and arch_image init's are handled in set_container_image
	bmp_init_container(RESOURCE_ID_LBRACE, &lbrace);
	bmp_init_container(RESOURCE_ID_RBRACE, &rbrace);
	
	// init moon layer and graphic
	layer_init(&moon_layer, GRect(3, 3, 22, 22));
	layer_add_child(&window.layer, &moon_layer);
	
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
	
/*	// init the debug text layer
	text_layer_init(&text_debug_layer, GRect(0, 0, 144, 30));
	text_layer_set_text_alignment(&text_debug_layer, GTextAlignmentRight);
	text_layer_set_text_color(&text_debug_layer, GColorWhite);
	text_layer_set_background_color(&text_debug_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug_layer.layer);
*/	
/*	// init the 2nd debug text layer
	text_layer_init(&text_debug2_layer, GRect(0, 20, 144, 30));
	text_layer_set_text_alignment(&text_debug2_layer, GTextAlignmentRight);
	text_layer_set_text_color(&text_debug2_layer, GColorWhite);
	text_layer_set_background_color(&text_debug2_layer, GColorClear);
	layer_add_child(&window.layer, &text_debug2_layer.layer);
*/	
	// init the current time layer
	text_layer_init(&current_time_layer, GRect(0, 50, 144, 60));
	text_layer_set_font(&current_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(&current_time_layer, GTextAlignmentCenter);
	text_layer_set_text_color(&current_time_layer, GColorWhite);
	text_layer_set_background_color(&current_time_layer, GColorClear);
//	text_layer_set_text(&current_time_layer, "");						// hide text
	layer_add_child(&window.layer, &current_time_layer.layer);
	
	bracesOpen = false;
	introComplete = false;
	
	
	// perform intro raise animation
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
		.stopped = (AnimationStoppedHandler) intro_animation_stopped
	}, &ctx);
	
	animation_schedule(&braces_animation[1].animation);
	animation_schedule(&braces_animation[2].animation);
	
	// Set default images (the ones that change).
	// set_container_image can only be used after initializing APP_RESOURCES
	PblTm t;
	get_time(&t);
	set_container_image(&moon_image, MOON_IMAGE_RESOURCE_IDS[moon_phase(t.tm_year+1900, t.tm_yday)], GPoint(0, 0), &moon_layer);
	set_container_image(&arch_image, ARCH_IMAGE_RESOURCE_IDS[0], GPoint(0, 0), &arch_layer); // place default arch image
	
}


void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	
	bmp_deinit_container(&arch_image);
	bmp_deinit_container(&moon_image);
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