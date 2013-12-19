/*
 
 Sworcery
 
 https://github.com/keelanc/sworcery
 
 Inspired by the excellent game Sword & Sworcery
 created by Superbrothers and Capybara Games.
 
 
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

#include "pebble.h"
#include <math.h>
#include "time_as_words.h"
//#include "mini-printf.h"


static Window *window;

static GBitmap *moon_image;
static BitmapLayer *moon_layer;

static GBitmap *arch_image;
static BitmapLayer *arch_layer;

static GBitmap *lbrace;
static BitmapLayer *lbrace_layer;

static GBitmap *rbrace;
static BitmapLayer *rbrace_layer;

static TextLayer *brace_hider_layer;
static TextLayer *current_time_layer;

static PropertyAnimation *lbrace_animation = NULL;
static PropertyAnimation *rbrace_animation = NULL;

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


static AppTimer *timer_handle;


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


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id) {
    GBitmap *old_image = *bmp_image;
    
    *bmp_image = gbitmap_create_with_resource(resource_id);
    
    Layer *layer = bitmap_layer_get_layer(bmp_layer);
    GRect frame = layer_get_frame(layer);
    
    bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
    layer_set_frame(layer, frame);
    
    if (old_image != NULL) {
        gbitmap_destroy(old_image);
    }
}


static void timer_smoke(void *context) {
    // timer callback for 'smoke' animation sequence
    
    set_container_image(&arch_image, arch_layer, ARCH_IMAGE_RESOURCE_IDS[smoke_ani[animation_frame]]);
    animation_frame++;
    if (animation_frame >= smoke_ani_length) {
        animation_frame = 0;
        animateNow = false;
    }
	if (animateNow) {
		timer_handle = app_timer_register(SPERF, timer_smoke, NULL);
	}
}

static void timer_raised(void *context) {
    // timer callback for 'raised' animation sequence
    
    set_container_image(&arch_image, arch_layer, ARCH_IMAGE_RESOURCE_IDS[raised_ani[animation_frame]]);
    animation_frame++;
    if (animation_frame >= raised_ani_length) {
        animation_frame = 0;
        animateNow = false;
        raisedNotPlaying = true;
    }
	if (animateNow) {
		timer_handle = app_timer_register(SPERF, timer_raised, NULL);
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

static void animation_stopped(Animation *animation, bool finished, void *data) {
	/*
	 callback for brace-opening animation
	 */
	
	bracesOpen = true;
	
	time_t now = time(NULL);
    struct tm * tick_time = localtime(&now);
	time_as_words(tick_time->tm_hour, tick_time->tm_min, current_time);
	text_layer_set_text(current_time_layer, current_time);
	text_layer_set_background_color(brace_hider_layer, GColorClear);
}

static void intro_animation_stopped(Animation *animation, bool finished, void *data) {
	/*
	 callback for the intro-brace-opening animation
	 */
	
	bracesOpen = true;
	
//	text_layer_set_text(&current_time_layer, "THE INTRO\nIS COMPLETE");
	time_t now = time(NULL);
    struct tm * tick_time = localtime(&now);
	time_as_words(tick_time->tm_hour, tick_time->tm_min, current_time);
	text_layer_set_text(current_time_layer, current_time);
	text_layer_set_background_color(brace_hider_layer, GColorClear);
	
	introComplete = true;
//	text_layer_set_text(&text_debug2_layer, "intro: true");
}

static void destroy_property_animation(PropertyAnimation **prop_animation) {
    // Borrowed from the 'feature_property_animation' example. Didn't need it in sdk1. Still unsure if I need it.
    if (*prop_animation == NULL) {
        return;
    }
    
    if (animation_is_scheduled((Animation*) *prop_animation)) {
        animation_unschedule((Animation*) *prop_animation);
    }
    
    property_animation_destroy(*prop_animation);
    *prop_animation = NULL;
}


static void handle_hour_tick(struct tm *t, TimeUnits units_changed) {
	
//	text_layer_set_text(&current_time_layer, "HOUR CHANGE\nHAPPENED");
	// TODO: celebrate with raise animation
	
	// on every change in day
	if ((units_changed & DAY_UNIT) == DAY_UNIT) {
		// change moon image
		set_container_image(&moon_image, moon_layer, MOON_IMAGE_RESOURCE_IDS[moon_phase(t->tm_year+1900, t->tm_yday)]);
	}
}


