<p align="center">
    <img src="assets/swayfx_logo.svg" width="256" alt="swayfx logo">
</p>

[SwayFX-Enhanced](https://github.com/CreitinGameplays/swayfx-enhanced)

Sway is an incredible window manager, and certainly one of the most well established Wayland window managers. However, it is restricted to only include the functionality that existed in i3. This fork ditches the simple `wlr_renderer`, and replaces it with our `fx_renderer` (via [scenefx](https://github.com/wlrfx/scenefx)), capable of rendering with fancy GLES2 effects.

### Note: this is a fork of the original [swayfx](https://github.com/WillPower3309/swayfx) repository.

SwayFX expands Sway's feature set to include eye-candy that many users have been asking for:

+ **Blur**: Sophisticated background blur for windows and layers.
+ **Shadows**: Real-time window drop shadows.
+ **Rounded Corners**: Anti-aliased rounded corners for windows, borders, and titlebars.
+ **Animations**: Smooth window movement and resizing animations, plus fast default pop/expand animations when windows open and close.
+ **Scrollable Tiling**: Fixed-width horizontal columns with animated workspace panning, wrap-around navigation, and direct column resizing.
+ **New: Liquid Glass (Experimental)**: A unique refractive glass effect.
+ **Dimming**: Dim unfocused windows to help you focus.
+ **Layer Shell Effects**: Apply blur, shadows, and rounded corners to panels and notifications.
+ **Scratchpad treated as minimize**: Allows docks and taskbars to correctly interpret minimize/unminimize requests.
+ **Nixified**: First-class support for Nix and NixOS.

<p align="center">
    <img src="assets/preview.png" width="500"></img>
    <sub>ignore the insane RAM usage in this pic</sub>
</p>

### Animations
Control the duration and easing of window open, close, and move/resize animations.

- `animation_duration_ms <value>`: Master timing in milliseconds (0-5000, default: 90). Set to 0 to disable all window animations. When no per-type duration is set, open and resize use this value and close uses 0.8x this value.
- `window_open_duration_ms <value>`: Open animation duration in milliseconds (0-5000, 0 = instant). Unset by default: follows `animation_duration_ms`.
- `window_close_duration_ms <value>`: Close animation duration in milliseconds (0-5000, 0 = instant). Unset by default: follows 0.8x `animation_duration_ms`.
- `window_resize_duration_ms <value>`: Move/resize animation duration in milliseconds (0-5000, 0 = instant). Unset by default: follows `animation_duration_ms`.
- `window_fullscreen_duration_ms <value>`: Fullscreen enter/exit animation duration in milliseconds (0-5000, 0 = instant snap). Unset by default: follows `animation_duration_ms`.
- `window_open_animation_delay <value>`: Holds a new window hidden for `<value>` milliseconds before its open animation runs, so sibling resize finishes first (0-5000, default: 90). Set to 0 to open immediately. Ignored when the target workspace is empty.
- `window_open_curve <preset|cubic-bezier(a, b, c, d)>`: Easing for the open animation.
- `window_close_curve <preset|cubic-bezier(a, b, c, d)>`: Easing for the close animation.
- `window_resize_curve <preset|cubic-bezier(a, b, c, d)>`: Easing for move/resize animations.
- `window_fullscreen_curve <preset|cubic-bezier(a, b, c, d)>`: Easing for fullscreen enter/exit animations.
  - Curve presets: `linear`, `ease`, `ease-in`, `ease-out`, `ease-in-out`, `ease-out-cubic` (default), `menu_decel`.
  - Example: `window_open_curve cubic-bezier(0.215, 0.61, 0.355, 1)`.

### Workspace Switch Animations
Directional slide animation when switching workspaces on the same output.

- `workspace_switch_anim <yes|no>`: Enable the slide animation (default: `no`).
- `workspace_anim_duration_ms <value>`: Fixed slide duration in milliseconds (0-5000, default: 200). Used when `workspace_anim_step_ms` is `0`.
- `workspace_anim_step_ms <value>`: Per-workspace-step time in milliseconds (0-5000, default: 200). A jump spanning N workspaces takes `step * N`. Set to `0` for a fixed `workspace_anim_duration_ms` regardless of distance.
- `workspace_switch_curve <preset|cubic-bezier(a, b, c, d)>`: Easing for the slide (default: `menu_decel`).
  - Curve presets: `linear`, `ease`, `ease-in`, `ease-out`, `ease-in-out`, `menu_decel` (default).
  - Example: `workspace_switch_curve cubic-bezier(0.1, 1, 0, 1)`.

Behavior:
- Slides right when moving forward (e.g. `1` to `2`) and left when moving back, based on workspace numbers and output order. `workspace next` always slides right and `workspace prev` always slides left, even when wrapping around.
- Long jumps slide through every non-empty workspace in between as a filmstrip, so switching `1` to `3` visibly passes workspace `2`. Empty middle workspaces are skipped; an empty destination still slides in as blank space.
- No animation across different outputs or when a fullscreen workspace is involved.

Example:
```sway
workspace_switch_anim yes
workspace_anim_step_ms 200
workspace_switch_curve menu_decel
```

New windows use pop/expand animations by default when opening and closing.

Default config convenience:
- `Mod+m maximize`: Maximize the focused window to the current workspace without entering fullscreen.

### Scrollable Tiling
Enable a niri-style horizontal strip layout for top-level tiled windows.
- `workspace_layout scrollable`: Make new workspaces default to the scrollable layout.
- `layout scrollable`: Switch the current workspace to the scrollable layout. In config, this also acts as a default-layout alias.
- `layout splith|splitv|stacking|tabbed|default`: Switch back out of scrollable mode.
- `scroll left|right [<px> px]`: Pan the scrollable workspace camera. Defaults to `100px` when no amount is given.
- `scroll center`: Center the focused column in the visible area.
- `scroll home|end`: Jump the camera to the beginning or end of the workspace strip.
- `scroll follow`: Return to focus-following camera behavior.
- `scrollable_tiling_touchpad_scroll enable|disable`: Enable direct two-finger horizontal touchpad panning for scrollable workspaces. Disabled by default.
- `scrollable_tiling_touchpad_scroll_factor <float>`: Scale touchpad pan distance. Defaults to `1.0`.
- `scrollable_tiling_touchpad_pinch_resize enable|disable`: Enable touchpad pinch resizing for the focused scrollable column. Disabled by default.
- `scrollable_tiling_touchpad_pinch_resize_factor <float>`: Scale pinch resize sensitivity. Defaults to `1.0`.

Current interaction behavior:
- New tiled windows open as new top-level columns.
- Two-finger horizontal touchpad gestures pan the workspace camera when `scrollable_tiling_touchpad_scroll` is enabled.
- Pinch outward grows the focused scrollable column and pinch inward shrinks it when `scrollable_tiling_touchpad_pinch_resize` is enabled. Maximized columns pass pinch gestures through to apps for page zoom.
- `Shift` + mouse wheel moves focus left or right across columns using the focused window, not the pointer location.
- `$mod` + `Shift` + `Left`/`Right` moves the focused column left or right in the scrollable strip.
- Focus navigation wraps around at the ends of the strip.
- `Mod` + right click resizes scrollable columns directly.
- Tiling drag gestures reorder top-level columns instead of splitting them.
- Dragging a column near the left or right edge auto-scrolls the workspace strip.
- The workspace camera animates smoothly and uses edge handoff instead of hard centering on every focus change.

### Blur
Global blur settings and per-window toggles.
- `blur <enable|disable>`: Global toggle.
- `blur_xray <enable|disable>`: Blur floating windows based on the desktop background instead of windows below.
- `blur_passes <integer>`: Number of blur passes (0-10).
- `blur_radius <integer>`: Blur radius (0-10).
- `blur_noise <float>`: Amount of noise to add to the blur (0-1).
- `blur_brightness <float>`: Adjust blur brightness (0-2).
- `blur_contrast <float>`: Adjust blur contrast (0-2).
- `blur_saturation <float>`: Adjust blur saturation (0-2).
- `for_window [CRITERIA] blur <enable|disable>`: Per-window toggle.

### Corner Radius
- `corner_radius <pixels>`: Set the radius for rounded corners.
- `smart_corner_radius <enable|disable>`: Automatically disable rounded corners when a window is fullscreen or the only one on a workspace (if gaps are 0).

### Shadows
- `shadows <enable|disable>`: Global toggle.
- `shadows_on_csd <enable|disable>`: Enable shadows on windows with Client-Side Decorations.
- `shadow_blur_radius <pixels>`: Blur radius for shadows (0-99).
- `shadow_color <hex>`: Shadow color (e.g., `#0000007F`).
- `shadow_inactive_color <hex>`: Shadow color for unfocused windows.
- `shadow_offset <x> <y>`: Offset for the shadow.

### Dimming
- `default_dim_inactive <0.0 - 1.0>`: Set the default dimming for inactive windows.
- `for_window [CRITERIA] dim_inactive <0.0 - 1.0>`: Per-window dimming.
- `dim_inactive_colors.unfocused <hex>`: Color used for dimming unfocused windows.
- `dim_inactive_colors.urgent <hex>`: Color used for dimming urgent windows.

### Layer Shell Effects
Apply effects to specific layer shell namespaces (e.g., "waybar", "notifications").
- `layer_effects <namespace> <effects>`
- Available Effects:
    - `blur <enable|disable>`
    - `blur_xray <enable|disable>`
    - `blur_ignore_transparent <enable|disable>`
    - `shadows <enable|disable>`
    - `corner_radius <pixels>`
- Example:
    ```sway
    layer_effects "waybar" {
        blur enable
        blur_ignore_transparent enable
        shadows enable
        corner_radius 10
    }
    ```

### Liquid Glass (Experimental)
A refractive glass effect that distorts the background.
- `liquid_glass <enable|disable>`
- `liquid_glass_bezel_width <float>`
- `liquid_glass_brightness_boost <float>`
- `liquid_glass_chromatic_aberration <float>`
- `liquid_glass_noise_intensity <float>`
- `liquid_glass_refraction_index <float>`
- `liquid_glass_saturation_boost <float>`
- `liquid_glass_specular <on/off>`
- `liquid_glass_specular_opacity <float>`
- `liquid_glass_specular_angle <float>`
- `liquid_glass_surface <convex_circle|convex_squircle|concave|lip>`
- `liquid_glass_thickness <float>`

### Miscellaneous
- `titlebar_separator <enable|disable>`: Show or hide the separator between the titlebar and window content.
- `scratchpad_minimize <enable|disable>`: Treat hiding a window to the scratchpad as minimizing it.

### Xwayland Compatibility
This fork requires wlroots Xwayland support at build time, starts Xwayland immediately by default, and installs the default config with:

```sway
xwayland force
```

Display managers launch the session through `sway-xwayland-session`, which keeps common GUI toolkits on native Wayland by default while leaving Xwayland available for X11-only apps:

```sh
XDG_SESSION_TYPE=wayland
GDK_BACKEND=wayland,x11
QT_QPA_PLATFORM='wayland;xcb'
SDL_VIDEODRIVER=wayland,x11
CLUTTER_BACKEND=wayland
MOZ_ENABLE_WAYLAND=1
ELECTRON_OZONE_PLATFORM_HINT=wayland
```

Ways to check a running system:

```bash
swayfx-check-xwayland
echo "$DISPLAY"
pgrep -a Xwayland
swaymsg -t get_tree | grep '"shell": "xwayland"'
```

If the final command has no output, launch an X11 test client first, such as `env GDK_BACKEND=x11 xeyes`, then run it again.

## Roadmap

+ Improve Liquid Glass stability and performance.

## Compiling From Source

### Nix

If you have Nix installed, you can build and run SwayFX easily:

```bash
nix build
./result/bin/sway
```

For development:
```bash
nix develop
```

### Debian

Check [INSTALL-deb.md](/INSTALL-deb.md)

### Container (Docker)

```bash
docker compose up --build
```

### Manual Steps

Install dependencies:
`meson`, `wlroots`, `wayland`, `wayland-protocols`, `pcre2`, `json-c`, `pango`, `cairo`, `scenefx`, `gdk-pixbuf2` (optional), `swaybg` (optional), `scdoc` (optional).

```bash
meson build/
ninja -C build/
sudo ninja -C build/ install
```

### ASan Debugging

When chasing compositor crashes or memory corruption, build a separate AddressSanitizer binary so the normal `build/` tree stays untouched:

```bash
meson setup build-asan -Db_sanitize=address -Dbuildtype=debug -Doptimization=0
ninja -C build-asan sway/sway
```

Run the instrumented compositor and save its debug output:

```bash
ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:detect_leaks=0 \
./build-asan/sway/sway -d 2> /tmp/swayfx-asan.log
```

After reproducing the crash, inspect the log for the allocator report and stack trace:

```bash
rg -n "AddressSanitizer|ERROR:|SUMMARY:" /tmp/swayfx-asan.log
tail -n 200 /tmp/swayfx-asan.log
```

## Acknowledgements

SwayFX is a community project built on the shoulders of giants. We thank:
- The **Sway** maintainers and contributors for the solid foundation.
- **pkdesuwu** and **honchokomodo** for the SwayFX mascot.
- **spooky_skeleton** for the SwayFX logo, and **Basil** for refinements.
- Our amazing community for testing and supporting the project.

### todo
- [x] steal Niriwm features
