#include <assert.h>
#include "sway/animation_manager.h"
#include "sway/config.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/arrange.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/root.h"

struct animation_manager {
	float tick_time;
	float progress_delta;
	struct wl_event_source *tick;
	struct wl_list animations;
	void (*update)(void);
} animation_manager;

struct animation init_animation() {
	return (struct animation){
		.progress = 0.0f,
		.multiplier = 0.0f,
		.duration_scale = 1.0f,
		.delay = 0.0f,
		.kind = ANIMATION_KIND_RESIZE,
	};
}

float ease_out_cubic(float t) {
	float p = t - 1;
	return pow(p, 3) + 1;
}

static double window_bezier_x(double t, double c1x, double c2x) {
	double one_minus_t = 1.0 - t;
	return 3 * one_minus_t * one_minus_t * t * c1x
		+ 3 * one_minus_t * t * t * c2x + t * t * t;
}

static float window_bezier_ease(float progress, double c1x, double c1y,
		double c2x, double c2y) {
	if (progress <= 0.0f) {
		return 0.0f;
	}
	if (progress >= 1.0f) {
		return 1.0f;
	}
	// Binary search for the curve time whose x-coordinate hits our progress.
	double t_low = 0.0;
	double t_high = 1.0;
	for (int iteration = 0; iteration < 32; ++iteration) {
		double t_mid = (t_low + t_high) * 0.5;
		if (window_bezier_x(t_mid, c1x, c2x) < progress) {
			t_low = t_mid;
		} else {
			t_high = t_mid;
		}
	}
	double t = (t_low + t_high) * 0.5;
	double one_minus_t = 1.0 - t;
	return (float)(3 * one_minus_t * one_minus_t * t * c1y
		+ 3 * one_minus_t * t * t * c2y + t * t * t);
}

static float ease_animation(float progress, enum animation_kind kind) {
	if (!config) {
		return ease_out_cubic(progress);
	}
	switch (kind) {
	case ANIMATION_KIND_OPEN:
		return window_bezier_ease(progress,
			config->window_open_curve_c1x, config->window_open_curve_c1y,
			config->window_open_curve_c2x, config->window_open_curve_c2y);
	case ANIMATION_KIND_CLOSE:
		return window_bezier_ease(progress,
			config->window_close_curve_c1x, config->window_close_curve_c1y,
			config->window_close_curve_c2x, config->window_close_curve_c2y);
	case ANIMATION_KIND_RESIZE:
	default:
		return window_bezier_ease(progress,
			config->window_resize_curve_c1x, config->window_resize_curve_c1y,
			config->window_resize_curve_c2x, config->window_resize_curve_c2y);
	}
}

int animation_timer() {
	struct animation *animation, *tmp;
	wl_list_for_each_reverse_safe(animation, tmp, &animation_manager.animations, link) {
		float duration_scale =
			animation->duration_scale > 0.0f ? animation->duration_scale : 1.0f;
		if (animation->delay > 0.0f) {
			animation->delay -= animation_manager.progress_delta / duration_scale;
			if (animation->delay < 0.0f) {
				animation->delay = 0.0f;
			}
			// hold at start pose while delayed; progress stays 0
			animation->progress = 0.0f;
			animation->multiplier = 0.0f;
			continue;
		}
		animation->progress = MIN(animation->progress +
			animation_manager.progress_delta / duration_scale, 1.0f);
		animation->multiplier = ease_animation(animation->progress,
			animation->kind);
		if (animation->progress == 1.0f) {
			wl_list_remove(&animation->link);
			animation->initialized = false;
		}
	}

	animation_manager.update();

	if (!wl_list_empty(&animation_manager.animations)) {
		wl_event_source_timer_update(animation_manager.tick,
				animation_manager.tick_time);
	}
	return 0;
}

void add_animation(struct animation *animation) {
	// remove previous instances of this animation
	if (animation->initialized) {
		wl_list_remove(&animation->link);
	}

	animation->progress = 0.0f;
	animation->multiplier = 0.0f;
	animation->initialized = true;
	wl_list_insert(&animation_manager.animations, &animation->link);
}

void start_animations(void (update_callback)(void)) {
	if (!config->animation_duration_ms) {
		return;
	}
	assert(animation_manager.tick);
	animation_manager.update = update_callback;
	wl_event_source_timer_update(animation_manager.tick, 1);
}

float get_fastest_output_refresh_ms() {
	float fastest_output_refresh_ms = 16.6667; // fallback to 60 Hz
	for (int i = 0; i < root->outputs->length; ++i) {
		struct sway_output *output = root->outputs->items[i];
		if (output->refresh_nsec > 0) {
			float output_refresh_ms = output->refresh_nsec / 1000000.0;
			fastest_output_refresh_ms = MIN(fastest_output_refresh_ms, output_refresh_ms);
		}
	}
	return fastest_output_refresh_ms;
}

void refresh_animation_manager_timing() {
	if (!config || !root) {
		return;
	}
	animation_manager.tick_time = get_fastest_output_refresh_ms();
	if (config->animation_duration_ms > 0.0f) {
		animation_manager.progress_delta =
			animation_manager.tick_time / config->animation_duration_ms;
	} else {
		animation_manager.progress_delta = 1.0f;
	}
}

float animation_kind_duration_ms(enum animation_kind kind) {
	if (!config || config->animation_duration_ms <= 0.0f) {
		return 0.0f;
	}
	float base = config->animation_duration_ms;
	switch (kind) {
	case ANIMATION_KIND_OPEN:
		return config->window_open_duration_ms >= 0.0f ?
			config->window_open_duration_ms : base;
	case ANIMATION_KIND_CLOSE:
		return config->window_close_duration_ms >= 0.0f ?
			config->window_close_duration_ms : base * 0.8f;
	case ANIMATION_KIND_RESIZE:
	default:
		return config->window_resize_duration_ms >= 0.0f ?
			config->window_resize_duration_ms : base;
	}
}

float animation_scale_for_kind(enum animation_kind kind) {
	if (!config || config->animation_duration_ms <= 0.0f) {
		return 1.0f;
	}
	float dur = animation_kind_duration_ms(kind);
	if (dur <= 0.0f) {
		return 1.0f;
	}
	return dur / config->animation_duration_ms;
}

void animation_manager_init(struct sway_server *server) {
	animation_manager.tick = wl_event_loop_add_timer(server->wl_event_loop,
			animation_timer, NULL);
	wl_list_init(&animation_manager.animations);
	refresh_animation_manager_timing();
}

float lerp(float a, float b, float t) {
	return a * (1.0 - t) + b * t;
}


float get_animated_value(float from, float to, struct animation animation) {
	if (!config->animation_duration_ms) {
		return to;
	}
	return lerp(from, to, animation.multiplier);
}
