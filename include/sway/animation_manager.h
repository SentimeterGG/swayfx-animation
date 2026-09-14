#ifndef _SWAY_ANIMATION_MANAGER_H
#define _SWAY_ANIMATION_MANAGER_H

#include <stdbool.h>
#include <wayland-util.h>

struct sway_server;

enum animation_kind {
	ANIMATION_KIND_RESIZE = 0,
	ANIMATION_KIND_OPEN = 1,
	ANIMATION_KIND_CLOSE = 2,
	ANIMATION_KIND_FULLSCREEN = 3,
};

struct animation {
	struct wl_list link;
	float progress;
	float multiplier;
	float duration_scale;
	float delay;
	enum animation_kind kind;
	bool initialized;
};

void animation_manager_init(struct sway_server *server);

struct animation init_animation();

void refresh_animation_manager_timing();

void add_animation(struct animation *animation);

void start_animations(void (update_callback)(void));

float animation_kind_duration_ms(enum animation_kind kind);

float animation_scale_for_kind(enum animation_kind kind);

float get_animated_value(float from, float to, struct animation animation);

#endif
