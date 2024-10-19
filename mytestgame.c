// includes all headers
#include "headers\masterheader.h"

// ----- ::Settings || ::Tweaks || ::Global --------------------------------------------------------|

// #define DEV_TESTING

// bool print_fps = true;
bool print_fps = false;
bool render_fps = true;
bool ENABLE_FRUSTRUM_CULLING = true;
bool runtime_debug = false;


bool enable_vignette = true;
bool enable_tooltip = true;
bool enable_chest_tooltip = false;
bool render_hotbar = true;
bool render_player_sprite = true;
bool render_other_entities = true;
bool draw_grid = false;
bool render_ground_texture = true;
bool enable_entity_selection_by_player_position = true;

float render_distance = 230;
float render_distance_32 = 10; // for 32x32 tiles

// keybinds
char KEY_player_use = 'F';
char KEY_toggle_inventory = KEY_TAB;


// COLORS
const Vector4 item_shadow_color = {0, 0, 0, 0.2};
const Vector4 entity_shadow_color = {0, 0, 0, 0.15};

// rendering layers
const s32 layer_ui = 20;
const s32 layer_workstation_ui = 15;
const s32 layer_world = 10;

// Global app stuff
float64 delta_t;
float64 current_time;
int fps = 0;

// ----- engine changes (by: randy) ----------------------------------------------------------------|
// maybe should move these into "functions.h"

// inline float v2_length(Vector2 a) {
//     return sqrt(a.x * a.x + a.y * a.y);
// }


// :Timer stuff
float float_alpha(float x, float min, float max) {
	float res = (x - min) / (max - min);
	res = clamp(res, 0.0, 1.0);
	return res;
}

inline float64 now() {
	// TODO: replace "os_get_elapsed_seconds" with "world->time_elapsed"
	return os_get_elapsed_seconds();
}

float alpha_from_end_time(float64 end_time, float length) {
	return float_alpha(now(), end_time - length, end_time);
}

bool has_reached_end_time(float64 end_time) {
	return now() > end_time;
}


int compare_strings(string str, const char* cstr) {
    u64 cstr_len = strlen(cstr);
    if (str.count != cstr_len) {
        return 0;
    }
    for (u64 i = 0; i < str.count; i++) {
        if (str.data[i] != (u8)cstr[i]) {
            return 0;
        }
    }
    return 1;
}



Vector2 range2f_get_center(Range2f range) {
	return (Vector2) {(range.max.x - range.min.x) * 0.5 + range.min.x, (range.max.y - range.min.y) * 0.5 + range.min.y};
}

// ----- the scuff zone (by: randy) --------------------------------------------------------------------|

Draw_Quad ndc_quad_to_screen_quad(Draw_Quad ndc_quad) {

	// NOTE: we're assuming these are the screen space matricies (by: randy)
	Matrix4 proj = draw_frame.projection;
	// Matrix4 view = draw_frame.view; 			// deprecated
	Matrix4 view = draw_frame.camera_xform;
	
	Matrix4 ndc_to_screen_space = m4_identity;
	ndc_to_screen_space = m4_mul(ndc_to_screen_space, m4_inverse(proj));
	ndc_to_screen_space = m4_mul(ndc_to_screen_space, view);

	ndc_quad.bottom_left = m4_transform		(ndc_to_screen_space, v4(v2_expand(ndc_quad.bottom_left), 0, 1)).xy;
	ndc_quad.bottom_right = m4_transform	(ndc_to_screen_space, v4(v2_expand(ndc_quad.bottom_right), 0, 1)).xy;
	ndc_quad.top_left = m4_transform		(ndc_to_screen_space, v4(v2_expand(ndc_quad.top_left), 0, 1)).xy;
	ndc_quad.top_right = m4_transform		(ndc_to_screen_space, v4(v2_expand(ndc_quad.top_right), 0, 1)).xy;

	return ndc_quad;
}


void clear_empty_slots_in_entities(Entity* entities, int count){
	// Verification loop to ensure no empty slots
    int shiftIndex = 0;
    for (int i = 0; i < count; i++) {
		Entity* en = &entities[i];
        if (en->arch == ARCH_nil) {
            shiftIndex++;
        } else if (shiftIndex > 0) {
            // Shift the entity left to fill the gap
            entities[i - shiftIndex] = entities[i];
        }
    }
}

void create_parallax_trees(int amount, int range) {
	// Creates trees
	// Wont allow multiple trees to spawn in the same tile

	Vector2 parallax_positions[amount];

	for (int i = 0; i < amount; i++){
		float x = get_random_float32_in_range(-range, range);
		float y = get_random_float32_in_range(-range, range);
		parallax_positions[i] = v2(x,y);
	}

	for (int i = 0; i < amount; i++){
		Entity* en = entity_create();
		setup_parallax(en);
		en->pos = v2(parallax_positions[i].x, parallax_positions[i].y);
		// en->pos = round_v2_to_tile(en->pos);
		// printf("Created a tree at '%.0f     %.0f'\n", tree_positions[i].x, tree_positions[i].y);
	}


	// free the list of positions
	memset(parallax_positions, 0, sizeof(parallax_positions));
}

// ----- ::FUNC Dump (name by randy) -------------------------------------------------------------------|

// :Render UI
void render_ui(){
	// sexy ui render func

	set_screen_space();
	push_z_layer(layer_ui);

	// Open Inventory
	if (is_key_just_pressed(KEY_toggle_inventory) && world->ux_state == UX_nil)
	{
		consume_key_just_pressed(KEY_toggle_inventory);
		// world->ux_state = (world->ux_state == UX_nil ? UX_inventory : UX_nil);
		// world->player->inventory_ui_open = true;
		world->player->inventory_ui_open = (world->player->inventory_ui_open == false ? true : false);
		world->ux_state = UX_nil;
	}

	// ::Render inventory
	// if (world->ux_state == UX_inventory)
	if (world->player->inventory_ui_open)
	{
		// world->inventory_alpha_target = (world->ux_state == UX_inventory ? 1.0 : 0.0);
		world->inventory_alpha_target = (world->player->inventory_ui_open == true ? 1.0 : 0.0);
		animate_f32_to_target(&world->inventory_alpha, world->inventory_alpha_target, delta_t, 15.0);
		bool is_inventory_enabled = world->inventory_alpha_target == 1.0;

		if (world->inventory_alpha_target != 0.0){ // temp line for instant opening of the inventory, since global draw frame alpha is not a thing (yet)

			// printf("RENDERING INVENTORY\n");

			// Inventory variables
			const int slot_size = 8;
			const float padding = 2.0;
			const int max_slots_row = 5;
			const int max_rows = 6;
			Draw_Quad* quad_item;	// pointer to item
			Vector2 inventory_bg_pos;
			// float box_width = (max_icons_per_row * icon_width) + ((max_icons_per_row * padding) + padding);

			Vector2 inventory_bg_size = v2(max_slots_row * slot_size + (max_slots_row * padding) + padding * 2, max_rows * slot_size + (max_rows * padding) + padding * 2);
			// Vector2 inventory_bg_pos = v2(screen_width * 0.5 - inventory_bg_size.x * 0.5, screen_height * 0.5 - inventory_bg_size.y * 0.5); // centered in the middle

			if (world->ux_state == UX_chest){
				inventory_bg_pos = v2((screen_width * 0.5) - (inventory_bg_size.x) - 10, (screen_height * 0.5) - (inventory_bg_size.y * 0.5));
			}
			else{
				inventory_bg_pos = v2(padding, screen_height - inventory_bg_size.y - padding);

			}

			// Colors
			Vector4 icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
			Vector4 inventory_bg = v4(0.0, 0.0, 0.0, 0.5);
			Vector4 tooltip_bg = inventory_bg;
			Vector4 slot_color = v4(0.5, 0.5, 0.5, 0.6);

			// init xform
			Matrix4 xform_inventory_bg = m4_identity;

			// xform translate inventory background
			xform_inventory_bg = m4_translate(xform_inventory_bg, v3(inventory_bg_pos.x, inventory_bg_pos.y, 0));

			// draw inventory background and take quad for collisions
			Draw_Quad* quad_bg = draw_rect_xform(xform_inventory_bg, v2(inventory_bg_size.x, inventory_bg_size.y), inventory_bg);

			// position where drawing items starts (top left)
			Vector2 item_start_pos = v2(inventory_bg_pos.x + padding, inventory_bg_pos.y + inventory_bg_size.y - slot_size - padding);

			// item variables
			int slot_index = 0; // basically column index but with sexier name
			int row_index = 0;

			// draw empty slots
			for (int i = 0; i < max_slots_row * max_rows; i++){
				
				if (slot_index >= max_slots_row){
					slot_index = 0;
					row_index++;
				}

				Matrix4 xform_item = m4_identity;
				xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

				// draw empty slot
				draw_rect_xform(xform_item, v2(slot_size, slot_size), slot_color);

				slot_index++;
			}

			// reset these variables
			slot_index = 0;
			row_index = 0;

			// draw items
			for(int i = 0; i < ITEM_MAX; i++){
				InventoryItemData* inventory_item = &world->player->inventory[i];
				if (inventory_item != NULL && inventory_item->amount > 0){

					// change to next row if row is full
					if (slot_index >= max_slots_row){
						slot_index = 0;
						row_index++;
					}

					// get sprite
					Sprite* sprite = get_sprite(inventory_item->sprite_id);

					// skip to next item if sprite is null. NOTE: prolly useless since the "inventory_item != NULL" if statement.
					if (!sprite || !sprite->image){
						continue;
					}

					Matrix4 xform_item = m4_identity;
					// xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding), item_start_pos.y - (row_index * slot_size) - padding, 0));
					xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));


					// save xform_item for later when drawing item counts
					Matrix4 xform_item_count = xform_item;



					// get quad
					quad_item = draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), v4(0,0,0,0));

					slot_index++;

					// hovering item
					if (quad_item && range2f_contains(quad_to_range(*quad_item), get_mouse_pos_in_ndc())){

						// :tooltip
						if (enable_tooltip){
							// const Vector2 tooltip_size = v2(30, 20);
							// Matrix4 xform_tooltip = xform_item;
							// xform_tooltip = m4_translate(xform_tooltip, v3((tooltip_size.x * -0.5) + slot_size * 0.5, -tooltip_size.y - padding, 0));

							Vector2 tooltip_size = v2(inventory_bg_size.x, 30);

							Matrix4 xform_tooltip = m4_identity;

							xform_tooltip = m4_translate(xform_tooltip, v3(inventory_bg_pos.x, inventory_bg_pos.y - tooltip_size.y - padding, 0));



							draw_rect_xform(xform_tooltip, v2(tooltip_size.x, tooltip_size.y), tooltip_bg);

							string item_name = inventory_item->name;

							// TODO fix the damn text rendering position thing pls
							Gfx_Text_Metrics tooltip_metrics = measure_text(font, item_name, font_height, v2(0.1, 0.1));
							// Vector2 tooltip_text_pos = v2(inventory_bg_pos.x + (tooltip_size.x * 0.5) - tooltip_metrics.visual_size.x, inventory_bg_pos.y - tooltip_size.y - padding + tooltip_size.y - 5);
							Vector2 justified = v2_sub(justified, v2_divf(tooltip_metrics.functional_size, 2));
							// Vector2 justified = v2_sub(tooltip_text_pos, tooltip_metrics.visual_pos_min);
							xform_tooltip = m4_translate(xform_tooltip, v3(tooltip_size.x * 0.5, tooltip_size.y - 5, 0));
							// xform_tooltip = m4_translate(xform_tooltip, v3(justified.x, justified.y, 0));
							draw_text_xform(font, item_name, font_height, xform_tooltip, v2(0.1, 0.1), COLOR_WHITE);
							// draw_text(font, item_name, font_height, justified, v2(0.1, 0.1), COLOR_RED);

							if (inventory_bg_pos.x + (tooltip_size.x * 0.5) + (justified.x) < 0){
								log_error("Failing to draw tooltip. its being drawn at %.1f, %.1f\n", inventory_bg_pos.x + (tooltip_size.x * 0.5) + (justified.x), inventory_bg_pos.y - tooltip_size.y - padding + (tooltip_size.y - 5) + (justified.y));
							}
						}

						// scale item
						float scale_adjust = 1.3;
						xform_item = m4_scale(xform_item, v3(scale_adjust, scale_adjust, 1));

						// selecting item
						// if (world->ux_state != UX_chest){
							if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
								consume_key_just_pressed(MOUSE_BUTTON_LEFT);

								inventory_selected_item = *inventory_item;
								delete_item_from_inventory(inventory_item->item_id, inventory_item->amount);
							}
						// }
					} // quad

					// draw item
					draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

					// draw item count (not for tools)
					if (inventory_item->arch != ARCH_tool){
						draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_item->amount), font_height, xform_item_count, v2(0.1, 0.1), COLOR_WHITE);
					}
				} // inventory_item != NULL && inventory_item->amount > 0
			} // for loop


			// dragging release
			if (is_key_up(MOUSE_BUTTON_LEFT) && inventory_selected_item.valid){

				// check if item is released inside of the inventory our outside
				if (range2f_contains(quad_to_range(*quad_bg), get_mouse_pos_in_ndc())){
					printf("Released inside inv\n");
					add_item_to_inventory_quick(&inventory_selected_item);
				}
				else{
					Vector2 pos = get_player_pos();
					if (!spawn_item_to_world(inventory_selected_item, v2(pos.x - 15, pos.y))){
						log_error("FAILED TO SPAWN ITEM TO WORLDSPACE, returning item to inventory\n");
						add_item_to_inventory_quick(&inventory_selected_item);
					}
				}

				// reset selected item. Setting the valid to false functions here like nullptr
				inventory_selected_item.valid = false;
			}
			
			// dragging
			if (inventory_selected_item.valid){
				
				// get mouse pos
				Vector2 mouse_pos_screen = get_mouse_pos_in_screen();

				Matrix4 xform_item_drag = m4_identity;
				xform_item_drag = m4_translate(xform_item_drag, v3(mouse_pos_screen.x, mouse_pos_screen.y, 0));
				xform_item_drag = m4_translate(xform_item_drag, v3(slot_size * -0.5, slot_size * -0.5, 0));

				// draw item
				Draw_Quad* quad_item_drag = draw_image_xform(get_sprite(inventory_selected_item.sprite_id)->image, xform_item_drag, v2(slot_size, slot_size), COLOR_WHITE);
				// draw item count
				// @pin5 categorizing items/tools is fucky
				if (inventory_selected_item_in_chest_ui.arch != ARCH_tool){
					draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_selected_item.amount), font_height, xform_item_drag, v2(0.1, 0.1), COLOR_WHITE);
				}

			}
		}
	}


	// Open Workstation Menu
	if (is_key_just_pressed('C'))
	{
		consume_key_just_pressed('C');
		world->ux_state = (world->ux_state == UX_nil ? UX_workstation : UX_nil);
		world->player->inventory_ui_open = false;
	}
	// :Render workstation menu
	if (world->ux_state == UX_workstation)
	{
		const int workstation_count = WORKSTATION_MAX - 1; // how many different TYPES of workstations there is in the game		// "WORKSTATION_MAX - 1" because we dont want to include the WORKSTATION_nil
		const int workstation_icon_size = 16;
		const int padding = 3;
		const int padding_border = 2;
		int slot_index = 0;
		Vector2 workstation_menu_size = v2((workstation_icon_size * workstation_count) + (padding * (workstation_count - 1)) + (padding_border * 2), workstation_icon_size + (padding * 2));
		Vector2 workstation_menu_pos = v2((screen_width * 0.5) - (workstation_menu_size.x * 0.5), 0);
		Draw_Quad* quad_icon = NULL;

		// colors
		Vector4 icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
		Vector4 workstation_menu_bg_col = v4(0.0, 0.0, 0.0, 0.5);
		Vector4 slot_col = v4(0.3, 0.3, 0.3, 1);
		Vector4 slot_border_col = v4(0.7, 0.7, 0.7, 0.9);
		Vector4 selected_slot_border_col = v4(1, 0, 0, 0.6);


		// draw bg
		Matrix4 xform_workstation_slot_bg = m4_identity;
		xform_workstation_slot_bg = m4_translate(xform_workstation_slot_bg, v3(workstation_menu_pos.x, workstation_menu_pos.y, 0));
		draw_rect_xform(xform_workstation_slot_bg, workstation_menu_size, workstation_menu_bg_col);

		// draw empty slots
		for (int i = 0; i < workstation_count; i++) {

			Vector2 pos = v2(workstation_menu_pos.x + (slot_index * workstation_icon_size) + (slot_index * padding) + padding_border, workstation_menu_pos.y + padding);

			Matrix4 xform_slot = m4_identity;
			xform_slot = m4_translate(xform_slot, v3(pos.x, pos.y, 0));

			// draw_rect_xform(xform_slot, v2(workstation_icon_size, workstation_icon_size), COLOR_RED);
			draw_rect_with_border(xform_slot, v2(workstation_icon_size, workstation_icon_size), 2, slot_col, slot_border_col);

			slot_index++;
		}

		// draw workstation icons
		slot_index = 0;
		for (int i = 0; i < WORKSTATION_MAX; i++){ 

			WorkstationData* workstation = &workstations[i];
			if (workstation->workstation_id != WORKSTATION_nil){

				Sprite* sprite = get_sprite(workstation->sprite_id);
				if (!sprite){
					continue;
				}

				Vector2 pos = v2(workstation_menu_pos.x + (slot_index * workstation_icon_size) + (slot_index * padding) + padding_border, workstation_menu_pos.y + padding);

				Matrix4 xform_icon = m4_identity;
				xform_icon = m4_translate(xform_icon, v3(pos.x, pos.y, 0));
				quad_icon = draw_image_xform(sprite->image, xform_icon, v2(workstation_icon_size, workstation_icon_size), v4(0,0,0,0));

				slot_index++;

				// Hovering 
				if (quad_icon && range2f_contains(quad_to_range(*quad_icon), get_mouse_pos_in_ndc())){
					// printf("HOVER\n");

					float scale_adjust = 2.5;
					// xform_icon = m4_translate(xform_icon, v3(-scale_adjust, -scale_adjust, 0));
					// xform_icon = m4_scale(xform_icon, v3(scale_adjust, scale_adjust, 1));
					xform_icon = m4_translate(xform_icon, v3(0, scale_adjust, 0));
					
					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);
						selected_workstation_buildmode = workstation;
						world->ux_state = UX_place_mode;
					}
				}

				draw_image_xform(sprite->image, xform_icon, v2(workstation_icon_size, workstation_icon_size), COLOR_WHITE);
			}
		}

	}

	
	// :Render workstation placement mode || :Build mode
	if (world->ux_state == UX_place_mode){
		world->player->inventory_ui_open = false;
		if (selected_workstation_buildmode){

			set_world_space();
			
			Vector4 ghost_col = v4(0.5, 0.5, 0.5, 0.7);

			Sprite* sprite = get_sprite(selected_workstation_buildmode->sprite_id);
			Vector2 sprite_size = get_sprite_size(sprite);

			if (!sprite){
				printf("FAILED TO GET SPRITE FROM SELECTED WORKSTATION\n");
			}

			Matrix4 xform_ghost = m4_identity;

			Vector2 mouse_pos = get_mouse_pos_in_world_space();

			xform_ghost = m4_translate(xform_ghost, v3(mouse_pos.x, mouse_pos.y, 0));
			xform_ghost = m4_translate(xform_ghost, v3(sprite_size.x * -0.5, sprite_size.y * -0.5, 0));

			// draw ghost image of workstation
			draw_image_xform(sprite->image, xform_ghost, sprite_size, ghost_col);

			if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);

				// create workstation
				// Entity* en = entity_create();
				Entity* en = entity_create_to_chunk(get_player_chunk());
				setup_workstation(en, selected_workstation_buildmode->workstation_id);
				en->pos = mouse_pos;
				// en->pos = round_v2_to_tile(en->pos);
				selected_workstation_buildmode = NULL;
				world->ux_state = UX_nil;
			}
		}
	}
	// :Render Hotbar || ::hotbar
	if (render_hotbar && world->ux_state != UX_workstation && world->ux_state != UX_place_mode)
	{
		// NOTE: could replace this long if statement with just the "render_hotbar" bool
		const int slot_size = 8;
		const Vector2 padding = v2(2, 1);
		const int slot_count = 9;
		int slot_index = 0;
		float border_width = 1.5;
		Vector2 hotbar_size = v2((slot_size * slot_count) + (padding.x * slot_count) + padding.x, slot_size + (padding.y * 2));
		Vector2 hotbar_pos = v2((screen_width * 0.5) - (hotbar_size.x * 0.5), padding.y);

		// colors
		Vector4 icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
		Vector4 hotbar_bg_col = v4(0.0, 0.0, 0.0, 0.5);
		Vector4 slot_col = v4(0.3, 0.3, 0.3, 1);
		Vector4 slot_border_col = v4(0.7, 0.7, 0.7, 0.9);
		Vector4 selected_slot_border_col = v4(1, 0, 0, 0.6);

		Matrix4 xform_slotbar_bg = m4_identity;
		xform_slotbar_bg = m4_translate(xform_slotbar_bg, v3(hotbar_pos.x, hotbar_pos.y, 0));

		// draw hotbar bg
		draw_rect_xform(xform_slotbar_bg, v2(hotbar_size.x, hotbar_size.y), hotbar_bg_col);

		for (int i = 0; i < slot_count; i++){

			Vector2 pos = v2(hotbar_pos.x + (slot_index * slot_size) + (slot_index * padding.x) + padding.x, hotbar_pos.y + padding.y);

			Matrix4 xform_slot = m4_identity;
			xform_slot = m4_translate(xform_slot, v3(pos.x, pos.y, 0));

			// draw empty slot (with border)
			if (i == selected_slot_index){
				draw_rect_with_border(xform_slot, v2(slot_size, slot_size), border_width * 0.5, slot_col, selected_slot_border_col);
			}
			else{
				draw_rect_with_border(xform_slot, v2(slot_size, slot_size), border_width * 0.5, slot_col, slot_border_col);
			}
			slot_index++;
		}

		// draw items and item counts
		slot_index = 0;
		for (int i = 0; i < slot_count; i++) {
			InventoryItemData* item = &world->player->inventory[i];
			if (item && item->valid){

				Vector2 pos = v2(hotbar_pos.x + (slot_index * slot_size) + (slot_index * padding.x) + padding.x, hotbar_pos.y + padding.y);

				Matrix4 xform_item = m4_identity;
				xform_item = m4_translate(xform_item, v3(pos.x, pos.y, 0));

				Sprite* sprite = get_sprite(item->sprite_id);

				draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

				// draw item count
				if (item->arch != ARCH_tool){
					draw_text_xform(font, sprint(temp_allocator, STR("%d"), item->amount), font_height, xform_item, v2(0.1, 0.1), COLOR_WHITE);
				}
				slot_index++;
			}
		}

		// get selected slot
		for (int i = 0; i < slot_count; i++){
			if (i == selected_slot_index){
				item_in_hand = &world->player->inventory[i];
				continue;
			}
		}

		// "item in hand" rendering is done in the render_entities func @player
	}


	set_world_space();
	pop_z_layer();
}