static void handle_second_tick(struct tm *t, TimeUnits units_changed) {
	/*
	 handles all triggers by the second
	 - handle_hour_tick
	 - timer-based arch animation. timer_handle
	 - braces animation. animation_schedule
	 */
	
	unsigned short display_second = t->tm_sec;
	
	
	// on every hour change
	if ((units_changed & HOUR_UNIT) == HOUR_UNIT) {
		handle_hour_tick(t,units_changed);
	}
	
//	update_debug(t->tick_time);
// test moon_image
//	set_container_image(&moon_image, moon_layer, MOON_IMAGE_RESOURCE_IDS[display_second % 12], GPoint(0, 0));
	
	
	// trigger raise animation (once ever) after intro is complete
	if (introComplete && bracesOpen) {	// Redundant? Not really! Avoids the scenario where the intro occurs right before the minute mark
		introComplete = false;	// never again!
		animateNow = true;
		raisedNotPlaying = false;
		timer_handle = app_timer_register(SPERF, timer_raised, NULL);
	}
	
	
	// Play smoke animation every SMOKE_LOOP seconds except on the minute mark when the braces animate.
	// If you do that, you're gonna have a bad time.
	if ((display_second % SMOKE_LOOP) == 0 && display_second != 0 && bracesOpen && raisedNotPlaying) {
		animateNow = true;
		timer_handle = app_timer_register(SPERF, timer_smoke, NULL);
	}
	
	
	// arch_turn every half SMOKE_LOOP for 2 seconds
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 && bracesOpen && raisedNotPlaying) {
		set_container_image(&arch_image, arch_layer, ARCH_IMAGE_RESOURCE_IDS[10]);
	}
	if ((display_second % SMOKE_LOOP) == SMOKE_LOOP/2 + 2 && bracesOpen && raisedNotPlaying) {
		set_container_image(&arch_image, arch_layer, ARCH_IMAGE_RESOURCE_IDS[0]);
	}
	
	
	
	// Animate braces every minute
	// Don't schedule the animations at the same time as the timer-based!
	
	// close braces
	if (display_second % 60 == 59 && bracesOpen && raisedNotPlaying) {
		bracesOpen = false;
		
		text_layer_set_text(current_time_layer, "");
		text_layer_set_background_color(brace_hider_layer, GColorBlack);
		
        destroy_property_animation(&lbrace_animation);
        destroy_property_animation(&rbrace_animation);
        
        Layer *llayer = bitmap_layer_get_layer(lbrace_layer);
        Layer *rlayer = bitmap_layer_get_layer(rbrace_layer);
        
		lbrace_animation = property_animation_create_layer_frame(llayer, NULL, &GRect((144-16)/2,40,16,61));
		rbrace_animation = property_animation_create_layer_frame(rlayer, NULL, &GRect((144-16)/2,40,16,61));
		animation_set_duration((Animation*) lbrace_animation, 300);
		animation_set_duration((Animation*) rbrace_animation, 300);
		animation_set_curve((Animation*) lbrace_animation,AnimationCurveLinear);
		animation_set_curve((Animation*) rbrace_animation,AnimationCurveLinear);		
		animation_schedule((Animation*) lbrace_animation);
		animation_schedule((Animation*) rbrace_animation);
	}
	
	// open braces
	if (display_second % 60 == 0 && !bracesOpen && raisedNotPlaying) {
		
        destroy_property_animation(&lbrace_animation);
        destroy_property_animation(&rbrace_animation);
        
        Layer *llayer = bitmap_layer_get_layer(lbrace_layer);
        Layer *rlayer = bitmap_layer_get_layer(rbrace_layer);
        
		lbrace_animation = property_animation_create_layer_frame(llayer, NULL, &GRect(0,40,16,61));
		rbrace_animation = property_animation_create_layer_frame(rlayer, NULL, &GRect(144-16,40,16,61));
		animation_set_duration((Animation*) lbrace_animation, 300);
		animation_set_duration((Animation*) rbrace_animation, 300);
		animation_set_curve((Animation*) lbrace_animation,AnimationCurveLinear);
		animation_set_curve((Animation*) rbrace_animation,AnimationCurveLinear);
		
		animation_set_handlers((Animation*) lbrace_animation, (AnimationHandlers) {
			.stopped = (AnimationStoppedHandler) animation_stopped
		}, NULL /* callback data */);
		
		animation_schedule((Animation*) lbrace_animation);
		animation_schedule((Animation*) rbrace_animation);
	}
	
}


