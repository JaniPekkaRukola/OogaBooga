#ifndef SETTINGS_H
#define SETTINGS_H

// debug / dev
bool IS_DEBUG = false;
bool render_fps = true;
bool print_fps = false;

// settings
const float screen_width = 240.0;
const float screen_height = 135.0;
float view_zoom = 0.1875;   // view zoom ratio x (pixelart layer width / window width = 240 / 1280 = 0.1875)

// keybinds
char KEY_player_use = 'F';
char KEY_toggle_inventory = KEY_TAB;

#endif