bool smelt_button(string label, Vector2 pos, Vector2 size, bool enabled) {

	Vector4 color = v4(.25, .25, .25, 1);
	
	float L = pos.x;
	float R = L + size.x;
	float B = pos.y;
	float T = B + size.y;
	
	float mx = input_frame.mouse_x - window.width/2;
	float my = input_frame.mouse_y - window.height/2;

	bool pressed = false;

	if (mx >= L && mx < R && my >= B && my < T) {
		color = v4(.15, .15, .15, 1);
		if (is_key_down(MOUSE_BUTTON_LEFT)) {
			color = v4(.05, .05, .05, 1);
		}
		
		pressed = is_key_just_released(MOUSE_BUTTON_LEFT);
	}
	
	if (enabled) {
		color = v4_add(color, v4(.1, .1, .1, 0));
	}

	draw_rect(pos, size, color);
	
	Gfx_Text_Metrics m = measure_text(font, label, font_height, v2(0.1, 0.1));
	
	Vector2 bottom_left = v2_sub(pos, m.functional_pos_min);
	bottom_left.x += size.x/2;
	bottom_left.x -= m.functional_size.x/2;
	
	bottom_left.y += size.y/2;
	bottom_left.y -= m.functional_size.y/2;
	
	draw_text(font, label, font_height, bottom_left, v2(0.1, 0.1), COLOR_WHITE);
	
	return pressed;
}