static void handle_init(void) {
	// initializing app
	
	window = window_create();
	window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);
	Layer *root_layer = window_get_root_layer(window);
	
	// init the archetype layer
	arch_layer = bitmap_layer_create(GRect(86, 87, 40, 81));
	layer_add_child(root_layer, bitmap_layer_get_layer(arch_layer));
	
//	moon_image and arch_image init's are handled in set_container_image
    // init brace images
	lbrace = gbitmap_create_with_resource(RESOURCE_ID_LBRACE);
	rbrace = gbitmap_create_with_resource(RESOURCE_ID_RBRACE);
	
	// init moon layer and graphic
	moon_layer = bitmap_layer_create(GRect(3, 3, 22, 22));
	layer_add_child(root_layer, bitmap_layer_get_layer(moon_layer));
	
	//init the braces layers
	lbrace_layer = bitmap_layer_create(GRect((144-16)/2,40,16,61));		// hide braces
    bitmap_layer_set_bitmap(lbrace_layer, lbrace);
	layer_add_child(root_layer, bitmap_layer_get_layer(lbrace_layer));
	rbrace_layer = bitmap_layer_create(GRect((144-16)/2,40,16,61));		// hide braces
    bitmap_layer_set_bitmap(rbrace_layer, rbrace);
	layer_add_child(root_layer, bitmap_layer_get_layer(rbrace_layer));
	brace_hider_layer = text_layer_create(GRect((144-32)/2, 40, 32, 61));
	text_layer_set_background_color(brace_hider_layer, GColorBlack);	// hide braces
	layer_add_child(root_layer, text_layer_get_layer(brace_hider_layer));
	
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
	current_time_layer = text_layer_create(GRect(0, 50, 144, 60));
	text_layer_set_font(current_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(current_time_layer, GTextAlignmentCenter);
	text_layer_set_text_color(current_time_layer, GColorWhite);
	text_layer_set_background_color(current_time_layer, GColorClear);
	layer_add_child(root_layer, text_layer_get_layer(current_time_layer));
	
	bracesOpen = false;
	introComplete = false;
	
	
	// perform intro brace animation
	animation_unschedule_all();
    Layer *llayer = bitmap_layer_get_layer(lbrace_layer);
    Layer *rlayer = bitmap_layer_get_layer(rbrace_layer);
	lbrace_animation = property_animation_create_layer_frame(llayer, NULL, &GRect(0,40,16,61));
	rbrace_animation = property_animation_create_layer_frame(rlayer, NULL, &GRect(144-16,40,16,61));
	animation_set_delay((Animation*) lbrace_animation, 600);
	animation_set_delay((Animation*) rbrace_animation, 600);
	animation_set_duration((Animation*) lbrace_animation, 300);
	animation_set_duration((Animation*) rbrace_animation, 300);
	animation_set_curve((Animation*) lbrace_animation,AnimationCurveLinear);
	animation_set_curve((Animation*) rbrace_animation,AnimationCurveLinear);
	
	animation_set_handlers((Animation*) lbrace_animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler) intro_animation_stopped
	}, NULL /* callback data */);
	
	animation_schedule((Animation*) lbrace_animation);
	animation_schedule((Animation*) rbrace_animation);
	
	// Set default images (the ones that change).
	time_t now = time(NULL);
    struct tm * t = localtime(&now);
	set_container_image(&moon_image, moon_layer, MOON_IMAGE_RESOURCE_IDS[moon_phase(t->tm_year+1900, t->tm_yday)]);
	set_container_image(&arch_image, arch_layer, ARCH_IMAGE_RESOURCE_IDS[0]); // place default arch image
	
    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}


static void handle_deinit(void) {
    
    layer_remove_from_parent(bitmap_layer_get_layer(moon_layer));
    bitmap_layer_destroy(moon_layer);
    gbitmap_destroy(moon_image);
    
    layer_remove_from_parent(bitmap_layer_get_layer(arch_layer));
    bitmap_layer_destroy(arch_layer);
    gbitmap_destroy(arch_image);
    
    layer_remove_from_parent(bitmap_layer_get_layer(lbrace_layer));
    bitmap_layer_destroy(lbrace_layer);
    gbitmap_destroy(lbrace);
    
    layer_remove_from_parent(bitmap_layer_get_layer(rbrace_layer));
    bitmap_layer_destroy(rbrace_layer);
    gbitmap_destroy(rbrace);
    
    text_layer_destroy(brace_hider_layer);
    text_layer_destroy(current_time_layer);
    window_destroy(window);
}


int main(void) {
    handle_init();
	app_event_loop();
    handle_deinit();
}