// :render workstation ui || ::workstation ui
void render_workstation_ui(UXState ux_state)
{

	// close workstation ui
	if (is_key_just_pressed(KEY_ESCAPE) || is_key_just_pressed(KEY_player_use) || is_key_just_pressed(KEY_toggle_inventory)){
		consume_key_just_pressed(KEY_ESCAPE);
		consume_key_just_pressed(KEY_player_use);
		consume_key_just_pressed(KEY_toggle_inventory);
		world->player->inventory_ui_open = false;
		world->ux_state = UX_nil;
		world->open_crafting_station = NULL;
		return;
	}

	// world->open_crafting_station = en;

	// if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)){
	// 	consume_key_just_pressed(MOUSE_BUTTON_RIGHT);
	// }

	set_screen_space();
	push_z_layer(layer_workstation_ui);

	// :RENDER WORKBENCH UI || :workbench ui
	if (ux_state == UX_workbench){
		world->player->inventory_ui_open = true;

		WorkstationData* selected_workstation = world->player->selected_workstation;

		// printf("RENDERING WORKBENCH UI\n");

		// workbench ui size variables
		const int max_icons_row = 6;		// this controls the size of the ui.  dunno why i got 2 variables for the same thing
		const int MAX_ICONS_PER_ROW = 6;	// this controls actually what is says
		const int max_icons_col = 4;
		const int icon_size = 16;
		const int padding = 2;
		const int padding_bg_vert = 15;
		// int slot_index = 0;
		int row_index = 0;
		int col_index = 0;
		Vector2 icon_pos;

		const Vector2 workbench_ui_size = v2((max_icons_row * icon_size) + (max_icons_row * padding) + padding, (max_icons_col * icon_size) + (max_icons_col * padding) + padding + padding_bg_vert);
		Vector2 workbench_ui_pos = v2(screen_width * 0.5, screen_height * 0.5);
		workbench_ui_pos = v2(workbench_ui_pos.x - (workbench_ui_size.x * 0.5), workbench_ui_pos.y - (workbench_ui_size.y * 0.5));

		// const float x_start_pos = workbench_ui_pos.x;
		// const float y_start_pos = workbench_ui_pos.y;

		// Gfx_Text_Metrics recipe_title;
		// Gfx_Text_Metrics furnace_title;

		// recipe info panel variables
		const Vector2 recipe_panel_size = v2(40, workbench_ui_size.y - 5);
		Vector2 recipe_panel_pos = v2(workbench_ui_pos.x + workbench_ui_size.x + padding, workbench_ui_pos.y + (5 * 0.5));

		


		// Colors
		Vector4 workbench_bg = v4(0.15, 0.15, 0.15, 0.8);
		Vector4 slot_border_color = v4(1, 1, 1, 0.7);
		// Vector4 slot_color = v4(1, 1, 1, 0.7);
		Vector4 craft_button_text_col = v4(0.6, 0.6, 0.6, 1);

		world->workbench_alpha_target = (world->ux_state == UX_workbench ? 1.0 : 0.0);
		animate_f32_to_target(&world->workbench_alpha, world->workbench_alpha_target, delta_t, 15.0);
		bool is_workbench_enabled = world->workbench_alpha_target == 1.0;


		// Workbench PANEL
		if (world->workbench_alpha_target != 0.0)
		{
			// Tabs
			const int tab_count = 4;
			EntityArchetype tab_order[4] = {ARCH_nil, ARCH_item, ARCH_tool, ARCH_workstation};		// compiler gives a warning if using the tab_count as the size of the array. #remember to keep these values the same
			float tab_padding = 4.0;
			const int tab_border_size = 1;
			Vector2 tab_size = v2((workbench_ui_size.x - ((tab_count - 1) * tab_padding)) / tab_count, 10);
			Vector2 tab_pos = v2(workbench_ui_pos.x, workbench_ui_pos.y + workbench_ui_size.y);

			for (int tab_index = 0; tab_index < tab_count; tab_index++){
				// Vector2 pos = v2(tab_pos.x + (tab_index * (tab_size.x + tab_padding)), tab_pos.y);
				Vector2 pos = v2(tab_pos.x + (tab_index * (tab_size.x + tab_padding)), tab_pos.y);
				Matrix4 xform_tab = m4_identity;
				xform_tab = m4_translate(xform_tab, v3(pos.x, pos.y + 1, 0));
				Draw_Quad* tab_quad = draw_rect_with_border(xform_tab, v2(tab_size.x, tab_size.y), tab_border_size, workbench_bg, COLOR_WHITE);

				// draw category icons
				Sprite* category_sprite = get_category_sprite(tab_order[tab_index]);

				xform_tab = m4_translate(xform_tab, v3((tab_size.x * 0.5) - (category_sprite->image->width * 0.5), (tab_size.y * 0.5) - (category_sprite->image->height * 0.5), 0));

				if (workbench_tab_category == tab_order[tab_index]){
					draw_image_xform(category_sprite->image, xform_tab, get_sprite_size(category_sprite), COLOR_RED);
				}
				else{
					draw_image_xform(category_sprite->image, xform_tab, get_sprite_size(category_sprite), COLOR_WHITE);
				}


				if (range2f_contains(quad_to_range(*tab_quad), get_mouse_pos_in_ndc())){
					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);
						workbench_tab_category = tab_order[tab_index];
						selected_recipe_workbench = NULL;
					}
				}
			}


			Matrix4 xform_bg = m4_identity;

			xform_bg = m4_translate(xform_bg, v3(workbench_ui_pos.x, workbench_ui_pos.y, 0));
			// draw backgrounds
			draw_rect_xform(xform_bg, v2(workbench_ui_size.x, workbench_ui_size.y), workbench_bg);

			// center furnace title
			Gfx_Text_Metrics workbench_title = measure_text(font, STR("Workbench"), font_height, v2(0.1, 0.1));
			Vector2 justified1 = v2_sub(justified1, v2_divf(workbench_title.functional_size, 2));
			xform_bg = m4_translate(xform_bg, v3(workbench_ui_size.x * 0.5, workbench_ui_size.y, 0));		// center text box
			xform_bg = m4_translate(xform_bg, v3(justified1.x, justified1.y, 0));						// center text
			xform_bg = m4_translate(xform_bg, v3(0, -5, 0));											// bring down a bit

			// draw title
			draw_text_xform(font, STR("Workbench"), font_height, xform_bg, v2(0.1, 0.1), COLOR_WHITE);
			// printf("drawing furance title\n");

			// draw icons
			Matrix4 xform_icon = m4_identity;

			Vector2 icon_start_pos = v2(workbench_ui_pos.x + padding, workbench_ui_pos.y + workbench_ui_size.y - icon_size - padding - workbench_title.visual_size.y);


			// xform_icon = m4_translate(xform_icon, v3(workbench_ui_size.x * 0.5, workbench_ui_size.y * 0.5, 0));

			// draw icons
			for (int i = 0; i < ITEM_MAX; i++){
				ItemData* item = &crafting_recipes[i];

				// tabs
				if (item->category != workbench_tab_category && workbench_tab_category != ARCH_nil){
					continue;
				}
				
				if (item->crafting_recipe_count != 0){

					if (row_index >= MAX_ICONS_PER_ROW){
						row_index = 0;
						col_index++;
					}

					// printf("ITEM NAME = %s\n", item->name);
					xform_icon = m4_identity;

					Sprite* sprite = get_sprite(item->sprite_id);
					
					icon_pos = v2(icon_start_pos.x + (row_index * (icon_size + padding)), icon_start_pos.y - (col_index * (icon_size + padding)) - 5);
					
					xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

					// draw_image_xform(sprite->image, xform_icon, get_sprite_size(sprite), COLOR_WHITE);
					Draw_Quad* quad = draw_image_xform(sprite->image, xform_icon, v2(icon_size, icon_size), COLOR_WHITE);

					// collision checking for icon
					Range2f icon_box = quad_to_range(*quad);
					if (is_workbench_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc())) {

						// selecting icon
						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
							consume_key_just_pressed(MOUSE_BUTTON_LEFT);

							selected_recipe_workbench = item;
							selected_recipe_xfrom = xform_icon;
							is_recipe_selected = true;

							// printf("selected item = %s\n", selected_recipe_workbench->name);
						}
					}
					row_index++;
				}
			}

			// :RECIPE PANEL || recipe selected || draw recipe panel
			if (selected_recipe_workbench){

				// draw indicator on selected recipe
				draw_rect_xform(m4_translate(selected_recipe_xfrom, v3(0,-1,0)), v2(icon_size, 1), COLOR_WHITE);

				// draw recipe panel
				Matrix4 xform_recipe_panel = m4_identity;
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
				draw_rect_xform(xform_recipe_panel, v2(recipe_panel_size.x, recipe_panel_size.y), workbench_bg);


				// center title
				Gfx_Text_Metrics recipe_title = measure_text(font, STR("Recipe"), font_height, v2(0.1, 0.1));
				Vector2 justified2 = v2_sub(justified2, v2_divf(recipe_title.functional_size, 2));
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_size.x * 0.5, recipe_panel_size.y * 0.75 , 0));	// center text box
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(justified2.x, justified2.y, 0));						// center text
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(0, -5, 0));											// bring down a bit
				// draw_rect_xform(xform_recipe_panel, justified2, COLOR_WHITE);
				draw_text_xform(font, STR("Recipe"), font_height, xform_recipe_panel, v2(0.1, 0.1), COLOR_WHITE);

				// Vector2 panel_icon_start_pos = v2(recipe_panel_pos.x + padding, recipe_panel_pos.y + (recipe_panel_size.y * 0.5) - icon_size - padding - recipe_title.visual_size.y);
				Vector2 panel_icon_start_pos = v2(recipe_panel_pos.x + padding, recipe_panel_pos.y + (recipe_panel_size.y) - recipe_title.visual_size.y - icon_size - padding);

				// draw recipe icon
				Matrix4 xform_recipe_icon = m4_identity;
				Sprite* recipe_icon_sprite = get_sprite(selected_recipe_workbench->sprite_id);
				Vector2 recipe_icon_sprite_size = get_sprite_size(recipe_icon_sprite);
				xform_recipe_icon = m4_translate(xform_recipe_icon, v3(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) - (recipe_icon_sprite_size.x * 0.5), recipe_panel_pos.y + recipe_panel_size.y - recipe_icon_sprite_size.y - 5, 0));
				draw_image_xform(get_sprite(selected_recipe_workbench->sprite_id)->image, xform_recipe_icon, recipe_icon_sprite_size, COLOR_WHITE);

				// draw separator
				Matrix4 separator = m4_identity;
				Vector2 separator_pos = v2(recipe_panel_pos.x, recipe_panel_pos.y + recipe_panel_size.y - recipe_title.visual_size.y - icon_size - (padding * 1));
				separator = m4_translate(separator, v3(separator_pos.x, separator_pos.y, 0));
				draw_rect_xform(separator, v2(recipe_panel_size.x, 1), COLOR_WHITE);

				const int MAX_ICONS_PER_ROW_panel = 1;
				int recipe_icon_index = 0;
				int recipe_row_index = 0;
				int recipe_col_index = 0;
				int recipe_icon_size = 8;
				Vector2 pos;

				// draw recipe materials
				for (int i = 0; i < MAX_RECIPE_ITEMS; i++){
					ItemAmount* recipe_item = &selected_recipe_workbench->crafting_recipe[i];

					// draw recipe icon
					if (recipe_item->amount != 0){

						if (recipe_icon_index >= MAX_ICONS_PER_ROW_panel){
							recipe_icon_index = 0;
							recipe_col_index++;
						}

						Matrix4 xform = m4_identity;
						Sprite* sprite = get_sprite(get_sprite_from_itemID(recipe_item->id));

						Gfx_Text_Metrics recipe_material_amount = measure_text(font, STR("0/0"), font_height, v2(0.1, 0.1));

						// icon_pos = v2(icon_panel_icon_start_pos.x + (row_index * (icon_size + padding)), icon_panel_icon_start_pos.y - (col_index * (icon_size + padding)));
						// xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

						// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));
						// pos = v2(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) + (recipe_icon_index * (icon_size + padding)) - (icon_size * 0.5), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));
						// pos = v2(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) + (recipe_icon_index * (icon_size + padding)) - (icon_size * 0.5), separator_pos.y - padding - (recipe_col_index * recipe_icon_size) - (recipe_col_index * padding) - recipe_icon_size - recipe_title.visual_size.y - padding);
						pos = v2(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) - (recipe_icon_index * (recipe_icon_size + (padding * 2) + recipe_material_amount.visual_size.x) - recipe_material_amount.visual_size.x), separator_pos.y - padding - (recipe_col_index * recipe_icon_size) - (recipe_col_index * padding) - recipe_icon_size - recipe_title.visual_size.y - padding);

						// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y);

						xform = m4_translate(xform, v3(pos.x, pos.y, 0));
						
						Draw_Quad* recipe_material_quad = draw_image_xform(sprite->image, xform, v2(recipe_icon_size, recipe_icon_size), COLOR_WHITE);

						draw_text_xform(font, sprint(temp_allocator, STR("%d/%d"), get_player_inventory_item_count(recipe_item->id), recipe_item->amount), font_height, m4_translate(xform, v3(-recipe_icon_size, (recipe_icon_size * 0.5) - (recipe_material_amount.visual_size.y * 0.5), 0)), v2(0.1, 0.1), COLOR_WHITE);

						recipe_icon_index++;

						// draw recipe material name
						if (range2f_contains(quad_to_range(*recipe_material_quad), get_mouse_pos_in_ndc())){
							Matrix4 xform = m4_identity;
							xform = m4_translate(xform, v3(get_mouse_pos_in_screen().x, get_mouse_pos_in_screen().y, 0));
							draw_text_xform(font, sprint(temp_allocator, STR("%s"), get_item_name(recipe_item->id)), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);
						}
					}
				}

				// update craft button color if enough items for crafting the recipe
				bool enough_items_for_recipe = true;
				for (int i = 0; i < selected_recipe_workbench->crafting_recipe_count; i++){
					ItemAmount* recipe_item = &selected_recipe_workbench->crafting_recipe[i];
					if (!check_player_inventory_for_items(recipe_item->id, recipe_item->amount)){
						enough_items_for_recipe = false;
					}
				}

				if (enough_items_for_recipe){
					craft_button_text_col = COLOR_GREEN;
				}
				else{
					craft_button_text_col = COLOR_RED;
				}


				// Craft button
				Matrix4 craft_xform = m4_identity;
				craft_xform = m4_translate(craft_xform, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
				Draw_Quad* quad = draw_rect_xform(craft_xform, v2(recipe_panel_size.x, 10), v4(0.5, 0.5, 0.5, 0.5));
				Gfx_Text_Metrics craft_text = measure_text(font, STR("Craft"), font_height, v2(0.1, 0.1));
				Vector2 justified3 = v2_sub(justified3, v2_divf(craft_text.functional_size, 2));
				craft_xform = m4_translate(craft_xform, v3(recipe_panel_size.x * 0.5, 4, 0));	// center text box
				craft_xform = m4_translate(craft_xform, v3(justified3.x, justified3.y, 0));						// center text
				draw_text_xform(font, STR("Craft"), font_height, craft_xform, v2(0.1, 0.1), craft_button_text_col);

				// selecting craft button
				if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())) {
					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);

						int result = 0;
	
						// check if player has the required items in inventory
						for (int i = 0; i < selected_recipe_workbench->crafting_recipe_count; i++){

							ItemAmount* recipe_item = &selected_recipe_workbench->crafting_recipe[i];
							// printf("searching for itemID '%d'\n", recipe_item->id);

							result += check_player_inventory_for_items(recipe_item->id, recipe_item->amount);
						}

						// :CRAFT ITEM
						if (result >= selected_recipe_workbench->crafting_recipe_count){
							
							
							selected_workstation->selected_crafting_item = selected_recipe_workbench;
							selected_workstation->crafting_queue++;
							delete_recipe_items_from_inventory(*selected_recipe_workbench);
						}
					}
				}
			}

			if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);
				selected_recipe_workbench = NULL;
				selected_recipe_xfrom = m4_identity;
				is_recipe_selected = false;
			}
		}
	}


	// :RENDER CHEST UI || :Chest ui
	else if (ux_state == UX_chest){
		world->player->inventory_ui_open = false;

		// shared variables
		const int middle_padding = 10;
		const int slot_size = 8;
		const float padding = 2.0;
		float slot_border_width = 1;
		Vector4 tooltip_bg;

		// INVENTORY VARIABLES
		const int max_slots_row_inventory = 5;
		const int max_rows_inventory = 6;
		Draw_Quad* quad_item_inventory;	// pointer to item
		Vector2 inventory_bg_pos;

		Vector2 inventory_bg_size = v2(max_slots_row_inventory * slot_size + (max_slots_row_inventory * padding) + padding * 2, max_rows_inventory * slot_size + (max_rows_inventory * padding) + padding * 2);
		inventory_bg_pos = v2((screen_width * 0.5) - (inventory_bg_size.x) - middle_padding, (screen_height * 0.5) - (inventory_bg_size.y * 0.5));

		// Colors
		Vector4 inventory_icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
		Vector4 inventory_bg = v4(0.0, 0.0, 0.0, 0.5);
		Vector4 slot_color = v4(0.5, 0.5, 0.5, 0.6);
		tooltip_bg = inventory_bg;

		// init xform
		Matrix4 xform_inventory_bg = m4_identity;

		// xform translate inventory background
		xform_inventory_bg = m4_translate(xform_inventory_bg, v3(inventory_bg_pos.x, inventory_bg_pos.y, 0));

		// draw inventory background and take quad for collisions
		Draw_Quad* inventory_quad = draw_rect_xform(xform_inventory_bg, v2(inventory_bg_size.x, inventory_bg_size.y), inventory_bg);

		// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

		// CHEST VARIABLES
		const int max_slots_row_chest = 5;
		const int max_rows_chest = 6;

		const Vector2 chest_ui_size = v2((max_slots_row_chest * slot_size) + (max_slots_row_chest * padding) + padding * 2, (max_rows_chest * slot_size) + (max_rows_chest * padding) + padding * 2);
		Vector2 chest_ui_pos = v2((screen_width * 0.5) + middle_padding, (screen_height * 0.5) - (chest_ui_size.y * 0.5));

		Draw_Quad* quad_item_chest;	// pointer to item

		// Colors
		Vector4 chest_icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
		Vector4 chest_bg = v4(0.0, 0.0, 0.0, 0.5);
		Vector4 slot_border_color = v4(1, 1, 1, 0.7);

		Matrix4 xform_chest_bg = m4_identity;
		xform_chest_bg = m4_translate(xform_chest_bg, v3(chest_ui_pos.x, chest_ui_pos.y, 0));

		// draw chest background and take quad for collisions
		chest_quad = draw_rect_xform(xform_chest_bg, v2(chest_ui_size.x, chest_ui_size.y), chest_bg);

		// @pin4 could this be the issue why pointer points to player inventory instead of chest inventory?
		//                            VV     '&' missing
	    WorkstationData* selected_chest = world->player->selected_workstation;

		world->chest_alpha_target = (world->ux_state == UX_chest ? 1.0 : 0.0);
		animate_f32_to_target(&world->chest_alpha, world->chest_alpha_target, delta_t, 15.0);
		bool is_chest_enabled = world->chest_alpha_target == 1.0;
		
		if (world->chest_alpha_target != 0.0)
		{
			// INVENTORY UI ------------------------------------------------------>
			{
				// position where drawing items starts (top left)
				Vector2 item_start_pos = v2(inventory_bg_pos.x + padding, inventory_bg_pos.y + inventory_bg_size.y - slot_size - padding);

				// item variables
				int slot_index = 0; // basically column index but with sexier name
				int row_index = 0;

				// draw empty slots
				for (int i = 0; i < max_slots_row_inventory * max_rows_inventory; i++){
					
					if (slot_index >= max_slots_row_inventory){
						slot_index = 0;
						row_index++;
					}

					Matrix4 xform_item = m4_identity;
					xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

					// draw empty slot
					draw_rect_xform(xform_item, v2(slot_size, slot_size), slot_color);
					// TODO: make it a rect with border 'draw_rect_with_border'

					slot_index++;
				}

				// reset these variables
				slot_index = 0;
				row_index = 0;

				// draw items
				for(int i = 0; i < ITEM_MAX; i++){
					InventoryItemData* inventory_item = &world->player->inventory[i];
					if (inventory_item != NULL && inventory_item->amount > 0){

						// change to next row if row is full
						if (slot_index >= max_slots_row_inventory){
							slot_index = 0;
							row_index++;
						}

						// get sprite
						Sprite* sprite = get_sprite(inventory_item->sprite_id);

						// skip to next item if sprite is null. NOTE: prolly useless since the "inventory_item != NULL" if statement.
						if (!sprite || !sprite->image){
							continue;
						}

						Matrix4 xform_item = m4_identity;
						xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

						// save xform_item for later when drawing item counts
						Matrix4 xform_item_count = xform_item;

						// get quad
						quad_item_inventory = draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), v4(0,0,0,0));

						slot_index++;

						// hovering item
						if (quad_item_inventory && range2f_contains(quad_to_range(*quad_item_inventory), get_mouse_pos_in_ndc())){

							// tooltip
							if (enable_tooltip){

								Vector2 tooltip_size = v2(inventory_bg_size.x, 30);

								Matrix4 xform_tooltip = m4_identity;
								xform_tooltip = m4_translate(xform_tooltip, v3(inventory_bg_pos.x, inventory_bg_pos.y - tooltip_size.y - padding, 0));

								draw_rect_xform(xform_tooltip, v2(tooltip_size.x, tooltip_size.y), tooltip_bg);

								string item_name = inventory_item->name;

								Gfx_Text_Metrics tooltip_metrics = measure_text(font, item_name, font_height, v2(0.1, 0.1));
								Vector2 justified = v2_sub(justified, v2_divf(tooltip_metrics.functional_size, 2));
								xform_tooltip = m4_translate(xform_tooltip, v3(tooltip_size.x * 0.5, tooltip_size.y - 5, 0));
								xform_tooltip = m4_translate(xform_tooltip, v3(justified.x, justified.y, 0));
								draw_text_xform(font, item_name, font_height, xform_tooltip, v2(0.1, 0.1), COLOR_WHITE);
							}

							// scale item
							float scale_adjust = 1.3;
							xform_item = m4_scale(xform_item, v3(scale_adjust, scale_adjust, 1));

							// selecting item
							if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
								consume_key_just_pressed(MOUSE_BUTTON_LEFT);

								inventory_selected_item_in_chest_ui = *inventory_item;
								delete_item_from_inventory(inventory_item->item_id, inventory_item->amount);
							}
						} // quad

						// draw item
						draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

						// draw item count (not tools)
						if (inventory_item->arch != ARCH_tool){
							draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_item->amount), font_height, xform_item_count, v2(0.1, 0.1), COLOR_WHITE);
						}
					} // inventory_item != NULL && inventory_item->amount > 0
				} // for loop


				// dragging release
				if (is_key_up(MOUSE_BUTTON_LEFT) && inventory_selected_item_in_chest_ui.valid){

					// check if item is released inside of the inventory our outside
					if (range2f_contains(quad_to_range(*inventory_quad), get_mouse_pos_in_ndc())){
						add_item_to_inventory_quick(&inventory_selected_item_in_chest_ui);
					}
					else{
						// check if item is released into chest
						if (chest_quad && range2f_contains(quad_to_range(*chest_quad), get_mouse_pos_in_ndc())){
							add_item_to_chest(inventory_selected_item_in_chest_ui);
						}
						else{
							// TODO: make the item spawn towards the mouse cursor from player
							Vector2 pos = get_player_pos();
							if (!spawn_item_to_world(inventory_selected_item_in_chest_ui, v2(pos.x - 15, pos.y))){
								log_error("FAILED TO SPAWN ITEM TO WORLDSPACE, returning item to inventory\n");
								add_item_to_inventory_quick(&inventory_selected_item_in_chest_ui);
							}
						}
					}

					// reset selected item. Setting the valid to false functions here like a nullptr
					inventory_selected_item_in_chest_ui.valid = false;
				}
				
				// dragging from inventory
				if (inventory_selected_item_in_chest_ui.valid){

					// get mouse pos
					Vector2 mouse_pos_screen = get_mouse_pos_in_screen();

					Matrix4 xform_item_drag = m4_identity;
					xform_item_drag = m4_translate(xform_item_drag, v3(mouse_pos_screen.x, mouse_pos_screen.y, 0));
					xform_item_drag = m4_translate(xform_item_drag, v3(slot_size * -0.5, slot_size * -0.5, 0));

					// draw item
					Draw_Quad* quad_item_drag = draw_image_xform(get_sprite(inventory_selected_item_in_chest_ui.sprite_id)->image, xform_item_drag, v2(slot_size, slot_size), COLOR_WHITE);
					// draw item amount
					if (inventory_selected_item_in_chest_ui.arch != ARCH_tool){
						draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_selected_item_in_chest_ui.amount), font_height, xform_item_drag, v2(0.1, 0.1), COLOR_WHITE);
					}

				}
			}
			// inventory ui end


			// CHEST UI ---------------------------------------------------------->
			{
				// position where drawing items starts (top left)
				Vector2 item_start_pos = v2(chest_ui_pos.x + padding, chest_ui_pos.y + chest_ui_size.y - slot_size - padding);

				// item variables
				int slot_index = 0; // basically column index but with sexier name
				int row_index = 0;

				// draw empty slots
				for (int i = 0; i < max_slots_row_chest * max_rows_chest; i++){
					
					if (slot_index >= max_slots_row_chest){
						slot_index = 0;
						row_index++;
					}

					Matrix4 xform_item = m4_identity;
					xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

					// draw empty slot
					draw_rect_xform(xform_item, v2(slot_size, slot_size), slot_color);
					// TODO: make it a rect with border 'draw_rect_with_border'

					slot_index++;
				}

				// reset these variables
				slot_index = 0;
				row_index = 0;

				// NOTE: this is how to add items straight to the chest inventory
				// world->dimension->entities[937].workstation_data.inventory[0] = (InventoryItemData){.item_id=ITEM_berry, .amount=1,.arch=ARCH_item,.name=STR("BERRY"),.valid=true,.sprite_id=SPRITE_item_berry};

				// draw items
				for(int i = 0; i < ITEM_MAX; i++){
					InventoryItemData* chest_item = &selected_chest->inventory[i];
					if (chest_item != NULL && chest_item->amount > 0){

						// change to next row if row is full
						if (slot_index >= max_slots_row_chest){
							slot_index = 0;
							row_index++;
						}

						// get sprite
						Sprite* sprite = get_sprite(chest_item->sprite_id);

						// skip to next item if sprite is null. NOTE: prolly useless since the "chest_item != NULL" if statement.
						if (!sprite || !sprite->image){
							continue;
						}

						Matrix4 xform_item = m4_identity;
						xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

						// save xform_item for later when drawing item counts
						Matrix4 xform_item_count = xform_item;

						// get quad
						quad_item_chest = draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), v4(0,0,0,0));

						slot_index++;

						// hovering item
						if (quad_item_chest && range2f_contains(quad_to_range(*quad_item_chest), get_mouse_pos_in_ndc())){

							// :tooltip
							if (enable_chest_tooltip){

								Vector2 tooltip_size = v2(chest_ui_size.x, 30);

								Matrix4 xform_tooltip = m4_identity;
								xform_tooltip = m4_translate(xform_tooltip, v3(chest_ui_pos.x, chest_ui_pos.y - tooltip_size.y - padding, 0));

								draw_rect_xform(xform_tooltip, v2(tooltip_size.x, tooltip_size.y), tooltip_bg);

								string item_name = chest_item->name;

								Gfx_Text_Metrics tooltip_metrics = measure_text(font, item_name, font_height, v2(0.1, 0.1));
								Vector2 justified = v2_sub(justified, v2_divf(tooltip_metrics.functional_size, 2));
								xform_tooltip = m4_translate(xform_tooltip, v3(tooltip_size.x * 0.5, tooltip_size.y - 5, 0));
								xform_tooltip = m4_translate(xform_tooltip, v3(justified.x, justified.y, 0));
								draw_text_xform(font, item_name, font_height, xform_tooltip, v2(0.1, 0.1), COLOR_WHITE);
							}

							// scale item
							float scale_adjust = 1.3;
							xform_item = m4_scale(xform_item, v3(scale_adjust, scale_adjust, 1));

							// selecting item
							if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
								consume_key_just_pressed(MOUSE_BUTTON_LEFT);

								chest_selected_item = *chest_item;
								delete_item_from_chest(chest_item->item_id, chest_item->amount);
							}
						} // quad

						// draw item
						draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

						// draw item count (not for tools)
						if (chest_item->arch != ARCH_tool){
							draw_text_xform(font, sprint(temp_allocator, STR("%d"), chest_item->amount), font_height, xform_item_count, v2(0.1, 0.1), COLOR_WHITE);
						}
					} // chest_item != NULL && chest_item->amount > 0
				} // for loop

				// dragging release
				if (is_key_up(MOUSE_BUTTON_LEFT) && chest_selected_item.valid){

					// check if item is released inside of the inventory our outside
					if (range2f_contains(quad_to_range(*chest_quad), get_mouse_pos_in_ndc())){
						add_item_to_chest(chest_selected_item);
					}
					else{
						// check if item is released into inventory
						if (inventory_quad && range2f_contains(quad_to_range(*inventory_quad), get_mouse_pos_in_ndc())){
							add_item_to_inventory_quick(&chest_selected_item);
						}
						else{
							// TODO: make the item spawn towards the mouse cursor from player
							Vector2 pos = get_player_pos();
							if (!spawn_item_to_world(chest_selected_item, v2(pos.x - 15, pos.y))){
								log_error("FAILED TO SPAWN ITEM TO WORLDSPACE, returning item to inventory\n");
								add_item_to_inventory_quick(&chest_selected_item);
							}
						}
					}

					// reset selected item. Setting the valid to false functions here like nullptr
					chest_selected_item.valid = false;
				}
				
				// dragging from chest
				if (chest_selected_item.valid){

					// get mouse pos
					Vector2 mouse_pos_screen = get_mouse_pos_in_screen();

					Matrix4 xform_item_drag = m4_identity;
					xform_item_drag = m4_translate(xform_item_drag, v3(mouse_pos_screen.x, mouse_pos_screen.y, 0));
					xform_item_drag = m4_translate(xform_item_drag, v3(slot_size * -0.5, slot_size * -0.5, 0));

					// draw item
					Draw_Quad* quad_item_drag = draw_image_xform(get_sprite(chest_selected_item.sprite_id)->image, xform_item_drag, v2(slot_size, slot_size), COLOR_WHITE);
					// draw item amount
					if (chest_selected_item.arch != ARCH_tool){
						draw_text_xform(font, sprint(temp_allocator, STR("%d"), chest_selected_item.amount), font_height, xform_item_drag, v2(0.1, 0.1), COLOR_WHITE);
					}
				}
			} // chest ui end
		}

		// prevent player attacking when chest ui is open
		if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
			consume_key_just_pressed(MOUSE_BUTTON_LEFT);
		}
	}


	// :RENDER FURNACE UI || :furnace ui
	else if (ux_state == UX_furnace){
		world->player->inventory_ui_open = true;
		// printf("RENDERING FURNACE UI\n");

		WorkstationData* selected_workstation = world->player->selected_workstation;

		// furnace ui size variables
		const int max_icons_row = 6;
		const int max_icons_col = 4;
		const int icon_size = 16;
		const int padding = 2;
		const int padding_bg_vert = 15;
		// int slot_index = 0;
		int row_index = 0;
		int col_index = 0;
		Vector2 icon_pos;
		const int MAX_ICONS_PER_ROW = 3;
		

		// const Vector2 furnace_ui_size = v2((max_slots_row * slot_size) + (max_slots_row * padding) + padding, (max_slots_col * slot_size) + (max_slots_col * padding) + padding);
		// const Vector2 furnace_ui_size = v2((max_icons_row * icon_size) + (max_icons_row * padding) + padding, (max_icons_col * icon_size) + (max_icons_col * padding) + padding + padding_bg_vert);
		const Vector2 furnace_ui_size = v2((max_icons_row * icon_size) + (max_icons_row * padding) + padding, (max_icons_col * icon_size) + (max_icons_col * padding) + padding + padding_bg_vert);
		Vector2 furnace_ui_pos = v2(screen_width * 0.5, screen_height * 0.5);
		furnace_ui_pos = v2(furnace_ui_pos.x - (furnace_ui_size.x * 0.5), furnace_ui_pos.y - (furnace_ui_size.y * 0.5));

		// const float x_start_pos = furnace_ui_pos.x;
		// const float y_start_pos = furnace_ui_pos.y;

		// Gfx_Text_Metrics recipe_title;
		// Gfx_Text_Metrics furnace_title;

		// recipe info panel variables
		const Vector2 recipe_panel_size = v2(40, furnace_ui_size.y - 5);
		Vector2 recipe_panel_pos = v2(furnace_ui_pos.x + furnace_ui_size.x + padding, furnace_ui_pos.y + (5 * 0.5));

		


		// Colors
		Vector4 furnace_bg = v4(0.15, 0.15, 0.15, 0.8);
		Vector4 slot_border_color = v4(1, 1, 1, 0.7);
		// Vector4 slot_color = v4(1, 1, 1, 0.7);

		world->furnace_alpha_target = (world->ux_state == UX_furnace ? 1.0 : 0.0);
		animate_f32_to_target(&world->furnace_alpha, world->furnace_alpha_target, delta_t, 15.0);
		bool is_furnace_enabled = world->furnace_alpha_target == 1.0;


		// FURNACE PANEL
		if (world->furnace_alpha_target != 0.0)
		{
			Matrix4 xform_bg = m4_identity;

			xform_bg = m4_translate(xform_bg, v3(furnace_ui_pos.x, furnace_ui_pos.y, 0));
			// draw backgrounds
			draw_rect_xform(xform_bg, v2(furnace_ui_size.x, furnace_ui_size.y), furnace_bg);

			// center furnace title
			Gfx_Text_Metrics furnace_title = measure_text(font, STR("Furnace"), font_height, v2(0.1, 0.1));
			Vector2 justified1 = v2_sub(justified1, v2_divf(furnace_title.functional_size, 2));
			xform_bg = m4_translate(xform_bg, v3(furnace_ui_size.x * 0.5, furnace_ui_size.y, 0));		// center text box
			xform_bg = m4_translate(xform_bg, v3(justified1.x, justified1.y, 0));						// center text
			xform_bg = m4_translate(xform_bg, v3(0, -5, 0));											// bring down a bit

			// draw title
			draw_text_xform(font, STR("Furnace"), font_height, xform_bg, v2(0.1, 0.1), COLOR_WHITE);
			// printf("drawing furance title\n");

			// draw icons
			Matrix4 xform_icon = m4_identity;

			Vector2 icon_start_pos = v2(furnace_ui_pos.x + padding, furnace_ui_pos.y + furnace_ui_size.y - icon_size - padding - furnace_title.visual_size.y);


			// xform_icon = m4_translate(xform_icon, v3(furnace_ui_size.x * 0.5, furnace_ui_size.y * 0.5, 0));

			// draw icons
			for (int i = 0; i < ITEM_MAX; i++){
				ItemData* item = &furnace_recipes[i];

				if (item->crafting_recipe_count != 0){

					if (row_index >= MAX_ICONS_PER_ROW){
						row_index = 0;
						col_index++;
					}

					// printf("ITEM NAME = %s\n", item->name);
					xform_icon = m4_identity;

					Sprite* sprite = get_sprite(item->sprite_id);
					
					icon_pos = v2(icon_start_pos.x + (row_index * (icon_size + padding)), icon_start_pos.y - (col_index * (icon_size + padding)) - 5);
					
					xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

					// draw_image_xform(sprite->image, xform_icon, get_sprite_size(sprite), COLOR_WHITE);
					Draw_Quad* quad = draw_image_xform(sprite->image, xform_icon, v2(icon_size, icon_size), COLOR_WHITE);

					// collision checking for icon
					Range2f icon_box = quad_to_range(*quad);
					if (is_furnace_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc())) {

						// selecting icon
						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
							consume_key_just_pressed(MOUSE_BUTTON_LEFT);

							selected_recipe_furnace = item;
							selected_recipe_xfrom = xform_icon;
							is_recipe_selected = true;

							// printf("selected item = %s\n", selected_recipe_furnace->name);
						}
					}
					row_index++;
				}
			}

			// :RECIPE PANEL || recipe selected || draw recipe panel
			if (selected_recipe_furnace){

				// draw indicator on selected recipe
				draw_rect_xform(m4_translate(selected_recipe_xfrom, v3(0,-1,0)), v2(icon_size, 1), COLOR_WHITE);

				// draw recipe panel
				Matrix4 xform_recipe_panel = m4_identity;
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
				draw_rect_xform(xform_recipe_panel, v2(recipe_panel_size.x, recipe_panel_size.y), furnace_bg);


				// center title
				Gfx_Text_Metrics recipe_title = measure_text(font, STR("Recipe"), font_height, v2(0.1, 0.1));
				Vector2 justified2 = v2_sub(justified2, v2_divf(recipe_title.functional_size, 2));
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_size.x * 0.5, recipe_panel_size.y * 0.75 , 0));	// center text box
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(justified2.x, justified2.y, 0));						// center text
				xform_recipe_panel = m4_translate(xform_recipe_panel, v3(0, -5, 0));											// bring down a bit
				// draw_rect_xform(xform_recipe_panel, justified2, COLOR_WHITE);
				draw_text_xform(font, STR("Recipe"), font_height, xform_recipe_panel, v2(0.1, 0.1), COLOR_WHITE);

				Vector2 panel_icon_start_pos = v2(recipe_panel_pos.x + padding, recipe_panel_pos.y + (recipe_panel_size.y * 0.5) - icon_size - padding - recipe_title.visual_size.y);

				// draw recipe icon
				Matrix4 xform_recipe_icon = m4_identity;
				Sprite* recipe_icon_sprite = get_sprite(selected_recipe_furnace->sprite_id);
				Vector2 recipe_icon_sprite_size = get_sprite_size(recipe_icon_sprite);
				xform_recipe_icon = m4_translate(xform_recipe_icon, v3(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) - (recipe_icon_sprite_size.x * 0.5), recipe_panel_pos.y + recipe_panel_size.y - recipe_icon_sprite_size.y - 5, 0));
				draw_image_xform(get_sprite(selected_recipe_furnace->sprite_id)->image, xform_recipe_icon, recipe_icon_sprite_size, COLOR_WHITE);

				const int MAX_ICONS_PER_ROW_panel = 5;
				int recipe_icon_index = 0;
				int recipe_row_index = 0;
				int recipe_col_index = 0;
				Vector2 pos;

				// draw recipe materials
				for (int i = 0; i < MAX_RECIPE_ITEMS; i++){
					ItemAmount* recipe_item = &selected_recipe_furnace->crafting_recipe[i];

					// draw recipe icon
					if (recipe_item->amount != 0){

						if (recipe_icon_index >= MAX_ICONS_PER_ROW_panel){
							recipe_icon_index = 0;
							recipe_col_index++;
						}

						Matrix4 xform = m4_identity;
						Sprite* sprite = get_sprite(get_sprite_from_itemID(recipe_item->id));

						// icon_pos = v2(icon_panel_icon_start_pos.x + (row_index * (icon_size + padding)), icon_panel_icon_start_pos.y - (col_index * (icon_size + padding)));
						// xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

						// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));
						pos = v2(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) + (recipe_icon_index * (icon_size + padding)) - (icon_size * 0.5), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));

						// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y);

						xform = m4_translate(xform, v3(pos.x, pos.y, 0));
						
						draw_image_xform(sprite->image, xform, v2(icon_size, icon_size), COLOR_WHITE);
						draw_text_xform(font, sprint(temp_allocator, STR("%d/%d"), get_player_inventory_item_count(recipe_item->id), recipe_item->amount), font_height, m4_translate(xform, v3(-8, + (icon_size * 0.5), 0)), v2(0.1, 0.1), COLOR_WHITE);

						recipe_icon_index++;
					}

				}

				// SMELT BUTTON
				// smelt_button(STR("Smelt"), v2(recipe_panel_pos.x, recipe_panel_pos.y), v2(recipe_panel_size.x, 10), true);

				// if (smelt_button){
				// 	printf("PRESSED_BUTTON\n");
				// }
				// if (button(STR("Song"), rect.xy, rect.zw, song_playing)) {

				Matrix4 smelt_xform = m4_identity;
				smelt_xform = m4_translate(smelt_xform, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
				Draw_Quad* quad = draw_rect_xform(smelt_xform, v2(recipe_panel_size.x, 10), v4(0.5, 0.5, 0.5, 0.5));
				Gfx_Text_Metrics smelt_text = measure_text(font, STR("Smelt"), font_height, v2(0.1, 0.1));
				Vector2 justified3 = v2_sub(justified3, v2_divf(smelt_text.functional_size, 2));
				smelt_xform = m4_translate(smelt_xform, v3(recipe_panel_size.x * 0.5, 4, 0));	// center text box
				smelt_xform = m4_translate(smelt_xform, v3(justified3.x, justified3.y, 0));						// center text
				// smelt_xform = m4_translate(smelt_xform, v3(0, -5, 0));											// bring down a bit
				draw_text_xform(font, STR("Smelt"), font_height, smelt_xform, v2(0.1, 0.1), COLOR_WHITE);

				// selecting smelt button
				if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())) {
					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);
						printf("SMELT\n");
						for (int i = 0; i < selected_recipe_furnace->crafting_recipe_count; i++){
							ItemAmount* recipe_item = &selected_recipe_furnace->crafting_recipe[i];
							printf("searching for itemID '%d'\n", recipe_item->id);
							bool result = check_player_inventory_for_items(recipe_item->id, recipe_item->amount);
							printf("RESULT = %d\n", result);

							if (result == 1){

								selected_workstation->selected_crafting_item = selected_recipe_furnace;
								selected_workstation->crafting_queue++;

								delete_item_from_inventory(recipe_item->id, recipe_item->amount);
								// add_item_to_inventory(selected_recipe_furnace->item_id, selected_recipe_furnace->name, 1, ARCH_item, selected_recipe_furnace->sprite_id, TOOL_nil, true);
							}
						}
					}
				}
			}

			








			if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);
				selected_recipe_furnace = NULL;
				selected_recipe_xfrom = m4_identity;
				is_recipe_selected = false;
			}

		}
	}



	set_world_space();
	pop_z_layer();
}



// :load dimension entities
// void load_dimension_entities(DimensionID id, Vector2 dest_pos){
	
// 	// BiomeID biome_id = world->dimension->biomes[0];
// 	BiomeID biome_id = world->dimension->current_biome_id;
// 	int min_entity_count = 10;	// spawn dimension entities only if "world->dimension->entity_count" is kinda low (min_entity_count). This fixes the issue of new entities spawning every time the dimension is changed -> MAX_ENTITY_COUNT is reached easily -> crash
// 	// TODO: find a better solution for this mess

// 	switch (id){
// 		case DIM_overworld:
// 		{
// 			if (get_dimensionData(id)->entity_count < min_entity_count){ 
// 				// spawn_biome(&biomes[biome_id]);
// 				// printf("Spawned overworld entities\n");
// 			}
// 		} break;

// 		case DIM_cavern:
// 		{
// 			if (get_dimensionData(id)->entity_count < min_entity_count){
// 				// spawn_biome(&biomes[biome_id]);
// 				// printf("Spawned cavern entities\n");
// 			}
// 		} break;

// 		default:{}break;
// 	}

// 	world->player->en->pos = dest_pos;
// 	// world->player->en->pos = get_player_en_from_dim(id)->pos;

// 	// check if player entity already exists in the destination dimension
// 	for (int i = 0; i < world->dimension->entity_count; i++){
// 		Entity* en = &world->dimension->entities[i];
// 		if (en->arch == ARCH_player){
// 			world->player->en->pos = dest_pos;
// 			return;
// 		}

// 	}
// 	// if not, add player entity to destination dimension
// 	world->dimension->entities[world->dimension->entity_count] = *get_player();
// 	world->dimension->entity_count++;
// }

// :change dimension
void change_dimensions(DimensionID new_dim, Vector2 dest_pos){
	printf("Changing DIMENSION: %s -> %s\n", world->dimension->name, get_dimensionData(new_dim)->name);
	// world->player->en->pos = dest_pos;
	// printf("Changed window color form %.0f, %.0f, %.0f", window.clear_color.r, window.clear_color.g, window.clear_color.b);
	// printf("   to   %.0f, %.0f, %.0f\n", window.clear_color.r, window.clear_color.g, window.clear_color.b);
	
	// world->dimension_id = new_dim;
	world->dimension = get_dimensionData(new_dim);
	world->dimension->id = new_dim;
	// world->current_biome_id = world->dimension->biomes[0];
	// load_dimension_entities(world->dimension_id, dest_pos);
	// load_dimension_entities(world->dimension->id, dest_pos);
	// window.clear_color = get_dimensionData(new_dim)->ground_color;

	if (new_dim == DIM_overworld) view_zoom = 0.2;
	else if (new_dim == DIM_cavern) view_zoom = 0.11;

	// if (new_dim == DIM_cavern){
	// 	trigger_dim_change_animation(camera_pos);
	// }
}


// not in use
	// void sort_entities_by_prio_and_y(Entity* entities, int count) {
	//     // sorts entities:
	//     // primary sort: sort entities based on their rendering_prio value
	//     // secondary sort: if rendering_prio is the same, sort based on their y coordinates in descending order

	//     for (int i = 0; i < count - 1; i++) {
	//         for (int j = 0; j < count - i - 1; j++) {
	//             int prio1 = entities[j].rendering_prio;
	//             int prio2 = entities[j + 1].rendering_prio;
	//             int y1 = entities[j].pos.y;
	//             int y2 = entities[j + 1].pos.y;

	//             // Sort by rendering priority first (ascending order)
	//             // If the priorities are equal, sort by y value (descending order)
	//             if (prio1 > prio2 || (prio1 == prio2 && y1 < y2)) {
	//                 Entity temp = entities[j];
	//                 entities[j] = entities[j + 1];
	//                 entities[j + 1] = temp;
	//             }
	//         }
	//     }

	// 	// Verification loop to ensure no empty slots
	//     int shiftIndex = 0;
	//     for (int i = 0; i < count; i++) {
	// 		Entity* en = &entities[i];
	//         if (en->arch == ARCH_nil) {
	//             shiftIndex++;
	//         } else if (shiftIndex > 0) {
	//             // Shift the entity left to fill the gap
	//             entities[i - shiftIndex] = entities[i];
	//         }
	//     }

	// }
// 


// :sort entities
void sort_entity_indices_by_prio_and_y(int* indices, Entity* entities, int count) {
	// sorts entities:
	// primary sort: sort entities based on their rendering_prio value
	// secondary sort: if rendering_prio is the same, sort based on their y coordinates in descending order

	// printf("--> SORTER ENTITIES <--\n");

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int prio1 = entities[indices[j]].rendering_prio;
            int prio2 = entities[indices[j + 1]].rendering_prio;
            int y1 = entities[indices[j]].pos.y;
            int y2 = entities[indices[j + 1]].pos.y;

            // Sort by rendering priority first (ascending order)
            // If the priorities are equal, sort by y value (descending order)
            if (prio1 > prio2 || (prio1 == prio2 && y1 < y2)) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
}

// ::render player
void render_player() {
	
	Entity* en = world->player->en;
	
	Vector2 pos = get_player_pos();
	Sprite* sprite = get_sprite(SPRITE_player);

	Matrix4 xform = m4_identity;
	xform = m4_translate(xform, v3(pos.x, pos.y, 0));
	xform = m4_translate(xform, v3(sprite->image->width * -0.5, sprite->image->height * -0.5, 0)); // center sprite

	// draw shadow
	if (en->enable_shadow){
		Vector2 shadow_pos = en->pos;
		shadow_pos.x = shadow_pos.x - (0.5 * get_sprite_size(sprite).x);
		shadow_pos.y = shadow_pos.y - (0.75 * get_sprite_size(sprite).y);
		draw_circle(shadow_pos, v2(get_sprite_size(sprite).x, 4.0), entity_shadow_color);
	}

	draw_image_xform(sprite->image, xform, get_sprite_size(sprite), COLOR_WHITE);

	// :Render held item
	if (item_in_hand != NULL && item_in_hand->valid){

		// TODO: here check if item has an animation
		if (item_in_hand->item_id == ITEM_TOOL_torch){
			// held_torch_animation->active = true;
			update_player_torch_animation(get_player_pos());

		}
		else{
			// held_torch_animation->active = false;
			stop_player_torch_animation();
			Sprite* sprite_held_item = get_sprite(item_in_hand->sprite_id);
			Matrix4 xform_held_item = m4_identity;
			xform_held_item = m4_translate(xform_held_item, v3(en->pos.x, en->pos.y, 0));
			xform_held_item = m4_translate(xform_held_item, v3(2, -3, 0));

			// held_torch_animation->active = false;
			// if (torch_animation->active) kill_animation_now(torch_animation);
			draw_image_xform(sprite_held_item->image, xform_held_item, v2(5, 5), COLOR_WHITE);
		}
	}
}

// ::Render entities || ::Entity render
void render_entities(World* world) {

	// NOTE: bugfix for (first item spawning is invisible) is to do "world->entity_count + 1". Don't know about the side effects of this fix tho
	// UPDATE: might be already fixed (because entity counts should work now correctly)
	int entity_count = world->dimension->entity_count + 1;

    // Create an array of indices
    int indices[entity_count];
    for (int i = 0; i < entity_count; i++) {
        indices[i] = i;
    }

    // Sort the indices based on the y coordinates AND the rendering prios of the entities 
	// NOTE: sorting the entities drops the fps like 2500 - 4000
	// ??: fix idea: cache the sorted entities and sort them again when changes occur. NOT EVERY DAMN FRAME
	// sort_entity_indices_by_prio_and_y(indices, world->entities, entity_count);
	// update_entity_render_order(&render_list, world->entities, render_list.count);


	// render_list.needs_sorting = true;

	// if (world->current_biome_id == BIOME_cave){
		// int asd = 1;
	// }

	// Entity* sorted_entities_by_culling[MAX_ENTITY_COUNT];
	// int temp_index = 0;

	// if (ENABLE_FRUSTRUM_CULLING){
	// 	// sort only entities within the render distance
	// 	for (int i = 0; i < entity_count; i++){
	// 		Entity* en = &world->dimension->entities[i];
	// 		if (en && range2f_contains(vector2_to_range(en->pos), get_player_pos())){
	// 			sorted_entities_by_culling[temp_index] = en;
	// 			temp_index++;
	// 		}
	// 	}
	// }
	// Create an array of indices
    // int indices2[temp_index];
    // for (int i = 0; i < temp_index; i++) {
    //     indices2[i] = i;
    // }

	// sort_entity_indices_by_prio_and_y(indices2, &sorted_entities_by_culling, temp_index);
	// int asdd = 0;

	// if (render_list.needs_sorting){
		// sort_entity_indices_by_prio_and_y(render_list.indices, world->entities, render_list.count);

		sort_entity_indices_by_prio_and_y(indices, world->dimension->entities, entity_count);

		// sort_entities_by_prio_and_y(&world->dimension->entities, entity_count);
	// 	render_list.needs_sorting = false;
	// }

   for (int i = 0; i < entity_count; i++)  {
		int index = indices[i];
		Entity* en = &world->dimension->entities[index];
		// Entity* en = &sorted_entities_by_culling[index];

		if (en->is_valid) {

			float entity_dist_from_player = fabsf(v2_dist(en->pos, get_player_pos()));

			switch (en->arch) {

				// :Render player
				case ARCH_player: {
					if (render_player_sprite)
					{
						render_player();

						// draw debug text
						if (IS_DEBUG){
							Matrix4 xform = m4_identity;
							xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
							xform = m4_translate(xform, v3(-3.5, -3, 0));
							draw_text_xform(font, STR("DEBUG"), font_height, xform, v2(0.1, 0.1), COLOR_RED);
							draw_text_xform(font, sprint(temp_allocator, STR("%.0f,%.0f"), get_player_pos().x, get_player_pos().y), font_height, m4_translate(xform, v3(0, -5, 0)), v2(0.1, 0.1), COLOR_WHITE);
						}
					}

				} break;

				case ARCH_portal:{
					{
						// // frustrum culling
						// if (entity_dist_from_player <= render_distance){
						// 	for (int i = 0; i < en->biome_count; i++) {
						// 		BiomeID portal_biome_id = en->biome_ids[i];
						// 		if (portal_biome_id == world->dimension->current_biome_id){
						// 			// printf("Drawing sprite at biome = %s\t", get_biome_data_from_id(portal_biome_id).name);
						// 			// printf("portal destination id: %d\n", en->portal_data.destination);
						// 			Sprite* sprite = get_sprite(en->sprite_id);
						// 			Matrix4 xform = m4_identity;

						// 			xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));

						// 			// center sprite
						// 			xform = m4_translate(xform, v3(sprite->image->width * -0.5, sprite->image->height * -0.5, 0));

						// 			// draw sprite
						// 			draw_image_xform(sprite->image, xform, get_sprite_size(sprite), COLOR_WHITE);

						// 		if (IS_DEBUG){
						// 			Matrix4 xform_debug = m4_identity;
						// 			xform_debug = m4_translate(xform_debug, v3(en->pos.x, en->pos.y, 0));
						// 			draw_rect_xform(xform_debug, v2(1,1), COLOR_RED);
						// 		}

						// 		}
						// 	}
						// }
						printf("RENDERING PORTAL IS COMMENTED\n");
					}
				} break;

				case ARCH_torch:{
					{
						Sprite* sprite = get_sprite(SPRITE_TOOL_torch);
						Texture* light = get_texture(TEXTURE_torch_light);
						Matrix4 xform = m4_identity;

						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
						xform = m4_translate(xform, v3(sprite->image->width * -0.5, 0.0, 0));

						Vector4 col = COLOR_WHITE;
						if (world_frame.selected_entity == en){
							col = v4(0.7, 0.7, 0.7, 1.0);
						}

						// draw sprite
						// draw_image_xform(sprite->image, xform, get_sprite_size(sprite), col);

						// torch animation on ground
						trigger_animation(ground_torch_animation, en->pos, 0);

						xform = m4_translate(xform, v3(light->image->width * -0.5, light->image->height * -0.5, 0));
						draw_image_xform(light->image, xform, get_texture_size(*light), COLOR_WHITE);

					}
				} break;

				case ARCH_parallax:{
					{
						Parallax* parallax = get_parallax_sprite(en->parallax_id);

						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));

						// get distance between parallax en and player (ABSOLUTE values)
						float dist = fabsf(en->pos.y) + fabsf(get_player_pos().y);

						// if parallax entity is above player (y-axis), then the opacity should always be "1"
						if (get_player_pos().y < en->pos.y) dist = -10;

						Vector4 col = COLOR_WHITE;

						// normalize parallax opacity between thresholds
						// col.a = normalizeWithThresholds(dist, parallax->threshold_min, parallax->threshold_max);

						// draw parallax image
						draw_image_xform(parallax->image, xform, v2(parallax->image->width, parallax->image->height), col);
					}
				} break;

				default: 
				{
					if (render_other_entities){
					// if (en->arch != ARCH_player){

						if (en->arch == ARCH_portal){
							assert(1==0, "forced crash @ 'render_entities'");
						}

						// frustrum culling
						if (entity_dist_from_player <= render_distance){

							Sprite* sprite = get_sprite(en->sprite_id);

							Matrix4 xform = m4_identity;
							
							// ITEM
							if (en->is_item) {
								// xform         = m4_translate(xform, v3(0, 2.0 * sin_breathe(os_get_current_time_in_seconds(), 5.0), 0)); // bob item up and down 		// deprecated
								xform         = m4_translate(xform, v3(0, 2.0 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0)); // bob item up and down
								
								// shadow position
								Vector2 position = en->pos;
								position.x = position.x - (0.5 * get_sprite_size(sprite).x);
								position.y = position.y - (0.5 * get_sprite_size(sprite).y) - 1;
								
								// item shadow
								draw_circle(position, v2(get_sprite_size(sprite).x, get_sprite_size(sprite).y * 0.5), item_shadow_color);

							}
							else{
								xform = m4_translate(xform, v3(0, tile_width * -0.5, 0));			// bring sprite down to bottom of Tile if not item

								if (en->enable_shadow){
									Vector2 shadow_pos = en->pos;
									shadow_pos.x = shadow_pos.x - (0.5 * get_sprite_size(sprite).x);
									// shadow_pos.y = shadow_pos.y - (0.75 * get_sprite_size(sprite).y);
									shadow_pos.y -= 6;
									draw_circle(shadow_pos, v2(get_sprite_size(sprite).x, 4.0), entity_shadow_color);
								}
							}
							
							if (en->workstation_id){
								if (runtime_debug){
									int asdasd = 1;
									printf("workstation\n");
								}
							}

							xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
							xform = m4_translate(xform, v3(sprite->image->width * -0.5, 0.0, 0));

							Vector4 col = COLOR_WHITE;
							if (world_frame.selected_entity == en){
								col = v4(0.7, 0.7, 0.7, 1.0);
							}

							// draw sprite
							draw_image_xform(sprite->image, xform, get_sprite_size(sprite), col);
							

							if (IS_DEBUG){
								// draw_text(font, sprint(temp_allocator, STR("%.0f, %.0f"), en->pos.x, en->pos.y), 40, en->pos, v2(0.1, 0.1), COLOR_WHITE);
								Matrix4 xform_debug = m4_identity;
								xform_debug = m4_translate(xform_debug, v3(en->pos.x, en->pos.y, 0));
								// xform_debug = m4_translate(xform_debug, v3(sprite->image->width * 0.5, sprite->image->height * 0.5, 0));

								// draw_rect_xform(xform, v2(1,1), COLOR_RED);
								draw_rect_xform(xform_debug, v2(1,1), COLOR_RED);
							}
						}
					// }
					}
				} break;
			}
		}

		// :render crafting progress bar || :crafting animation
		if (en->arch == ARCH_workstation && en->is_crafting_station && en->workstation_data.selected_crafting_item) {
			{

				ItemData item = *en->workstation_data.selected_crafting_item;
				Sprite icon = *get_sprite(get_sprite_from_itemID(item.item_id));
				Vector4 col = v4(1, 1, 1, 0.1);

				float alpha = alpha_from_end_time(en->workstation_data.crafting_end_time, item.cooking_time);
				col.a = alpha;

				Matrix4 xform_item = m4_identity;
				xform_item = m4_translate(xform_item, v3(en->pos.x, en->pos.y, 0));
				xform_item = m4_translate(xform_item, v3(icon.image->width * -0.5, 0, 0));
				draw_image_xform(icon.image, xform_item, v2(icon.image->width, icon.image->height), col);

			}


			// randy's solution
				// float radius = 4.0;
				// Vector4 outside_col = v4(1,1,1,1);
				// Vector4 inside_col = v4(0,0,0,1);
				// {
				// 	Matrix4 xform = m4_identity;
				// 	xform = m4_translate(xform, v3(en->pos.x, en->pos.y + 14.0, 0));
				// 	xform = m4_translate(xform, v3(-radius, -radius, 0));
				// 	draw_circle_xform(xform, v2(radius*2, radius*2), inside_col);
				// }

				// ItemData craft_item_data = *en->workstation_data.selected_crafting_item;
				// float alpha = alpha_from_end_time(en->workstation_data.crafting_end_time, craft_item_data.cooking_time);

				// {
				// 	Matrix4 xform = m4_identity;
				// 	xform = m4_translate(xform, v3(en->pos.x, en->pos.y + 14.0, 0));
				// 	xform = m4_scale(xform, v3(alpha, alpha, 1.0));
				// 	xform = m4_translate(xform, v3(-radius, -radius, 0));
				// 	draw_circle_xform(xform, v2(radius*2, radius*2), outside_col);
			// }
		}
	}
}

// :render keybinding
void render_keybinding(Entity* en, char keybind) {
	Matrix4 xform = m4_identity;
	xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
	xform = m4_translate(xform, v3(0, 10, 0));

	draw_text_xform(font, sprint(temp_allocator, STR("%c"), keybind), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);
}






// ----- SETUP -----------------------------------------------------------------------------------------|
// ::Entry
int entry(int argc, char **argv) 
{
	// Window
	window.title = STR("Game.");
	window.point_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.point_height = 720;

	// window spawn position
	window.x = window.point_width * 0.5 + 150;				// window.x = 200; // default value // +150 so i can see console
	window.y = window.point_height * 0.5 - 100;				// window.y = 200; // default value	// -100

	// bg color
	// window.clear_color = hex_to_rgba(0x43693aff);
	window.clear_color = v4(0.3,0.3,0.3,1);
	window.force_topmost = false;
	window.allow_resize = false;

	window.enable_vsync = true;

	// Memory
	world = alloc(get_heap_allocator(), sizeof(World));
	memset(world, 0, sizeof(World));

	// Audio
	Audio_Player *audio_player = audio_player_get_one();

	// Font
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	render_atlas_if_not_yet_rendered(font, font_height, 'A'); // fix for the stuttering bug for first time text rendering courtesy of charlie (Q&A #3)

	// ::LOAD RESOURCES --------------------------------->

		// ::Load sprites
			// :Load entity sprites
			sprites[0] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/missing_texture.png"), get_heap_allocator())};
			sprites[SPRITE_TEST] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/test.png"), get_heap_allocator())};
			// sprites[SPRITE_player] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/player.png"), get_heap_allocator())};
			sprites[SPRITE_player] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/player_new.png"), get_heap_allocator())};

			// trees
			sprites[SPRITE_tree_pine] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_pine.png"), get_heap_allocator())};
			sprites[SPRITE_tree_spruce] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_birch.png"), get_heap_allocator())};
			sprites[SPRITE_tree_birch] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_spruce.png"), get_heap_allocator())};
			sprites[SPRITE_tree_magical0] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_magical0.png"), get_heap_allocator())};
			sprites[SPRITE_tree_magical1] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_magical1.png"), get_heap_allocator())};

			// rocks
			sprites[SPRITE_rock_normal_small] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_normal_small.png"), get_heap_allocator())};
			sprites[SPRITE_rock_normal_medium] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_normal_medium.png"), get_heap_allocator())};
			sprites[SPRITE_rock_normal_large] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_normal_large.png"), get_heap_allocator())};
			sprites[SPRITE_rock_mossy_small] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_mossy_small.png"), get_heap_allocator())};
			sprites[SPRITE_rock_mossy_medium] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_mossy_medium.png"), get_heap_allocator())};
			sprites[SPRITE_rock_mossy_large] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_mossy_large.png"), get_heap_allocator())};
			sprites[SPRITE_rock_sandy_small] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/rock_sandy_small.png"), get_heap_allocator())};
			sprites[SPRITE_rock_sandy_medium] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/missing_texture.png"), get_heap_allocator())};
			sprites[SPRITE_rock_sandy_large] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/missing_texture.png"), get_heap_allocator())};

			// foliage
			sprites[SPRITE_bush_small] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/bush_small.png"), get_heap_allocator())};
			sprites[SPRITE_bush_medium] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/bush_medium.png"), get_heap_allocator())};
			sprites[SPRITE_bush_berry] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/bush_berry.png"), get_heap_allocator())};
			sprites[SPRITE_short_grass] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/short_grass.png"), get_heap_allocator())};
			sprites[SPRITE_tall_grass] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tall_grass.png"), get_heap_allocator())};
			sprites[SPRITE_mushroom0] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/mushroom0.png"), get_heap_allocator())};

			// portals
			sprites[SPRITE_portal0] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/portal0.png"), get_heap_allocator())};
			sprites[SPRITE_portal1] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/portal1.png"), get_heap_allocator())};

			// ores
			sprites[SPRITE_ORE_iron] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ore_iron.png"), get_heap_allocator())};
			sprites[SPRITE_ORE_gold] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ore_gold.png"), get_heap_allocator())};
			sprites[SPRITE_ORE_copper] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ore_copper.png"), get_heap_allocator())};


			// :Load item sprites
			sprites[SPRITE_item_rock] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_rock.png"), get_heap_allocator())};
			sprites[SPRITE_item_pine_wood] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_pine_wood.png"), get_heap_allocator())};
			sprites[SPRITE_item_sprout] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_sprout.png"), get_heap_allocator())};
			sprites[SPRITE_item_berry] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_berry.png"), get_heap_allocator())};
			sprites[SPRITE_item_twig] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_twig.png"), get_heap_allocator())};
			sprites[SPRITE_item_fossil0] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_fossil0.png"), get_heap_allocator())};
			sprites[SPRITE_item_fossil1] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_fossil1.png"), get_heap_allocator())};
			sprites[SPRITE_item_fossil2] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_fossil2.png"), get_heap_allocator())};
			sprites[SPRITE_INGOT_iron] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ingot_iron.png"), get_heap_allocator())};
			sprites[SPRITE_INGOT_gold] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ingot_gold.png"), get_heap_allocator())};
			sprites[SPRITE_INGOT_copper] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/ingot_copper.png"), get_heap_allocator())};
			sprites[SPRITE_ITEM_ore_iron] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_ore_iron.png"), get_heap_allocator())};
			sprites[SPRITE_ITEM_ore_gold] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_ore_gold.png"), get_heap_allocator())};
			sprites[SPRITE_ITEM_ore_copper] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/item_ore_copper.png"), get_heap_allocator())};
			sprites[SPRITE_tree_sap] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tree_sap.png"), get_heap_allocator())};

			// :Load tool sprites
			sprites[SPRITE_TOOL_pickaxe] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tool_pickaxe.png"), get_heap_allocator())};
			sprites[SPRITE_TOOL_axe] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tool_axe.png"), get_heap_allocator())};
			sprites[SPRITE_TOOL_shovel] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/tool_shovel.png"), get_heap_allocator())};
			sprites[SPRITE_TOOL_torch] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/torch.png"), get_heap_allocator())};

			// :Load workstation sprites
			sprites[SPRITE_WORKSTATION_furnace] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/workstation_furnace.png"), get_heap_allocator())};
			sprites[SPRITE_WORKSTATION_workbench] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/workstation_workbench.png"), get_heap_allocator())};
			sprites[SPRITE_WORKSTATION_chest] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/workstation_chest.png"), get_heap_allocator())};

			// Load category sprites
			sprites[SPRITE_CATEGORY_all] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/category_all.png"), get_heap_allocator())};
			sprites[SPRITE_CATEGORY_items] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/category_item.png"), get_heap_allocator())};
			sprites[SPRITE_CATEGORY_tools] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/category_tool.png"), get_heap_allocator())};
			sprites[SPRITE_CATEGORY_workstations] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/category_workstation.png"), get_heap_allocator())};
		// 

		// :Load parallax
			parallaxes[PARALLAX_tree0] = (Parallax){ .image=load_image_from_disk(STR("res/sprites/tree_parallax.png"), get_heap_allocator()), .threshold_min=60, .threshold_max=230};
			parallaxes[PARALLAX_tree1] = (Parallax){ .image=load_image_from_disk(STR("res/sprites/tree_parallax2.png"), get_heap_allocator()), .threshold_min=200, .threshold_max=320};
		// 

		// :Load textures
			textures[0] = (Texture){ .image=load_image_from_disk(STR("res/textures/texture_nil.png"), get_heap_allocator())};
			textures[TEXTURE_vignette_no_torch] = (Texture){ .image=load_image_from_disk(STR("res/textures/vignette_no_torch.png"), get_heap_allocator())};
			textures[TEXTURE_vignette_torch] = (Texture){ .image=load_image_from_disk(STR("res/textures/vignette_torch.png"), get_heap_allocator())};
			textures[TEXTURE_torch_light] = (Texture){ .image=load_image_from_disk(STR("res/textures/torch_light.png"), get_heap_allocator())};
			
			textures[TEXTURE_TILE_grass] = (Texture){ .image=load_image_from_disk(STR("res/textures/tileset/grass.png"), get_heap_allocator())};
			textures[TEXTURE_TILE_grass2] = (Texture){ .image=load_image_from_disk(STR("res/textures/tileset/grass4.png"), get_heap_allocator())};
			textures[TEXTURE_TILE_moss] = (Texture){ .image=load_image_from_disk(STR("res/textures/tileset/moss2.png"), get_heap_allocator())};
			textures[TEXTURE_TILE_sand] = (Texture){ .image=load_image_from_disk(STR("res/textures/tileset/sand.png"), get_heap_allocator())};
		// 

		// :Load buildings
			// buildings[BUILDING_house] = (BuildingData){ .image=load_image_from_disk(STR("res/sprites/house1.png"), get_heap_allocator())};
			sprites[SPRITE_BUILDING_house] = (Sprite){ .image=load_image_from_disk(STR("res/sprites/house2.png"), get_heap_allocator())};


		// 

		// :Load audio
			Audio_Source hit_metal1, hit_metal2, rock_breaking1, swing_slow, swing_fast;
			audioFiles[AUDIO_hit_metal1] = (Audio){.name=STR("Hit metal 1"),.ok=audio_open_source_load(&hit_metal1, STR("res/sounds/hit_metal1.wav"), get_heap_allocator()),.path=STR("res/sounds/hit_metal1.wav"),.source=hit_metal1};
			audioFiles[AUDIO_hit_metal2] = (Audio){.name=STR("Hit metal 2"),.ok=audio_open_source_load(&hit_metal2, STR("res/sounds/hit_metal2.wav"), get_heap_allocator()),.path=STR("res/sounds/hit_metal2.wav"),.source=hit_metal2};
			audioFiles[AUDIO_rock_breaking1] = (Audio){.name=STR("Rock Breaking 1"),.ok=audio_open_source_load(&rock_breaking1, STR("res/sounds/rock_breaking1.wav"), get_heap_allocator()),.path=STR("res/sounds/rock_breaking1.wav"),.source=rock_breaking1};
			audioFiles[AUDIO_swing_slow] = (Audio){.name=STR("Swing slow"),.ok=audio_open_source_load(&swing_slow, STR("res/sounds/swing_slow.wav"), get_heap_allocator()),.path=STR("res/sounds/swing_slow.wav"),.source=swing_slow};
			audioFiles[AUDIO_swing_fast] = (Audio){.name=STR("Swing fast"),.ok=audio_open_source_load(&swing_fast, STR("res/sounds/swing_fast.wav"), get_heap_allocator()),.path=STR("res/sounds/swing_fast.wav"),.source=swing_fast};
		// 

		// assert all sprites			@ship debug this off (by: randy)
		{
			// crash when image fails to be setup properly (by: randy)
			for (SpriteID i = 0; i < SPRITE_MAX; i++) {
				Sprite* sprite = &sprites[i];
				if (!sprite->image){
					log_error("\n\nSPRITE '%d' was not setup correctly!", i);
					printf("Might be because SpriteID for the sprite exists, but the sprite doesn't exist in 'sprites[]' or the image.png is missing!");
					assert(sprite->image);
				}
				// assert(sprite->image, STR("%d Sprite was not setup correctly", i));
			}
		}
		
		// assert all texture files		@ship debug this off (by: jani)
		{
			for (TextureID i = 1; i < TEXTURE_MAX; i++) {
				Texture* texture = &textures[i];
				if (!texture->image){
					log_error("\n\nTEXTURE '%d' was not setup correctly!", i);
					printf("Might be because TextureID for the texture exists, but the texture doesn't exist in 'textures[]' or the image.png is missing!");
					assert(texture->image);
				}
				// assert(texture->image, "Texture was not setup correctly");
			}
		}
		
		// assert all audio files		@ship debug this off (by: jani)
		{
			for (AudioID i = 1; i < AUDIO_MAX; i++) {
				Audio* audio = &audioFiles[i];
				if (!audio->ok){
					log_error("\n\nAUDIO '%d' was not setup correctly!", i);
					printf("Might be because AudioID for the audio exists, but the audio doesn't exist in 'audioFiles[]' or the audio.wav is missing!");
					assert(audio->ok);
				}
				else{
					log_verbose("Audio file set up '%s'", audio->name);
				}
				// assert(audio->ok, "Audio was not setup correctly: '%s'", audio->name);
			}
		}
	// 

	// ::INIT

	// init_WorldData(&map);
	init_worldmap();

	setup_world();

	// setup dimensions
	setup_all_dimensions();
	world->dimension = get_dimensionData(DIM_overworld);
	// world->dimension->id = DIM_overworld;
	world->dimension->entity_count = 0;
	// world->dimension->current_biome_id = biome_at_tile(v2_world_pos_to_tile_pos(v2(0,0))); // do get_player_pos() here instead of 0,0 vector


	setup_player();


	#if defined(DEV_TESTING)
	// test adding workstations to world
	{
		// House:
		{
			load_chunk_with_offset(world->dimension, v2(0,0));
			Entity* en = entity_create_to_chunk(world->dimension->chunks[256][256]);
			setup_building(en, BUILDING_house);
			en->pos = v2(255, 255);
		}
		// FURNACE:
		{
			// Entity* en = entity_create();
			// load_chunk(world->dimension, v2(0, 0));
			// Entity* en = entity_create_to_chunk(world->dimension->chunks[256][256]);
			// setup_building(en, BUILDING_furnace);
			// en->pos = v2(-15, 0);
			// en->pos = round_v2_to_tile(en->pos);
		}


		// CHEST:
		{
			// Entity* en = entity_create();
			// setup_building(en, BUILDING_chest);
			// en->pos = v2(-25, -10);
			// en->pos = round_v2_to_tile(en->pos);
		}

		// workbench
		{
			// Entity* en = entity_create();
			// setup_building(en, BUILDING_workbench);
			// en->pos = v2(-28, -20);
			// en->pos = round_v2_to_tile(en->pos);
		}

		// parallax test
		{
			// Entity* en = entity_create();
			// setup_parallax(en);
			// en->pos = v2(0, 0);
		}
	}
	#endif

	// Workstation resource setup
	setup_all_workstation_resources();

	setup_all_building_resources();


	// crafting & smelting recipes setup
	setup_all_recipes();
	setup_all_tools();

	// setup audio
	// setup_audio_player();
	// Audio_Playback_Config audio_config = {0};
	// Audio_Playback_Config audio_config = {};
	// audio_config.volume                = 0.1;
	// audio_config.playback_speed        = 1.0;
	// audio_config.enable_spacialization = true;
	// audio_config.position_ndc          = v3(0, 0, 0);

	Audio_Playback_Config audio_config = {
		.enable_spacialization = true,
		.playback_speed = 1.0,
		.volume = 1.0,
		.position_ndc = v3(0, 0, 0)
	};
	
	Audio_Playback_Config audio_config_player = {
		.enable_spacialization = true,
		.playback_speed = 1.0,
		.volume = 1.0,
		.position_ndc = v3(0, 0, 0)
	};

	// spawning
	// BiomeData temp_data = get_biome_data_from_id(world->dimension->current_biome_id);
	// spawn_biome(&temp_data);
	// memset(&temp_data, 0, sizeof(temp_data)); // i dont know what im doing

	// setup loot-tables
	setup_all_loot_tables();

	// Timing
	float64 seconds_counter = 0.0;
	s32 frame_count = 0;
	// float64 last_time = os_get_current_time_in_seconds(); // deprecated
	float64 last_time = os_get_elapsed_seconds();

	// view_zoom += 0.2;		// zoom out a bit


	held_torch_animation = setup_held_torch_animation();
	ground_torch_animation = setup_ground_torch_animation();
	crafting_animation = setup_crafting_animation();
	// smelting_animation = setup_smelting_animation();


	// world->ux_state = UX_mainmenu;
	Gfx_Image* mainmenu_bg = load_image_from_disk(STR("res/title_screen.png"), get_heap_allocator());


// ----- MAIN LOOP ----------------------------------------------------------------------------------------- 
	while (!window.should_close) {

		reset_temporary_storage();

		// :Timing
		// float64 current_time = os_get_current_time_in_seconds(); // deprecated
		float64 current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;
		last_time = current_time;
		os_update();

		// world_frame.hover_consumed = false;

		if (world->ux_state == UX_mainmenu){
			{
			
			Vector2 size = v2(mainmenu_bg->width, mainmenu_bg->height);
			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, v3(size.x * -0.5, size.y * -0.5, 0));
			draw_image_xform(mainmenu_bg, xform, v2(mainmenu_bg->width, mainmenu_bg->height), COLOR_WHITE);

			Matrix4 xform_play = m4_identity;
			// xform_play = m4_translate(xform_play, v3((size.x * -0.5) + 658, (size.y * -0.5) + 403, 0));
			xform_play = m4_translate(xform_play, v3((size.x * -0.5), (size.y * -0.5), 0));
			xform_play = m4_translate(xform_play, v3(658, size.y - 506, 0));

			Vector4 col = v4(1, 0, 0, 0.0);

			Draw_Quad* quad_play = draw_rect_xform(xform_play, v2(392, 103), col);

			if (range2f_contains(quad_to_range(*quad_play), get_mouse_pos_in_ndc())){
				if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
					
					if (os_is_file_s(STR("world"))) {
						bool result = world_attempt_load_from_disk();
						if (!result) {
							// just setup a new world if it fails. temp
							setup_world();
						}
						setup_world();
					}
					else {
						setup_world();
					}

					world_save_to_disk();
				}
			}

			// gfx_update();

			
			}
		}


	else{


		// reset world_frame
		world_frame = (WorldFrame){0};


		// player
		// world->player->en = get_player_en_from_current_dim();
		// sync_player_pos_between_dims();	// NOTE: this has an impact of only about 1fps		// also could just sync the pos only when player moves!? 
		// update_biome();


		// :Frame :update
		draw_frame.enable_z_sorting = true;
		world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		

		// :camera
		if (!animation_dim_change.active)
		{
			Vector2 target_pos = world->player->en->pos;
			animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

			world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));
		}
		else {
			update_dim_change_animation(delta_t);
		}

		set_world_space();
		push_z_layer(layer_world);

		Vector2 mouse_pos_world = get_mouse_pos_in_world_space();
		// int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
		// int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);


		chunk_manager();

		// Chunk* player_chunk = get_player_chunk();
		player_chunk = get_player_chunk();

		// if (IS_DEBUG) draw_rect(player_chunk->pos_in_world, v2(CHUNK_SIZE, CHUNK_SIZE), v4(0, 0, 1, 0.5));

		// if (IS_DEBUG){
		// 	if (player_chunk->has_been_loaded){
		// 		draw_rect(player_chunk->pos_in_world, v2(CHUNK_SIZE, CHUNK_SIZE), v4(0, 1, 0, 0.5));
		// 	}
		// 	else{
		// 		draw_rect(player_chunk->pos_in_world, v2(CHUNK_SIZE, CHUNK_SIZE), v4(1, 0, 0, 0.5));
		// 	}
		// }


		if (world->ux_state != UX_nil) world_frame.hover_consumed = true;
		else world_frame.hover_consumed = false;


		// Render ui
		// should prolly move this way down
		render_ui();

		// :Entity selection by MOUSE
		// if (!world_frame.hover_consumed)
		// {
		// 	// log("%f, %f", input_frame.mouse_x, input_frame.mouse_y);
		// 	// draw_text(font, sprint(temp, STR("%.0f, %.0f"), pos.x, pos.y), font_height, pos, v2(0.1, 0.1), COLOR_RED);

		// 	if (world->ux_state == UX_nil){

		// 		float smallest_dist = 9999999;

		// 		// for (int i = 0; i < MAX_ENTITY_COUNT; i++){		// NOTE: actually faster to use MAX_ENTITY_COUNT here
		// 		for (int i = 0; i < player_chunk->entity_count; i++){
		// 			// Entity* en = &world->dimension->entities[i];
		// 			Entity* en = &player_chunk->entities[i];

		// 			if (!en){
		// 				int asd = 0;
		// 				continue;
		// 			}

		// 			if (IS_DEBUG){
		// 				draw_rect(en->pos, get_sprite_size(get_sprite(en->sprite_id)), v4(1,1,0,0.5));
		// 			}

		// 			// if (IS_DEBUG){
		// 			// 	// world_frame.selected_entity = en;
		// 			// 	if (en->is_valid){
		// 			// 		float dist = fabsf(v2_dist(en->pos, mouse_pos_world));
		// 			// 		if (dist < world->player->entity_selection_radius){
		// 			// 			if (!world_frame.selected_entity || (dist < smallest_dist)){
		// 			// 				if (en->arch == ARCH_item){
		// 			// 					printf("EN = %s\t%.1f, %.1f\n", en->name, en->pos.x, en->pos.y);
		// 			// 				}
		// 			// 				else if (en->arch == ARCH_portal){
		// 			// 					printf("selected portal = %s\t%.1f, %.1f\n", en->name, en->pos.x, en->pos.y);
		// 			// 				}
		// 			// 			}
		// 			// 		}
		// 			// 	}
		// 			// }

		// 			if (en->is_valid && en->destroyable && en->arch != ARCH_portal){
		// 				Sprite* sprite = get_sprite(en->sprite_id);

		// 				// int entity_tile_x = world_pos_to_tile_pos(en->pos.x);
		// 				// int entity_tile_y = world_pos_to_tile_pos(en->pos.y);

		// 				Vector2 sprite_bottom_center = en->pos;
		// 				sprite_bottom_center.x += get_sprite_size(sprite).x * 0.5;

		// 				// float dist = fabsf(v2_dist(en->pos, mouse_pos_world));
		// 				float dist = fabsf(v2_dist(sprite_bottom_center, mouse_pos_world));

		// 				if (is_key_just_pressed('K')){
		// 					int asd = 0;
		// 				} 

		// 				// :select entity
		// 				if (dist < world->player->entity_selection_radius) {
		// 					if (!world_frame.selected_entity || (dist < smallest_dist)) {
		// 						// this is selected entity by mouse. RENAME
		// 						world_frame.selected_entity = en;

		// 						Vector2 size = get_sprite_size(get_sprite(en->sprite_id));
		// 						draw_rect(en->pos, v2(size.x, size.y), v4(1,0,0,0.5));

		// 						printf("selected entity by mouse\n");

		// 						// if (en->is_crafting_station){
		// 						// 	world->open_crafting_station = en;
		// 						// }

		// 						if (en->arch == ARCH_workstation){
		// 							// printf("UPDATED 'world->player->selected_workstation' to %d\n", en->arch);
		// 							world->player->selected_workstation = &en->workstation_data;
		// 							world->player->selected_workstation->en = en;
		// 						}
		// 						// smallest_dist = dist; // imo entity selection works better with this line commented
		// 					}
		// 				}
		// 			}
		// 		}
		// 	}
		// }


		// entity selection by mouse NEW
		if (!world_frame.hover_consumed)
		{
			for (int i = 0; i < player_chunk->entity_count; i++){
				Entity* en = &player_chunk->entities[i];
				Sprite* sprite = get_sprite(en->sprite_id);

				Range2f en_range = range2f_make(en->pos, v2(en->pos.x + sprite->image->width, en->pos.y + sprite->image->height));

				if (!world_frame.selected_entity && range2f_contains(en_range, get_mouse_pos_in_world_space()) && !en->unselectable){

					if (IS_DEBUG) draw_rect(en->pos, get_sprite_size(sprite), COLOR_RED);

					if (en->arch == ARCH_workstation){
						world->player->selected_workstation = &en->workstation_data;
						world->player->selected_workstation->en = en;
						// printf("SELECTTED workstation '%s'\n", en->name);
					}

					world_frame.selected_entity = en;
				}



			}
		}


		// :Entity selection by player position
		// @pin6
		if (enable_entity_selection_by_player_position)
		{
			float smallest_dist = 9999999;
			float entity_selection_radius = 10.0f;

			// for (int i = 0; i < world->dimension->entity_count; i++){
			for (int i = 0; i < player_chunk->entity_count; i++){

				// Entity* en = &world->dimension->entities[i];
				Entity* en = &player_chunk->entities[i];
				
				if (en->is_valid){
					if (en->arch == ARCH_portal || en->arch == ARCH_workstation){ // #portal or #workstation

						float dist = fabsf(v2_dist(en->pos, get_player_pos()));

						if (dist < entity_selection_radius){

							if (!world_frame.selected_entity || (dist < smallest_dist)){
								// printf("PORTAL SELECTED\n");
								// printf("%.1f, %.1f\n", en->pos.x, en->pos.y);
								world_frame.selected_entity = en;
							}
						}
						// else{
						// 	if (en->is_crafting_station && world->open_crafting_station == en){
						// 		world->open_crafting_station = NULL;
						// 	}
						// }
					}
				}
			}
		}

		// // :Render grid (:Grid)
			// draw_grid = false;
			// if (draw_grid)
			// {	
			// 	// NOTE: rendering tiles has a big fkin impact on fps 
			// 	int player_tile_x = world_pos_to_tile_pos(world->player->en->pos.x);
			// 	int player_tile_y = world_pos_to_tile_pos(world->player->en->pos.y);
			// 	int tile_radius_x = 18;
			// 	int tile_radius_y = 12;

			// 	for (int x = player_tile_x - tile_radius_x; x < player_tile_x + tile_radius_x; x++){
			// 		for (int y = player_tile_y - tile_radius_y; y < player_tile_y + tile_radius_y; y++){
							
			// 				// BiomeID id = biome_at_tile(v2_world_pos_to_tile_pos(get_player_pos()));

			// 				float x_pos = x * tile_width;
			// 				float y_pos = y * tile_width;

			// 				BiomeID id = biome_at_tile(v2_world_pos_to_tile_pos(v2(x_pos, y_pos)));
			// 				Vector4 col = get_biome_data_from_id(id).grass_color;
							
			// 				// Draw Tile Grid
			// 				draw_rect(v2(x_pos + tile_width * -0.5, y_pos + tile_width * -0.5), v2(tile_width, tile_width), col);
			// 			// }
			// 		}
			// 	}
			// 	// Draw different tile on mouse pos
			// 	// draw_rect(v2(tile_pos_to_world_pos(mouse_tile_x) + tile_width * -0.5, tile_pos_to_world_pos(mouse_tile_y) + tile_width * -0.5), v2(tile_width, tile_width), v4(0.5, 0.5, 0.5, 0.5)); //hex_to_rgba(0x406438ff)
			// }


			// // :Draw ground texture || :draw_ground 
			// render_ground_texture = false;
			// if (render_ground_texture)
			// {

			// 	// this shit way too complex
			// 	// disgusting

			// 	// int player_tile_x = world_pos_to_tile_pos(player_en->pos.x);
			// 	// int player_tile_y = world_pos_to_tile_pos(player_en->pos.y);
			// 	float player_pos_x = world->player->en->pos.x;
			// 	float player_pos_y = world->player->en->pos.y;

			// 	if (world->dimension->current_biome_id == BIOME_cave){
			// 		int asd = 1;
			// 	}
			// 	// Texture* texture = get_texture(get_biome_data_from_id(world->current_biome_id).ground_texture);
			// 	Texture* texture = get_texture(get_biome_data_from_id(world->dimension->current_biome_id).ground_texture);
			// 	// Texture* texture = get_texture(get_biome_data_from_id(biome_at_tile(v2_world_pos_to_tile_pos(v2(player_pos_x, player_pos_y)))).ground_texture);
			// 	Vector4 color = COLOR_WHITE;

			// 	// add color adjustment to texture
			// 	// Vector4 col_adjustment = get_biome_data_from_id(world->current_biome_id).grass_color;
			// 	if (get_biome_data_from_id(world->dimension->current_biome_id).grass_color.a != 0){
			// 		color = get_biome_data_from_id(world->dimension->current_biome_id).grass_color;
			// 	}

			// 	//  ______________
			// 	// | Xform LOGIC:
			// 	// | - A B C
			// 	// | - D M E
			// 	// | - F G H
			// 	// |______________

			// 	Matrix4 xform_M = m4_identity;
			// 	Matrix4 xform_A = m4_identity;
			// 	Matrix4 xform_B = m4_identity;
			// 	Matrix4 xform_C = m4_identity;
			// 	Matrix4 xform_D = m4_identity;
			// 	Matrix4 xform_E = m4_identity;
			// 	Matrix4 xform_F = m4_identity;
			// 	Matrix4 xform_G = m4_identity;
			// 	Matrix4 xform_H = m4_identity;

			// 	// attempt #4
			// 	// Vector2 texture_size = get_texture_size(texture); // old
			// 	Vector2 texture_size = get_texture_size(*texture); // new

			// 	float traveled_step_x = (int)((player_pos_x / (texture_size.x * 2)) * 2) * texture_size.x;
			// 	float traveled_step_y = (int)((player_pos_y / (texture_size.y * 2)) * 2) * texture_size.y;
			// 	bool x_axis_fix = false;
				
			// 	// x-axis fix
			// 	if (player_pos_y < 0 && traveled_step_y == 0 && traveled_step_x != 0){
			// 		x_axis_fix = true;
			// 	}

			// 	if (traveled_step_x < 0){ 			// left
			// 		// printf("LEFT ");
			// 		if (traveled_step_y < 0){
			// 			// printf("DOWN \n");
			// 			xform_M = m4_translate(xform_M, v3(traveled_step_x - texture_size.x		,	traveled_step_y - texture_size.y			, 0));
			// 			xform_A = m4_translate(xform_A, v3(traveled_step_x - texture_size.x * 2	,	traveled_step_y 							, 0));
			// 			xform_B = m4_translate(xform_B, v3(traveled_step_x - texture_size.x		,	traveled_step_y 							, 0));
			// 			xform_C = m4_translate(xform_C, v3(traveled_step_x						,	traveled_step_y 							, 0));
			// 			xform_D = m4_translate(xform_D, v3(traveled_step_x - texture_size.x * 2	,	traveled_step_y - texture_size.y			, 0));
			// 			xform_E = m4_translate(xform_E, v3(traveled_step_x						,	traveled_step_y - texture_size.y			, 0));
			// 			xform_F = m4_translate(xform_F, v3(traveled_step_x - texture_size.x * 2	,	traveled_step_y - (texture_size.y * 2)		, 0));
			// 			xform_G = m4_translate(xform_G, v3(traveled_step_x - texture_size.x		,	traveled_step_y - (texture_size.y * 2)		, 0));
			// 			xform_H = m4_translate(xform_H, v3(traveled_step_x						,	traveled_step_y - (texture_size.y * 2)		, 0));
			// 		}
			// 		else{
			// 			// printf("UP \n");
			// 			xform_M = m4_translate(xform_M, v3(traveled_step_x - texture_size.x		, 	traveled_step_y								, 0));
			// 			xform_A = m4_translate(xform_A, v3(traveled_step_x - texture_size.x * 2	, 	traveled_step_y + texture_size.y			, 0));
			// 			xform_B = m4_translate(xform_B, v3(traveled_step_x - texture_size.x		, 	traveled_step_y + texture_size.y			, 0));
			// 			xform_C = m4_translate(xform_C, v3(traveled_step_x						, 	traveled_step_y + texture_size.y			, 0));
			// 			xform_D = m4_translate(xform_D, v3(traveled_step_x - texture_size.x * 2	, 	traveled_step_y								, 0));
			// 			xform_E = m4_translate(xform_E, v3(traveled_step_x						, 	traveled_step_y								, 0));
			// 			xform_F = m4_translate(xform_F, v3(traveled_step_x - texture_size.x * 2	, 	traveled_step_y - texture_size.y			, 0));
			// 			xform_G = m4_translate(xform_G, v3(traveled_step_x - texture_size.x		, 	traveled_step_y - texture_size.y			, 0));
			// 			xform_H = m4_translate(xform_H, v3(traveled_step_x						, 	traveled_step_y - texture_size.y			, 0));
			// 		}
			// 	}
				
			// 	else if (traveled_step_x == 0){ 	// middle x = 0
			// 		// printf("MIDDLE\n");
			// 		if (player_pos_x < 0.0f){
			// 			if (player_pos_y < 0.0f){
			// 				xform_M = m4_translate(xform_M, v3(-texture_size.x							, traveled_step_y - texture_size.y			, 0));
			// 				xform_A = m4_translate(xform_A, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y							, 0));
			// 				xform_B = m4_translate(xform_B, v3(-traveled_step_x - texture_size.x		, traveled_step_y							, 0));
			// 				xform_C = m4_translate(xform_C, v3(-traveled_step_x							, traveled_step_y							, 0));
			// 				xform_D = m4_translate(xform_D, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y - texture_size.y			, 0));
			// 				xform_E = m4_translate(xform_E, v3(-traveled_step_x							, traveled_step_y - texture_size.y			, 0));
			// 				xform_F = m4_translate(xform_F, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y - texture_size.y * 2		, 0));
			// 				xform_G = m4_translate(xform_G, v3(-traveled_step_x - texture_size.x		, traveled_step_y - texture_size.y * 2		, 0));
			// 				xform_H = m4_translate(xform_H, v3(-traveled_step_x							, traveled_step_y - texture_size.y * 2		, 0));

			// 			}
			// 			else{
			// 				xform_M = m4_translate(xform_M, v3(-texture_size.x							, traveled_step_y							, 0));
			// 				xform_A = m4_translate(xform_A, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y + texture_size.y			, 0));
			// 				xform_B = m4_translate(xform_B, v3(-traveled_step_x - texture_size.x		, traveled_step_y + texture_size.y			, 0));
			// 				xform_C = m4_translate(xform_C, v3(-traveled_step_x							, traveled_step_y + texture_size.y			, 0));
			// 				xform_D = m4_translate(xform_D, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y							, 0));
			// 				xform_E = m4_translate(xform_E, v3(-traveled_step_x							, traveled_step_y							, 0));
			// 				xform_F = m4_translate(xform_F, v3(-traveled_step_x - texture_size.x * 2	, traveled_step_y - texture_size.y			, 0));
			// 				xform_G = m4_translate(xform_G, v3(-traveled_step_x - texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 				xform_H = m4_translate(xform_H, v3(-traveled_step_x							, traveled_step_y - texture_size.y			, 0));

			// 			}
			// 		}
			// 		else{
			// 			if (player_pos_y < 0.0f){
			// 				xform_M = m4_translate(xform_M, v3(traveled_step_x							, traveled_step_y - texture_size.y			, 0));
			// 				xform_A = m4_translate(xform_A, v3(-traveled_step_x - texture_size.x		, traveled_step_y							, 0));
			// 				xform_B = m4_translate(xform_B, v3(-traveled_step_x							, traveled_step_y							, 0));
			// 				xform_C = m4_translate(xform_C, v3(-traveled_step_x + texture_size.x		, traveled_step_y							, 0));
			// 				xform_D = m4_translate(xform_D, v3(-traveled_step_x - texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 				xform_E = m4_translate(xform_E, v3(-traveled_step_x + texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 				xform_F = m4_translate(xform_F, v3(-traveled_step_x - texture_size.x		, traveled_step_y - texture_size.y * 2		, 0));
			// 				xform_G = m4_translate(xform_G, v3(-traveled_step_x							, traveled_step_y - texture_size.y * 2		, 0));
			// 				xform_H = m4_translate(xform_H, v3(-traveled_step_x + texture_size.x		, traveled_step_y - texture_size.y * 2		, 0));

			// 			}
			// 			else{
			// 				xform_M = m4_translate(xform_M, v3(traveled_step_x							, traveled_step_y							, 0));
			// 				xform_A = m4_translate(xform_A, v3(-traveled_step_x - texture_size.x		, traveled_step_y + texture_size.y			, 0));
			// 				xform_B = m4_translate(xform_B, v3(-traveled_step_x							, traveled_step_y + texture_size.y			, 0));
			// 				xform_C = m4_translate(xform_C, v3(-traveled_step_x + texture_size.x		, traveled_step_y + texture_size.y			, 0));
			// 				xform_D = m4_translate(xform_D, v3(-traveled_step_x - texture_size.x		, traveled_step_y							, 0));
			// 				xform_E = m4_translate(xform_E, v3(-traveled_step_x + texture_size.x		, traveled_step_y							, 0));
			// 				xform_F = m4_translate(xform_F, v3(-traveled_step_x - texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 				xform_G = m4_translate(xform_G, v3(-traveled_step_x							, traveled_step_y - texture_size.y			, 0));
			// 				xform_H = m4_translate(xform_H, v3(-traveled_step_x + texture_size.x		, traveled_step_y - texture_size.y			, 0));

			// 			}
			// 		}
			// 	}

			// 	else{ 								// right
			// 		// printf("RIGHT ");
			// 		if (traveled_step_y < 0){
			// 			// printf("DOWN\n");
			// 			xform_M = m4_translate(xform_M, v3(traveled_step_x 						, traveled_step_y - texture_size.y			, 0));
			// 			xform_A = m4_translate(xform_A, v3(traveled_step_x - texture_size.x		, traveled_step_y 							, 0));
			// 			xform_B = m4_translate(xform_B, v3(traveled_step_x						, traveled_step_y 							, 0));
			// 			xform_C = m4_translate(xform_C, v3(traveled_step_x + texture_size.x		, traveled_step_y 							, 0));
			// 			xform_D = m4_translate(xform_D, v3(traveled_step_x - texture_size.x 	, traveled_step_y - texture_size.y			, 0));
			// 			xform_E = m4_translate(xform_E, v3(traveled_step_x + texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 			xform_F = m4_translate(xform_F, v3(traveled_step_x - texture_size.x 	, traveled_step_y - (texture_size.y * 2)	, 0));
			// 			xform_G = m4_translate(xform_G, v3(traveled_step_x						, traveled_step_y - (texture_size.y * 2)	, 0));
			// 			xform_H = m4_translate(xform_H, v3(traveled_step_x + texture_size.x		, traveled_step_y - (texture_size.y * 2)	, 0));
			// 		}
			// 		else{
			// 			// printf("UP\n");
			// 			xform_M = m4_translate(xform_M, v3(traveled_step_x 						, traveled_step_y							, 0));
			// 			xform_A = m4_translate(xform_A, v3(traveled_step_x - texture_size.x		, traveled_step_y + texture_size.y			, 0));
			// 			xform_B = m4_translate(xform_B, v3(traveled_step_x						, traveled_step_y + texture_size.y			, 0));
			// 			xform_C = m4_translate(xform_C, v3(traveled_step_x + texture_size.x		, traveled_step_y + texture_size.y			, 0));
			// 			xform_D = m4_translate(xform_D, v3(traveled_step_x - texture_size.x 	, traveled_step_y							, 0));
			// 			xform_E = m4_translate(xform_E, v3(traveled_step_x + texture_size.x		, traveled_step_y							, 0));
			// 			xform_F = m4_translate(xform_F, v3(traveled_step_x - texture_size.x 	, traveled_step_y - texture_size.y			, 0));
			// 			xform_G = m4_translate(xform_G, v3(traveled_step_x						, traveled_step_y - texture_size.y			, 0));
			// 			xform_H = m4_translate(xform_H, v3(traveled_step_x + texture_size.x		, traveled_step_y - texture_size.y			, 0));
			// 		}
			// 	}

			// 	if (x_axis_fix){
			// 		xform_M = m4_translate(xform_M, v3(0, -texture_size.y, 0));
			// 		xform_A = m4_translate(xform_A, v3(0, -texture_size.y, 0));
			// 		xform_B = m4_translate(xform_B, v3(0, -texture_size.y, 0));
			// 		xform_C = m4_translate(xform_C, v3(0, -texture_size.y, 0));
			// 		xform_D = m4_translate(xform_D, v3(0, -texture_size.y, 0));
			// 		xform_E = m4_translate(xform_E, v3(0, -texture_size.y, 0));
			// 		xform_F = m4_translate(xform_F, v3(0, -texture_size.y, 0));
			// 		xform_G = m4_translate(xform_G, v3(0, -texture_size.y, 0));
			// 		xform_H = m4_translate(xform_H, v3(0, -texture_size.y, 0));
			// 	}

			// 	draw_image_xform(texture->image, xform_M, texture_size, color);
			// 	draw_image_xform(texture->image, xform_A, texture_size, color);
			// 	draw_image_xform(texture->image, xform_B, texture_size, color);
			// 	draw_image_xform(texture->image, xform_C, texture_size, color);
			// 	draw_image_xform(texture->image, xform_D, texture_size, color);
			// 	draw_image_xform(texture->image, xform_E, texture_size, color);
			// 	draw_image_xform(texture->image, xform_F, texture_size, color);
			// 	draw_image_xform(texture->image, xform_G, texture_size, color);
			// 	draw_image_xform(texture->image, xform_H, texture_size, color);
		// }


		// :Update entities || :Item pick up
		{
			// Chunk* chunk = get_player_chunk();
			// for (int i = 0; i < MAX_ENTITY_COUNT; i++) { // NOTE: actually faster to use MAX_ENTITY_COUNT here
			for (int i = 0; i < player_chunk->entity_count; i++) { 
				// Entity* en = &world->dimension->entities[i];
				Entity* en = &player_chunk->entities[i];
				if (en && en->is_valid) {

					// item pick up
					if (en->is_item) {
						// TODO PHYSICS

						if (fabsf(v2_dist(en->pos, world->player->en->pos)) < world->player->item_pickup_radius) {
							add_item_to_inventory(en->item_id, en->name, 1, en->arch, en->sprite_id, en->tool_id, true);

							// render_pickup_text(en);
							trigger_pickup_text(*en);

							// entity_destroy(en);
							entity_destroy(en, player_chunk);
							
						}
					}

					// :crafting
					else if (en->is_crafting_station){
						if (en->workstation_data.selected_crafting_item){
							float cooking_time = en->workstation_data.selected_crafting_item->cooking_time;
							if (en->workstation_data.crafting_end_time == 0){
								en->workstation_data.crafting_end_time = now() + cooking_time;
								assert(world->player->selected_workstation != NULL, "Selected workstation was a NULLPTR, when trying to trigger an animation");
								trigger_animation(crafting_animation, v2(world->player->selected_workstation->en->pos.x, world->player->selected_workstation->en->pos.y), selected_recipe_workbench->cooking_time);

							}

							float alpha = alpha_from_end_time(en->workstation_data.crafting_end_time, cooking_time);

							if (has_reached_end_time(en->workstation_data.crafting_end_time)){
								printf("CRAFTED ITEM\n");
								// craft item
								{
									Entity* item = entity_create();
									setup_item(item, en->workstation_data.selected_crafting_item->item_id);
									item->pos = en->pos;
								}

								en->workstation_data.crafting_queue--;
								en->workstation_data.crafting_end_time = 0;
								assert(en->workstation_data.crafting_queue >= 0, "asd");
								if (en->workstation_data.crafting_queue == 0){
									en->workstation_data.selected_crafting_item = 0;
								}
							}
						}
					}
				}
			}
		}

		// if (world_frame.selected_entity) printf("selected en = %d\n", world_frame.selected_entity->arch);

		// :player use || :trigger workstation ui || MOUSE BUTTON RIGHT
		{
			// Chunk* chunk = get_player_chunk();
			if (world_frame.selected_entity && world_frame.selected_entity->arch == ARCH_workstation){
				// open chest
				if (is_key_just_pressed(MOUSE_BUTTON_RIGHT) || is_key_just_pressed(KEY_player_use)) {
					consume_key_just_pressed(MOUSE_BUTTON_RIGHT);

					// this might do none
					if (world->ux_state == UX_nil){
						consume_key_just_pressed(KEY_player_use);
					}

					// world->ux_state = (world->ux_state == UX_chest ? UX_nil : UX_chest);
					// Entity* selected_en = world_frame.selected_entity;
					switch (world_frame.selected_entity->workstation_id){
						case WORKSTATION_chest: {world->ux_state = UX_chest;}break;
						case WORKSTATION_furnace: {world->ux_state = UX_furnace;}break;
						case WORKSTATION_workbench: {world->ux_state = UX_workbench;}break;
						default:{}break;
					}
					printf("UX STATE = '%d'\n", world->ux_state);
				}
			}
			else if (!world_frame.selected_entity && item_in_hand && item_in_hand->item_id == ITEM_TOOL_torch){
				if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)){
					consume_key_just_pressed(MOUSE_BUTTON_RIGHT);
					Entity* torch = entity_create();
					setup_torch(torch);
					torch->pos = get_mouse_pos_in_world_space();
					delete_item_from_inventory(torch->item_id, 1);
					stop_player_torch_animation();
					
				}
			}

		}


		// idk what this is doing here!? move somewhere idk!?
		if (world->ux_state != UX_nil){
			render_workstation_ui(world->ux_state);
		}



		// :Player attack || ::attack || :Spawn item || :MOUSE BUTTON LEFT
		{
			// @PIN1: instead of switch case, maybe just do "generateLoot(selected_en->arch, 0, selected_en->pos);"
			// and in the generateLoot func decide what loot table to use based on the passed arch

			// @pin3 can the "first item not being rendered - bug" be because of world->entity_count?? like if i break a tree and spawn the item, the entity count is + - 0, and the item entity is the final instance in the entities list.
			// UPDATE ON @pin3: this is precisely why the bug happens
			// one possible fix: https://chatgpt.com/share/8e7d0f98-8706-4f0c-8a79-7aa9f02aee56


			Entity* selected_en = world_frame.selected_entity;
			bool allow_destroy = false;


			if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);
				
				// Play audio
				if (item_in_hand){

					// swing sound if tool is selected
					switch (item_in_hand->tool_id){
						// case TOOL_pickaxe:{play_one_audio_clip(audioFiles[AUDIO_swing_fast].path);}break;
						// case TOOL_axe:{play_one_audio_clip(audioFiles[AUDIO_swing_slow].path);}break;
						// case TOOL_shovel:{play_one_audio_clip(audioFiles[AUDIO_swing_slow].path);}break;
						case TOOL_pickaxe: play_one_audio_clip_with_config(audioFiles[AUDIO_swing_fast].path, audio_config_player); break;
						case TOOL_axe: play_one_audio_clip_with_config(audioFiles[AUDIO_swing_slow].path, audio_config_player); break;
						case TOOL_shovel: play_one_audio_clip_with_config(audioFiles[AUDIO_swing_slow].path, audio_config_player); break;
						default:{printf("Failed to play specific audio. toolID = %d\n", item_in_hand->tool_id);}break;		// hopefully this won't cause a crash. Because of trying to print tool_id if selected_item has no tool_id
					}
				}

				// if mouse is close to an entity (selected)  AND  selected entity is destroyable  AND  player has selected an item from hotbar
				if (selected_en && selected_en->destroyable && item_in_hand) {

					// printf("SELECTED EN ITEM ID = '%d'\n", selected_en->item_id);
						
					// get entity pos (for playing audio at position)
					// Vector3 audio_pos = v3(get_mouse_pos_in_ndc(selected_en->pos.x, selected_en->pos.y).x, get_mouse_pos_in_ndc(selected_en->pos.x, selected_en->pos.y).y, 0);
					Vector3 audio_pos = v3(selected_en->pos.x, selected_en->pos.y, 0);
					audio_config.position_ndc = v3(audio_pos.x, audio_pos.y, 0);



					switch (selected_en->arch) {
						case ARCH_tree: {	// |------- TREE -------|
							{
								if (item_in_hand->tool_id == TOOL_axe){

									// play_one_audio_clip_at_position(audioFiles[AUDIO_hit_wood1].path, audio_pos);

									selected_en->health -= world->player->damage;
									if (selected_en->health <= 0) {
										generateLoot(lootTable_pine_tree, 0, v2(selected_en->pos.x + get_sprite(selected_en->sprite_id)->image->width * 0.5, selected_en->pos.y));
										// Entity* en = entity_create();
										// setup_item(en, ITEM_pine_wood);
										// en->pos = selected_en->pos;
										allow_destroy = true;
										// play_one_audio_clip_at_position(audioFiles[AUDIO_wood_breaking1].path, audio_pos);
									}
								}
								else{printf("WRONG TOOL\n");}
							}
						} break;

						case ARCH_rock: {	// |------- ROCK -------|
							{
								if (item_in_hand->tool_id == TOOL_pickaxe){

									// play_one_audio_clip_at_position(audioFiles[AUDIO_hit_metal1].path, audio_pos);
									play_one_audio_clip_with_config(audioFiles[AUDIO_hit_metal1].path, audio_config);

									selected_en->health -= world->player->damage;
									if (selected_en->health <= 0) {
										// entity_destroy(selected_en);
										generateLoot(lootTable_rock, 0, selected_en->pos);
										allow_destroy = true;
										// play_one_audio_clip_at_position(audioFiles[AUDIO_rock_breaking1].path, audio_pos);
										play_one_audio_clip_with_config(audioFiles[AUDIO_rock_breaking1].path, audio_config);
									}
								}
								else{printf("WRONG TOOL\n");}
							}
						} break;

						case ARCH_workstation: {	// |------- WORKSTATION -------|
							{
								Entity* en = entity_create();
								setup_item_workstation(en, selected_en->workstation_id);
								// printf("SELECTED EN WORKSTATION ID = %d\n", en->workstation_id);
								en->pos = selected_en->pos;
								allow_destroy = true;

							}
						} break;

						case ARCH_ore: {	// |------- ORES -------|
							{
								if (item_in_hand->tool_id == TOOL_pickaxe){
									// play_one_audio_clip_at_position(audioFiles[AUDIO_hit_metal1].path, audio_pos);
									play_one_audio_clip_with_config(audioFiles[AUDIO_hit_metal1].path, audio_config);
									selected_en->health -= world->player->damage;
									if (selected_en->health <= 0){

										// create entity for item
										Entity* en = entity_create();
										setup_item(en, selected_en->item_id);
										en->pos = selected_en->pos;

										allow_destroy = true;
										// play_one_audio_clip_at_position(audioFiles[AUDIO_rock_breaking1].path, audio_pos);
										play_one_audio_clip_with_config(audioFiles[AUDIO_rock_breaking1].path, audio_config);
									}
								}
								else{printf("WRONG TOOL\n");}
							}
						} break;

						case ARCH_foliage: {
							{
								if (selected_en->item_id != ITEM_nil){
									Entity* en = entity_create_to_chunk(player_chunk);
									setup_item(en, selected_en->item_id);
									en->pos = selected_en->pos;
								}
								allow_destroy = true;
							}
						} break;

						default: { 	// |------- OTHERS -------|
							{
								printf("DEBUG OTHER ENTITY CREATED. ENTITY COUNT +1. ITEMID = %d\n", selected_en->item_id);
								// Entity* en = entity_create();
								Entity* en = entity_create_to_chunk(player_chunk);
								setup_item(en, selected_en->item_id);
								en->pos = selected_en->pos;
								allow_destroy = true;

								// TODO: fix this caveman solution
								if (selected_en->has_animation && selected_en->arch == ARCH_torch){
									kill_animation(ground_torch_animation);
								}
								
							}
						} break;
					}
				}	// item_in_hand == NULL
				else{
					if (item_in_hand){
						// |------- SHOVEL -------|
						// #portal
						if (item_in_hand->tool_id == TOOL_shovel){
							// if (world->dimension->current_biome_id == BIOME_forest){
							if (world->dimension->id == DIM_overworld){
								create_portal_to(DIM_cavern, true); 
							}
						}
					}
				}


				if (allow_destroy){
					// entity_destroy(selected_en);
					entity_destroy(selected_en, player_chunk);
					// entity_bin[entity_bin_size] = selected_en;
					// entity_bin_size++;
					player_chunk->has_been_modified = true;
				}
			}
		}

		// :Player use 'F'
		// @pin6: selecting stuff is split between two places. combine
		{
			if (is_key_just_pressed('F')){
				consume_key_just_pressed('F');

				// printf("PLAYER USE\n");

				Entity* selected_en = world_frame.selected_entity;
				if (selected_en){

					// if (selected_en->is_crafting_station){
					// 	world->open_crafting_station = selected_en;
					// }

					switch (selected_en->arch){

						case ARCH_portal:{ // #portal
							{
								printf("portal clicked ('f')\n");
								if (selected_en->portal_data.enabled){
									change_dimensions(selected_en->portal_data.dim_destination, selected_en->pos);
								}
							}
						} break;

						case ARCH_workstation:{
							{
								printf("SELECTED WORKSTATION\n");
							}
						} break;

						default:{}break;
					}
				}
			}
		}

		// Render entities
		// render_entities(world);
    	render_chunk_entities();
		render_player();


		// update animations
		update_animations(delta_t);

		
		// render screen effects
		// render vignette || :vignette
		if (enable_vignette){
			if (world->dimension->id == DIM_cavern){
				{
					Texture* texture;
					if (item_in_hand && item_in_hand->item_id == ITEM_TOOL_torch){
						texture = get_texture(TEXTURE_vignette_torch);
					}
					else{
						texture = get_texture(TEXTURE_vignette_no_torch);
					}
					Matrix4 xform = m4_identity;
					// Vector2 pos = camera_pos;
					Vector2 pos = get_player_pos();
					xform = m4_translate(xform, v3(pos.x, pos.y, 0));
					xform = m4_translate(xform, v3(texture->image->width * -0.5, texture->image->height * -0.5, 0));
					draw_image_xform(texture->image, xform, v2(screen_width, screen_height), COLOR_WHITE);
				}
			}
		}
		
		
		update_pickup_text(delta_t);

		// render keybinding
		if (world_frame.selected_entity && world_frame.selected_entity->arch == ARCH_portal){
			render_keybinding(world_frame.selected_entity, KEY_player_use);
		}

		if (render_fps){
			set_screen_space();
			draw_text(font, sprint(get_heap_allocator(), STR("%d"), fps), font_height, v2(0,screen_height-3), v2(0.1, 0.1), COLOR_WHITE);
			set_world_space();
		}


		if (IS_DEBUG){
			world->player->damage = 10;
			set_screen_space();
			draw_text(font, STR("DEBUG"), font_height, v2(screen_width/2-15,screen_height-10), v2(0.2, 0.2), COLOR_RED);
			set_world_space();

		    chunk_debug_print();
		}
		else{
			world->player->damage = 1;
		}

		// #animation test
		// draw_animation(torch_animation, now, v2(get_player_pos().x, get_player_pos().y));

		// ::DEBUG STUFF ------------------------------------------------------------------------------->

		// runtime debug
		if (is_key_just_pressed('P')){
			if (!runtime_debug){
				runtime_debug = true;
			}
			else{
				runtime_debug = false;
			}
			// update_biome();
			// player->en->pos.x -= 10; 
			// world_frame.player->pos.x -= 10;
			// printf("%.0f, %.0f\n", get_player_pos().x, get_player_pos().y);
		}

		// printf("current dim = %d\n", world->dimension->id);


		// if(runtime_debug){
			// {
			// 	// Vector2 size = v2(30, 20) ;
			// 	Matrix4 xform = m4_identity;
			// 	xform = m4_translate(xform, v3(0, 10, 0));

			// 	Sprite* sprite = &(Sprite){.image=load_image_from_disk(STR("res/textures/grass.png"), get_heap_allocator())};
			// 	Vector2 size = get_sprite_size(sprite);

			// 	Vector4 col = v4(0.6, 0.7, 1.0, 1);

			// 	draw_image_xform(sprite->image, xform, v2(size.x, size.y), col);

			// }
		// }

		// if (world_frame.selected_entity){
		// 	printf("Selected entity = %s\n", world_frame.selected_entity->name);
		// 	if (world_frame.selected_entity->arch == ARCH_portal){
		// 		printf("Selected portal id = %d\n", world_frame.selected_entity->portal_data.id);
		// 		printf("Selected portal pos = %.1f,%.1f\n", world_frame.selected_entity->portal_data.pos.x, world_frame.selected_entity->portal_data.pos.y);

		// 	}
		// 		printf("\n");
		// }


		// :Input ------------------------------------------------------------------------------------>
		if (is_key_just_pressed('X')){ // EXIT
			window.should_close = true;
		}
		if (is_key_just_pressed('V')){
			if (IS_DEBUG){
				IS_DEBUG = false;
			}
			else{
				IS_DEBUG = true;
			}
		}

		if (is_key_just_pressed('Q')){
			{
				InventoryItemData* item = item_in_hand;
				if (item){
					Vector2 pos = get_player_pos();
					pos.y -= 20;
					spawn_item_to_world(*item, pos);
					delete_item_from_inventory(item->item_id, item->amount);
				}
			}
		}


		// Mousewheel ZOOM (debug)
		// Selecting slots with mouse wheel
		for (u64 i = 0; i < input_frame.number_of_events; i++) {
			Input_Event e = input_frame.events[i];
			switch (e.kind) {
				case (INPUT_EVENT_SCROLL):
				{
					if (e.yscroll > 0){
						if (is_key_down(KEY_CTRL)){view_zoom -= 0.03;}
						else{
							selected_slot_index -= 1;
							if (selected_slot_index < 0){
								selected_slot_index = 8;
							}
						}
					}
					else{
						if (is_key_down(KEY_CTRL)){view_zoom += 0.03;}
						else{
							selected_slot_index += 1;
							if (selected_slot_index > 8){
								selected_slot_index = 0;
							}
						}
					}
					break;
				}
				case (INPUT_EVENT_KEY):{break;}
				case (INPUT_EVENT_TEXT):{break;}
				default:{}break;
			}
		}


		// selecting slots with keys
		if (is_key_just_pressed('1')) {selected_slot_index = 1 - 1;}
		if (is_key_just_pressed('2')) {selected_slot_index = 2 - 1;}
		if (is_key_just_pressed('3')) {selected_slot_index = 3 - 1;}
		if (is_key_just_pressed('4')) {selected_slot_index = 4 - 1;}
		if (is_key_just_pressed('5')) {selected_slot_index = 5 - 1;}
		if (is_key_just_pressed('6')) {selected_slot_index = 6 - 1;}
		if (is_key_just_pressed('7')) {selected_slot_index = 7 - 1;}
		if (is_key_just_pressed('8')) {selected_slot_index = 8 - 1;}
		if (is_key_just_pressed('9')) {selected_slot_index = 9 - 1;}

		// Sprint
		if (is_key_down(KEY_SHIFT)){ world->player->is_running = true;}
		else { world->player->is_running = false;}

		// ::Player movement || ::Movement
		Vector2 input_axis = v2(0, 0);
		if (is_key_down('W')){input_axis.y += 1.0;}
		if (is_key_down('A')){input_axis.x -= 1.0;}
		if (is_key_down('S')){input_axis.y -= 1.0;}
		if (is_key_down('D')){input_axis.x += 1.0;}

		// check for collisions
		// check_for_collisions(input_axis);


		// normalize
		input_axis = v2_normalize(input_axis);

		// player_pos = player_pos + (input_axis * 10.0);


		// @ship del this
		if (IS_DEBUG){
			world->player->running_speed_amount = 1000;
		}
		else{
			world->player->running_speed_amount = 100;
		}
		
		if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed_amount * delta_t)); }
		else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }
	}

		gfx_update();

		// :Timing
		seconds_counter += delta_t;
		frame_count += 1;
		if (seconds_counter > 1.0){
			if (print_fps){
				log("fps: %i", frame_count);
			}
			fps = frame_count;
			seconds_counter = 0.0;
			frame_count = 0;
		}
	}

	return 0;
}