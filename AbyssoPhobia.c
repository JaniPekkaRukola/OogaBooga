// AbyssoPhobia (19.10.2024)

#include "headers/masterheader.h"

int saatna = 1;
bool saatnaan = false;
bool level_selected = false;


void render_background(){

	if (world->game_state == GAMESTATE_hub){
		Background* bg = get_background(BACKGROUND_hub);
		assert(bg, "background was NULL");

		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(bg->image->width * -0.25, bg->image->height * -0.25, 0));

		if (bg->has_custom_size) draw_image_xform(bg->image, xform, bg->custom_size, COLOR_WHITE);
		else draw_image_xform(bg->image, xform, v2(bg->image->width, bg->image->height), COLOR_WHITE);
	}

	else if (world->game_state == GAMESTATE_level || world->game_state == GAMESTATE_editor){
		// Matrix4 xform = m4_identity;
		// xform = m4_translate(xform, v3(0, 0, 0));
		// draw_rect_xform(xform, v2(screen_width, screen_height), v4(0, 0.3, 1, 1));

		// Vector2 player_pos = get_player_pos();
		// Vector2 size = v2(800, 500);

		// draw_rect(v2(player_pos.x - (size.x * 0.5), -size.y), size, v4(0, 0.3, 0.8, 1));

		set_world_space();

		Level* level = &world->level;
		
		if (!level || level == NULL || level->id == 0) return;

		Vector2 size = v2(level->level->width, level->level->height);

		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(0, -size.y, 0));

		draw_image_xform(level->level, xform, size, COLOR_WHITE);

		if (IS_DEBUG){
			draw_image_xform(level->level_meta, xform, size, COLOR_WHITE);
		}

	}
}

void render_surfaceLine(){
    Vector2 pos = get_player_pos();
    int size_x = screen_width * 3;
    draw_rect(v2(pos.x-(size_x*0.5), 0), v2(size_x, 1), COLOR_RED);
}

void render_oxygen(){

    set_screen_space();

    Vector2 oxygen_bar_size = v2(100, 5);

    float oxygen_amount = clamp(world->player->oxygen, 0, 100);

    Matrix4 xform = m4_identity;
    xform = m4_translate(xform, v3((screen_width * 0.5) - (oxygen_bar_size.x * 0.5), screen_height-oxygen_bar_size.y, 0));
    
    // border
    draw_rect_xform(xform, oxygen_bar_size, v4(0.5, 0.5, 0.5, 1));

    // oxygen bar
    xform = m4_translate(xform, v3(1, 1, 0));
    draw_rect_xform(xform, v2(oxygen_amount-2, oxygen_bar_size.y-2), v4(0, 0, 1, 1));

    set_world_space();

}

void render_ui(){

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
		// world->ux_state = UX_nil;
        world->ux_state = UX_gameplay;
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
			EntityID tab_order[4] = {ENTITY_nil, ENTITY_item, ENTITY_tool, ENTITY_workstation};		// compiler gives a warning if using the tab_count as the size of the array. #remember to keep these values the same
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
				ItemData* item = &workbench_recipes[i];

				// tabs
				if (item->category != workbench_tab_category && workbench_tab_category != ENTITY_nil){
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
							selected_recipe_xform = xform_icon;
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
				draw_rect_xform(m4_translate(selected_recipe_xform, v3(0,-1,0)), v2(icon_size, 1), COLOR_WHITE);

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
				selected_recipe_xform = m4_identity;
				is_recipe_selected = false;
			}
		}
	}


	// // :RENDER CHEST UI || :Chest ui
	// else if (ux_state == UX_chest){
	// 	world->player->inventory_ui_open = false;

	// 	// shared variables
	// 	const int middle_padding = 10;
	// 	const int slot_size = 8;
	// 	const float padding = 2.0;
	// 	float slot_border_width = 1;
	// 	Vector4 tooltip_bg;

	// 	// INVENTORY VARIABLES
	// 	const int max_slots_row_inventory = 5;
	// 	const int max_rows_inventory = 6;
	// 	Draw_Quad* quad_item_inventory;	// pointer to item
	// 	Vector2 inventory_bg_pos;

	// 	Vector2 inventory_bg_size = v2(max_slots_row_inventory * slot_size + (max_slots_row_inventory * padding) + padding * 2, max_rows_inventory * slot_size + (max_rows_inventory * padding) + padding * 2);
	// 	inventory_bg_pos = v2((screen_width * 0.5) - (inventory_bg_size.x) - middle_padding, (screen_height * 0.5) - (inventory_bg_size.y * 0.5));

	// 	// Colors
	// 	Vector4 inventory_icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
	// 	Vector4 inventory_bg = v4(0.0, 0.0, 0.0, 0.5);
	// 	Vector4 slot_color = v4(0.5, 0.5, 0.5, 0.6);
	// 	tooltip_bg = inventory_bg;

	// 	// init xform
	// 	Matrix4 xform_inventory_bg = m4_identity;

	// 	// xform translate inventory background
	// 	xform_inventory_bg = m4_translate(xform_inventory_bg, v3(inventory_bg_pos.x, inventory_bg_pos.y, 0));

	// 	// draw inventory background and take quad for collisions
	// 	Draw_Quad* inventory_quad = draw_rect_xform(xform_inventory_bg, v2(inventory_bg_size.x, inventory_bg_size.y), inventory_bg);

	// 	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	// 	// CHEST VARIABLES
	// 	const int max_slots_row_chest = 5;
	// 	const int max_rows_chest = 6;

	// 	const Vector2 chest_ui_size = v2((max_slots_row_chest * slot_size) + (max_slots_row_chest * padding) + padding * 2, (max_rows_chest * slot_size) + (max_rows_chest * padding) + padding * 2);
	// 	Vector2 chest_ui_pos = v2((screen_width * 0.5) + middle_padding, (screen_height * 0.5) - (chest_ui_size.y * 0.5));

	// 	Draw_Quad* quad_item_chest;	// pointer to item

	// 	// Colors
	// 	Vector4 chest_icon_background_col = v4(1.0, 1.0, 1.0, 0.2);
	// 	Vector4 chest_bg = v4(0.0, 0.0, 0.0, 0.5);
	// 	Vector4 slot_border_color = v4(1, 1, 1, 0.7);

	// 	Matrix4 xform_chest_bg = m4_identity;
	// 	xform_chest_bg = m4_translate(xform_chest_bg, v3(chest_ui_pos.x, chest_ui_pos.y, 0));

	// 	// draw chest background and take quad for collisions
	// 	chest_quad = draw_rect_xform(xform_chest_bg, v2(chest_ui_size.x, chest_ui_size.y), chest_bg);

	// 	// @pin4 could this be the issue why pointer points to player inventory instead of chest inventory?
	// 	//                            VV     '&' missing
	//     WorkstationData* selected_chest = world->player->selected_workstation;

	// 	world->chest_alpha_target = (world->ux_state == UX_chest ? 1.0 : 0.0);
	// 	animate_f32_to_target(&world->chest_alpha, world->chest_alpha_target, delta_t, 15.0);
	// 	bool is_chest_enabled = world->chest_alpha_target == 1.0;
		
	// 	if (world->chest_alpha_target != 0.0)
	// 	{
	// 		// INVENTORY UI ------------------------------------------------------>
	// 		{
	// 			// position where drawing items starts (top left)
	// 			Vector2 item_start_pos = v2(inventory_bg_pos.x + padding, inventory_bg_pos.y + inventory_bg_size.y - slot_size - padding);

	// 			// item variables
	// 			int slot_index = 0; // basically column index but with sexier name
	// 			int row_index = 0;

	// 			// draw empty slots
	// 			for (int i = 0; i < max_slots_row_inventory * max_rows_inventory; i++){
					
	// 				if (slot_index >= max_slots_row_inventory){
	// 					slot_index = 0;
	// 					row_index++;
	// 				}

	// 				Matrix4 xform_item = m4_identity;
	// 				xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

	// 				// draw empty slot
	// 				draw_rect_xform(xform_item, v2(slot_size, slot_size), slot_color);
	// 				// TODO: make it a rect with border 'draw_rect_with_border'

	// 				slot_index++;
	// 			}

	// 			// reset these variables
	// 			slot_index = 0;
	// 			row_index = 0;

	// 			// draw items
	// 			for(int i = 0; i < ITEM_MAX; i++){
	// 				InventoryItemData* inventory_item = &world->player->inventory[i];
	// 				if (inventory_item != NULL && inventory_item->amount > 0){

	// 					// change to next row if row is full
	// 					if (slot_index >= max_slots_row_inventory){
	// 						slot_index = 0;
	// 						row_index++;
	// 					}

	// 					// get sprite
	// 					Sprite* sprite = get_sprite(inventory_item->sprite_id);

	// 					// skip to next item if sprite is null. NOTE: prolly useless since the "inventory_item != NULL" if statement.
	// 					if (!sprite || !sprite->image){
	// 						continue;
	// 					}

	// 					Matrix4 xform_item = m4_identity;
	// 					xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

	// 					// save xform_item for later when drawing item counts
	// 					Matrix4 xform_item_count = xform_item;

	// 					// get quad
	// 					quad_item_inventory = draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), v4(0,0,0,0));

	// 					slot_index++;

	// 					// hovering item
	// 					if (quad_item_inventory && range2f_contains(quad_to_range(*quad_item_inventory), get_mouse_pos_in_ndc())){

	// 						// tooltip
	// 						if (enable_tooltip){

	// 							Vector2 tooltip_size = v2(inventory_bg_size.x, 30);

	// 							Matrix4 xform_tooltip = m4_identity;
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(inventory_bg_pos.x, inventory_bg_pos.y - tooltip_size.y - padding, 0));

	// 							draw_rect_xform(xform_tooltip, v2(tooltip_size.x, tooltip_size.y), tooltip_bg);

	// 							string item_name = inventory_item->name;

	// 							Gfx_Text_Metrics tooltip_metrics = measure_text(font, item_name, font_height, v2(0.1, 0.1));
	// 							Vector2 justified = v2_sub(justified, v2_divf(tooltip_metrics.functional_size, 2));
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(tooltip_size.x * 0.5, tooltip_size.y - 5, 0));
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(justified.x, justified.y, 0));
	// 							draw_text_xform(font, item_name, font_height, xform_tooltip, v2(0.1, 0.1), COLOR_WHITE);
	// 						}

	// 						// scale item
	// 						float scale_adjust = 1.3;
	// 						xform_item = m4_scale(xform_item, v3(scale_adjust, scale_adjust, 1));

	// 						// selecting item
	// 						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 							consume_key_just_pressed(MOUSE_BUTTON_LEFT);

	// 							inventory_selected_item_in_chest_ui = *inventory_item;
	// 							delete_item_from_inventory(inventory_item->item_id, inventory_item->amount);
	// 						}
	// 					} // quad

	// 					// draw item
	// 					draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

	// 					// draw item count (not tools)
	// 					if (inventory_item->arch != ARCH_tool){
	// 						draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_item->amount), font_height, xform_item_count, v2(0.1, 0.1), COLOR_WHITE);
	// 					}
	// 				} // inventory_item != NULL && inventory_item->amount > 0
	// 			} // for loop


	// 			// dragging release
	// 			if (is_key_up(MOUSE_BUTTON_LEFT) && inventory_selected_item_in_chest_ui.valid){

	// 				// check if item is released inside of the inventory our outside
	// 				if (range2f_contains(quad_to_range(*inventory_quad), get_mouse_pos_in_ndc())){
	// 					add_item_to_inventory_quick(&inventory_selected_item_in_chest_ui);
	// 				}
	// 				else{
	// 					// check if item is released into chest
	// 					if (chest_quad && range2f_contains(quad_to_range(*chest_quad), get_mouse_pos_in_ndc())){
	// 						add_item_to_chest(inventory_selected_item_in_chest_ui);
	// 					}
	// 					else{
	// 						// TODO: make the item spawn towards the mouse cursor from player
	// 						Vector2 pos = get_player_pos();
	// 						if (!spawn_item_to_world(inventory_selected_item_in_chest_ui, v2(pos.x - 15, pos.y))){
	// 							log_error("FAILED TO SPAWN ITEM TO WORLDSPACE, returning item to inventory\n");
	// 							add_item_to_inventory_quick(&inventory_selected_item_in_chest_ui);
	// 						}
	// 					}
	// 				}

	// 				// reset selected item. Setting the valid to false functions here like a nullptr
	// 				inventory_selected_item_in_chest_ui.valid = false;
	// 			}
				
	// 			// dragging from inventory
	// 			if (inventory_selected_item_in_chest_ui.valid){

	// 				// get mouse pos
	// 				Vector2 mouse_pos_screen = get_mouse_pos_in_screen();

	// 				Matrix4 xform_item_drag = m4_identity;
	// 				xform_item_drag = m4_translate(xform_item_drag, v3(mouse_pos_screen.x, mouse_pos_screen.y, 0));
	// 				xform_item_drag = m4_translate(xform_item_drag, v3(slot_size * -0.5, slot_size * -0.5, 0));

	// 				// draw item
	// 				Draw_Quad* quad_item_drag = draw_image_xform(get_sprite(inventory_selected_item_in_chest_ui.sprite_id)->image, xform_item_drag, v2(slot_size, slot_size), COLOR_WHITE);
	// 				// draw item amount
	// 				if (inventory_selected_item_in_chest_ui.arch != ARCH_tool){
	// 					draw_text_xform(font, sprint(temp_allocator, STR("%d"), inventory_selected_item_in_chest_ui.amount), font_height, xform_item_drag, v2(0.1, 0.1), COLOR_WHITE);
	// 				}

	// 			}
	// 		}
	// 		// inventory ui end


	// 		// CHEST UI ---------------------------------------------------------->
	// 		{
	// 			// position where drawing items starts (top left)
	// 			Vector2 item_start_pos = v2(chest_ui_pos.x + padding, chest_ui_pos.y + chest_ui_size.y - slot_size - padding);

	// 			// item variables
	// 			int slot_index = 0; // basically column index but with sexier name
	// 			int row_index = 0;

	// 			// draw empty slots
	// 			for (int i = 0; i < max_slots_row_chest * max_rows_chest; i++){
					
	// 				if (slot_index >= max_slots_row_chest){
	// 					slot_index = 0;
	// 					row_index++;
	// 				}

	// 				Matrix4 xform_item = m4_identity;
	// 				xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

	// 				// draw empty slot
	// 				draw_rect_xform(xform_item, v2(slot_size, slot_size), slot_color);
	// 				// TODO: make it a rect with border 'draw_rect_with_border'

	// 				slot_index++;
	// 			}

	// 			// reset these variables
	// 			slot_index = 0;
	// 			row_index = 0;

	// 			// NOTE: this is how to add items straight to the chest inventory
	// 			// world->dimension->entities[937].workstation_data.inventory[0] = (InventoryItemData){.item_id=ITEM_berry, .amount=1,.arch=ARCH_item,.name=STR("BERRY"),.valid=true,.sprite_id=SPRITE_item_berry};

	// 			// draw items
	// 			for(int i = 0; i < ITEM_MAX; i++){
	// 				InventoryItemData* chest_item = &selected_chest->inventory[i];
	// 				if (chest_item != NULL && chest_item->amount > 0){

	// 					// change to next row if row is full
	// 					if (slot_index >= max_slots_row_chest){
	// 						slot_index = 0;
	// 						row_index++;
	// 					}

	// 					// get sprite
	// 					Sprite* sprite = get_sprite(chest_item->sprite_id);

	// 					// skip to next item if sprite is null. NOTE: prolly useless since the "chest_item != NULL" if statement.
	// 					if (!sprite || !sprite->image){
	// 						continue;
	// 					}

	// 					Matrix4 xform_item = m4_identity;
	// 					xform_item = m4_translate(xform_item, v3(item_start_pos.x + ((slot_index * slot_size) + (slot_index * padding) + padding * 0.5), item_start_pos.y - (row_index * slot_size) - (row_index * padding) - padding * 0.5, 0));

	// 					// save xform_item for later when drawing item counts
	// 					Matrix4 xform_item_count = xform_item;

	// 					// get quad
	// 					quad_item_chest = draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), v4(0,0,0,0));

	// 					slot_index++;

	// 					// hovering item
	// 					if (quad_item_chest && range2f_contains(quad_to_range(*quad_item_chest), get_mouse_pos_in_ndc())){

	// 						// :tooltip
	// 						if (enable_chest_tooltip){

	// 							Vector2 tooltip_size = v2(chest_ui_size.x, 30);

	// 							Matrix4 xform_tooltip = m4_identity;
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(chest_ui_pos.x, chest_ui_pos.y - tooltip_size.y - padding, 0));

	// 							draw_rect_xform(xform_tooltip, v2(tooltip_size.x, tooltip_size.y), tooltip_bg);

	// 							string item_name = chest_item->name;

	// 							Gfx_Text_Metrics tooltip_metrics = measure_text(font, item_name, font_height, v2(0.1, 0.1));
	// 							Vector2 justified = v2_sub(justified, v2_divf(tooltip_metrics.functional_size, 2));
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(tooltip_size.x * 0.5, tooltip_size.y - 5, 0));
	// 							xform_tooltip = m4_translate(xform_tooltip, v3(justified.x, justified.y, 0));
	// 							draw_text_xform(font, item_name, font_height, xform_tooltip, v2(0.1, 0.1), COLOR_WHITE);
	// 						}

	// 						// scale item
	// 						float scale_adjust = 1.3;
	// 						xform_item = m4_scale(xform_item, v3(scale_adjust, scale_adjust, 1));

	// 						// selecting item
	// 						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 							consume_key_just_pressed(MOUSE_BUTTON_LEFT);

	// 							chest_selected_item = *chest_item;
	// 							delete_item_from_chest(chest_item->item_id, chest_item->amount);
	// 						}
	// 					} // quad

	// 					// draw item
	// 					draw_image_xform(sprite->image, xform_item, v2(slot_size, slot_size), COLOR_WHITE);

	// 					// draw item count (not for tools)
	// 					if (chest_item->arch != ARCH_tool){
	// 						draw_text_xform(font, sprint(temp_allocator, STR("%d"), chest_item->amount), font_height, xform_item_count, v2(0.1, 0.1), COLOR_WHITE);
	// 					}
	// 				} // chest_item != NULL && chest_item->amount > 0
	// 			} // for loop

	// 			// dragging release
	// 			if (is_key_up(MOUSE_BUTTON_LEFT) && chest_selected_item.valid){

	// 				// check if item is released inside of the inventory our outside
	// 				if (range2f_contains(quad_to_range(*chest_quad), get_mouse_pos_in_ndc())){
	// 					add_item_to_chest(chest_selected_item);
	// 				}
	// 				else{
	// 					// check if item is released into inventory
	// 					if (inventory_quad && range2f_contains(quad_to_range(*inventory_quad), get_mouse_pos_in_ndc())){
	// 						add_item_to_inventory_quick(&chest_selected_item);
	// 					}
	// 					else{
	// 						// TODO: make the item spawn towards the mouse cursor from player
	// 						Vector2 pos = get_player_pos();
	// 						if (!spawn_item_to_world(chest_selected_item, v2(pos.x - 15, pos.y))){
	// 							log_error("FAILED TO SPAWN ITEM TO WORLDSPACE, returning item to inventory\n");
	// 							add_item_to_inventory_quick(&chest_selected_item);
	// 						}
	// 					}
	// 				}

	// 				// reset selected item. Setting the valid to false functions here like nullptr
	// 				chest_selected_item.valid = false;
	// 			}
				
	// 			// dragging from chest
	// 			if (chest_selected_item.valid){

	// 				// get mouse pos
	// 				Vector2 mouse_pos_screen = get_mouse_pos_in_screen();

	// 				Matrix4 xform_item_drag = m4_identity;
	// 				xform_item_drag = m4_translate(xform_item_drag, v3(mouse_pos_screen.x, mouse_pos_screen.y, 0));
	// 				xform_item_drag = m4_translate(xform_item_drag, v3(slot_size * -0.5, slot_size * -0.5, 0));

	// 				// draw item
	// 				Draw_Quad* quad_item_drag = draw_image_xform(get_sprite(chest_selected_item.sprite_id)->image, xform_item_drag, v2(slot_size, slot_size), COLOR_WHITE);
	// 				// draw item amount
	// 				if (chest_selected_item.arch != ARCH_tool){
	// 					draw_text_xform(font, sprint(temp_allocator, STR("%d"), chest_selected_item.amount), font_height, xform_item_drag, v2(0.1, 0.1), COLOR_WHITE);
	// 				}
	// 			}
	// 		} // chest ui end
	// 	}

	// 	// prevent player attacking when chest ui is open
	// 	if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 		consume_key_just_pressed(MOUSE_BUTTON_LEFT);
	// 	}
	// }


	// :RENDER FURNACE UI || :furnace ui
	// else if (ux_state == UX_furnace){
	// 	world->player->inventory_ui_open = true;
	// 	// printf("RENDERING FURNACE UI\n");

	// 	WorkstationData* selected_workstation = world->player->selected_workstation;

	// 	// furnace ui size variables
	// 	const int max_icons_row = 6;
	// 	const int max_icons_col = 4;
	// 	const int icon_size = 16;
	// 	const int padding = 2;
	// 	const int padding_bg_vert = 15;
	// 	// int slot_index = 0;
	// 	int row_index = 0;
	// 	int col_index = 0;
	// 	Vector2 icon_pos;
	// 	const int MAX_ICONS_PER_ROW = 3;
		

	// 	// const Vector2 furnace_ui_size = v2((max_slots_row * slot_size) + (max_slots_row * padding) + padding, (max_slots_col * slot_size) + (max_slots_col * padding) + padding);
	// 	// const Vector2 furnace_ui_size = v2((max_icons_row * icon_size) + (max_icons_row * padding) + padding, (max_icons_col * icon_size) + (max_icons_col * padding) + padding + padding_bg_vert);
	// 	const Vector2 furnace_ui_size = v2((max_icons_row * icon_size) + (max_icons_row * padding) + padding, (max_icons_col * icon_size) + (max_icons_col * padding) + padding + padding_bg_vert);
	// 	Vector2 furnace_ui_pos = v2(screen_width * 0.5, screen_height * 0.5);
	// 	furnace_ui_pos = v2(furnace_ui_pos.x - (furnace_ui_size.x * 0.5), furnace_ui_pos.y - (furnace_ui_size.y * 0.5));

	// 	// const float x_start_pos = furnace_ui_pos.x;
	// 	// const float y_start_pos = furnace_ui_pos.y;

	// 	// Gfx_Text_Metrics recipe_title;
	// 	// Gfx_Text_Metrics furnace_title;

	// 	// recipe info panel variables
	// 	const Vector2 recipe_panel_size = v2(40, furnace_ui_size.y - 5);
	// 	Vector2 recipe_panel_pos = v2(furnace_ui_pos.x + furnace_ui_size.x + padding, furnace_ui_pos.y + (5 * 0.5));

		


	// 	// Colors
	// 	Vector4 furnace_bg = v4(0.15, 0.15, 0.15, 0.8);
	// 	Vector4 slot_border_color = v4(1, 1, 1, 0.7);
	// 	// Vector4 slot_color = v4(1, 1, 1, 0.7);

	// 	world->furnace_alpha_target = (world->ux_state == UX_furnace ? 1.0 : 0.0);
	// 	animate_f32_to_target(&world->furnace_alpha, world->furnace_alpha_target, delta_t, 15.0);
	// 	bool is_furnace_enabled = world->furnace_alpha_target == 1.0;


	// 	// FURNACE PANEL
	// 	if (world->furnace_alpha_target != 0.0)
	// 	{
	// 		Matrix4 xform_bg = m4_identity;

	// 		xform_bg = m4_translate(xform_bg, v3(furnace_ui_pos.x, furnace_ui_pos.y, 0));
	// 		// draw backgrounds
	// 		draw_rect_xform(xform_bg, v2(furnace_ui_size.x, furnace_ui_size.y), furnace_bg);

	// 		// center furnace title
	// 		Gfx_Text_Metrics furnace_title = measure_text(font, STR("Furnace"), font_height, v2(0.1, 0.1));
	// 		Vector2 justified1 = v2_sub(justified1, v2_divf(furnace_title.functional_size, 2));
	// 		xform_bg = m4_translate(xform_bg, v3(furnace_ui_size.x * 0.5, furnace_ui_size.y, 0));		// center text box
	// 		xform_bg = m4_translate(xform_bg, v3(justified1.x, justified1.y, 0));						// center text
	// 		xform_bg = m4_translate(xform_bg, v3(0, -5, 0));											// bring down a bit

	// 		// draw title
	// 		draw_text_xform(font, STR("Furnace"), font_height, xform_bg, v2(0.1, 0.1), COLOR_WHITE);
	// 		// printf("drawing furance title\n");

	// 		// draw icons
	// 		Matrix4 xform_icon = m4_identity;

	// 		Vector2 icon_start_pos = v2(furnace_ui_pos.x + padding, furnace_ui_pos.y + furnace_ui_size.y - icon_size - padding - furnace_title.visual_size.y);


	// 		// xform_icon = m4_translate(xform_icon, v3(furnace_ui_size.x * 0.5, furnace_ui_size.y * 0.5, 0));

	// 		// draw icons
	// 		for (int i = 0; i < ITEM_MAX; i++){
	// 			ItemData* item = &furnace_recipes[i];

	// 			if (item->crafting_recipe_count != 0){

	// 				if (row_index >= MAX_ICONS_PER_ROW){
	// 					row_index = 0;
	// 					col_index++;
	// 				}

	// 				// printf("ITEM NAME = %s\n", item->name);
	// 				xform_icon = m4_identity;

	// 				Sprite* sprite = get_sprite(item->sprite_id);
					
	// 				icon_pos = v2(icon_start_pos.x + (row_index * (icon_size + padding)), icon_start_pos.y - (col_index * (icon_size + padding)) - 5);
					
	// 				xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

	// 				// draw_image_xform(sprite->image, xform_icon, get_sprite_size(sprite), COLOR_WHITE);
	// 				Draw_Quad* quad = draw_image_xform(sprite->image, xform_icon, v2(icon_size, icon_size), COLOR_WHITE);

	// 				// collision checking for icon
	// 				Range2f icon_box = quad_to_range(*quad);
	// 				if (is_furnace_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc())) {

	// 					// selecting icon
	// 					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 						consume_key_just_pressed(MOUSE_BUTTON_LEFT);

	// 						selected_recipe_furnace = item;
	// 						selected_recipe_xfrom = xform_icon;
	// 						is_recipe_selected = true;

	// 						// printf("selected item = %s\n", selected_recipe_furnace->name);
	// 					}
	// 				}
	// 				row_index++;
	// 			}
	// 		}

	// 		// :RECIPE PANEL || recipe selected || draw recipe panel
	// 		if (selected_recipe_furnace){

	// 			// draw indicator on selected recipe
	// 			draw_rect_xform(m4_translate(selected_recipe_xfrom, v3(0,-1,0)), v2(icon_size, 1), COLOR_WHITE);

	// 			// draw recipe panel
	// 			Matrix4 xform_recipe_panel = m4_identity;
	// 			xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
	// 			draw_rect_xform(xform_recipe_panel, v2(recipe_panel_size.x, recipe_panel_size.y), furnace_bg);


	// 			// center title
	// 			Gfx_Text_Metrics recipe_title = measure_text(font, STR("Recipe"), font_height, v2(0.1, 0.1));
	// 			Vector2 justified2 = v2_sub(justified2, v2_divf(recipe_title.functional_size, 2));
	// 			xform_recipe_panel = m4_translate(xform_recipe_panel, v3(recipe_panel_size.x * 0.5, recipe_panel_size.y * 0.75 , 0));	// center text box
	// 			xform_recipe_panel = m4_translate(xform_recipe_panel, v3(justified2.x, justified2.y, 0));						// center text
	// 			xform_recipe_panel = m4_translate(xform_recipe_panel, v3(0, -5, 0));											// bring down a bit
	// 			// draw_rect_xform(xform_recipe_panel, justified2, COLOR_WHITE);
	// 			draw_text_xform(font, STR("Recipe"), font_height, xform_recipe_panel, v2(0.1, 0.1), COLOR_WHITE);

	// 			Vector2 panel_icon_start_pos = v2(recipe_panel_pos.x + padding, recipe_panel_pos.y + (recipe_panel_size.y * 0.5) - icon_size - padding - recipe_title.visual_size.y);

	// 			// draw recipe icon
	// 			Matrix4 xform_recipe_icon = m4_identity;
	// 			Sprite* recipe_icon_sprite = get_sprite(selected_recipe_furnace->sprite_id);
	// 			Vector2 recipe_icon_sprite_size = get_sprite_size(recipe_icon_sprite);
	// 			xform_recipe_icon = m4_translate(xform_recipe_icon, v3(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) - (recipe_icon_sprite_size.x * 0.5), recipe_panel_pos.y + recipe_panel_size.y - recipe_icon_sprite_size.y - 5, 0));
	// 			draw_image_xform(get_sprite(selected_recipe_furnace->sprite_id)->image, xform_recipe_icon, recipe_icon_sprite_size, COLOR_WHITE);

	// 			const int MAX_ICONS_PER_ROW_panel = 5;
	// 			int recipe_icon_index = 0;
	// 			int recipe_row_index = 0;
	// 			int recipe_col_index = 0;
	// 			Vector2 pos;

	// 			// draw recipe materials
	// 			for (int i = 0; i < MAX_RECIPE_ITEMS; i++){
	// 				ItemAmount* recipe_item = &selected_recipe_furnace->crafting_recipe[i];

	// 				// draw recipe icon
	// 				if (recipe_item->amount != 0){

	// 					if (recipe_icon_index >= MAX_ICONS_PER_ROW_panel){
	// 						recipe_icon_index = 0;
	// 						recipe_col_index++;
	// 					}

	// 					Matrix4 xform = m4_identity;
	// 					Sprite* sprite = get_sprite(get_sprite_from_itemID(recipe_item->id));

	// 					// icon_pos = v2(icon_panel_icon_start_pos.x + (row_index * (icon_size + padding)), icon_panel_icon_start_pos.y - (col_index * (icon_size + padding)));
	// 					// xform_icon = m4_translate(xform_icon, v3(icon_pos.x, icon_pos.y, 0));

	// 					// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));
	// 					pos = v2(recipe_panel_pos.x + (recipe_panel_size.x * 0.5) + (recipe_icon_index * (icon_size + padding)) - (icon_size * 0.5), panel_icon_start_pos.y - (recipe_col_index * (icon_size + padding)));

	// 					// pos = v2(panel_icon_start_pos.x + (recipe_icon_index * (icon_size + padding)), panel_icon_start_pos.y);

	// 					xform = m4_translate(xform, v3(pos.x, pos.y, 0));
						
	// 					draw_image_xform(sprite->image, xform, v2(icon_size, icon_size), COLOR_WHITE);
	// 					draw_text_xform(font, sprint(temp_allocator, STR("%d/%d"), get_player_inventory_item_count(recipe_item->id), recipe_item->amount), font_height, m4_translate(xform, v3(-8, + (icon_size * 0.5), 0)), v2(0.1, 0.1), COLOR_WHITE);

	// 					recipe_icon_index++;
	// 				}

	// 			}

	// 			// SMELT BUTTON
	// 			// smelt_button(STR("Smelt"), v2(recipe_panel_pos.x, recipe_panel_pos.y), v2(recipe_panel_size.x, 10), true);

	// 			// if (smelt_button){
	// 			// 	printf("PRESSED_BUTTON\n");
	// 			// }
	// 			// if (button(STR("Song"), rect.xy, rect.zw, song_playing)) {

	// 			Matrix4 smelt_xform = m4_identity;
	// 			smelt_xform = m4_translate(smelt_xform, v3(recipe_panel_pos.x, recipe_panel_pos.y, 0));
	// 			Draw_Quad* quad = draw_rect_xform(smelt_xform, v2(recipe_panel_size.x, 10), v4(0.5, 0.5, 0.5, 0.5));
	// 			Gfx_Text_Metrics smelt_text = measure_text(font, STR("Smelt"), font_height, v2(0.1, 0.1));
	// 			Vector2 justified3 = v2_sub(justified3, v2_divf(smelt_text.functional_size, 2));
	// 			smelt_xform = m4_translate(smelt_xform, v3(recipe_panel_size.x * 0.5, 4, 0));	// center text box
	// 			smelt_xform = m4_translate(smelt_xform, v3(justified3.x, justified3.y, 0));						// center text
	// 			// smelt_xform = m4_translate(smelt_xform, v3(0, -5, 0));											// bring down a bit
	// 			draw_text_xform(font, STR("Smelt"), font_height, smelt_xform, v2(0.1, 0.1), COLOR_WHITE);

	// 			// selecting smelt button
	// 			if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())) {
	// 				if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 					consume_key_just_pressed(MOUSE_BUTTON_LEFT);
	// 					printf("SMELT\n");
	// 					for (int i = 0; i < selected_recipe_furnace->crafting_recipe_count; i++){
	// 						ItemAmount* recipe_item = &selected_recipe_furnace->crafting_recipe[i];
	// 						printf("searching for itemID '%d'\n", recipe_item->id);
	// 						bool result = check_player_inventory_for_items(recipe_item->id, recipe_item->amount);
	// 						printf("RESULT = %d\n", result);

	// 						if (result == 1){

	// 							selected_workstation->selected_crafting_item = selected_recipe_furnace;
	// 							selected_workstation->crafting_queue++;

	// 							delete_item_from_inventory(recipe_item->id, recipe_item->amount);
	// 							// add_item_to_inventory(selected_recipe_furnace->item_id, selected_recipe_furnace->name, 1, ARCH_item, selected_recipe_furnace->sprite_id, TOOL_nil, true);
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}









	// 		if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
	// 			consume_key_just_pressed(MOUSE_BUTTON_LEFT);
	// 			selected_recipe_furnace = NULL;
	// 			selected_recipe_xfrom = m4_identity;
	// 			is_recipe_selected = false;
	// 		}

	// 	}
	// }



	set_world_space();
	pop_z_layer();
}

// :: render entities
void render_entities(){

    for (int i = 0; i < MAX_ENTITY_COUNT; i++){
        Entity* en = &world->entities[i];

        if (en && en->is_valid){
            switch (en->entity_id){

                // :Render player
                case ENTITY_player:{
                    {
                        Sprite* sprite = get_sprite(SPRITE_player);

                        Matrix4 xform = m4_identity;
                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
                        // draw_rect_xform(xform, v2(20, 20), COLOR_BLACK);
                        draw_image_xform(sprite->image, xform, v2(get_sprite_size(sprite).x * 2, get_sprite_size(sprite).y * 2), COLOR_WHITE);
                    }
                } break;

                default:{
                    {
                        Sprite* sprite = get_sprite(en->sprite);

                        Matrix4 xform = m4_identity;
                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
                        
                        // draw_rect_xform(xform, v2(10, 10), COLOR_BLACK);
                        draw_image_xform(sprite->image, xform, get_sprite_size(sprite), COLOR_WHITE);
                    }
                } break;
            }
        }
    }
}






// ::Entry
int entry(int argc, char **argv){
    window.title = STR("Sub Game");
    window.point_width = 1280;
    window.point_height = 720;

    window.x = window.point_width * 0.5 + 150;
    window.y = window.point_height * 0.5 - 100;
	// window.x = 50;
	// window.y = 50;

    window.clear_color = v4(0.8, 0.8, 1, 1);
    window.force_topmost = false;
    window.allow_resize = false;
    window.enable_vsync = true;

    // init world
	world = alloc(get_heap_allocator(), sizeof(World));
	memset(world, 0, sizeof(World));

    // Font
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	render_atlas_if_not_yet_rendered(font, font_height, 'A'); // fix for the stuttering bug for first time text rendering courtesy of charlie (Q&A #3)


    // ::Load
    // sprites[SPRITE_nil] = (Sprite){.image=load_image_from_disk(  STR(res_folder  ),get_heap_allocator())};
    sprites[SPRITE_nil] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/nil.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_player] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/player.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_algae1] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/algae1.png"), res_folder)   ,get_heap_allocator())};



    // Load category sprites
    sprites[SPRITE_CATEGORY_all] = (Sprite){ .image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/category_all.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_CATEGORY_items] = (Sprite){ .image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/category_item.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_CATEGORY_tools] = (Sprite){ .image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/category_tool.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_CATEGORY_workstations] = (Sprite){ .image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/Sprites/category_workstation.png"), res_folder)   ,get_heap_allocator())};



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


	// Load backgrounds
	backgrounds[BACKGROUND_nil] = (Background){.image=load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/Sprites/nil.png"), res_folder), get_heap_allocator())};
	backgrounds[BACKGROUND_hub] = (Background){.image=load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/Levels/hub_concept.png"), res_folder), get_heap_allocator()),.has_custom_size=true,.custom_size=v2(1280, 720)};
	// backgrounds[BACKGROUND_level_1] = (Background){.image=load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/Sprites/level_1.png"), res_folder), get_heap_allocator())};

	// assert all backgrounds @ship
	{
		for (BackgroundID i = 0; i < BACKGROUND_MAX; i++){
			Background* bg = &backgrounds[i];
			if (!bg->image){
                log_error("\n\nBACKGROUND '%d' was not setup correctly!", i);
                printf("Might be because BackgroundID for the bg exists, but the bg doesn't exist in 'backgrounds[]' or the image.png is missing!");
                assert(bg->image);
			}
		}
	}


	// // ::Load levels
	// CollisionBox* level_collision_box = NULL;
	// ResourceSpawn* level_resourcespawn = NULL;
	// LevelTransition* level_transition = NULL;

	// int num_collisions = 1;
	// int num_resources = 1;
	// int num_transitions = 1;




	// WorldMap
	Gfx_Image* worldmap = load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/worldmap.png"), res_folder), get_heap_allocator());

    setup_world();
    setup_player();
	setup_levels();

	LevelEditor editor;
	init_level_editor(&editor);

	// Level level_test = load_level(LEVEL_1);
	// for (int i = 0; i < my_level.boundary_count; i++) {
    // Segment seg = my_level.collision_boundaries[i];
    // // Use `seg.start` and `seg.end` for collision detection
	// }

	// parse_image("res/AbyssoPhobia/Levels/level_1_meta.png", &level_collision_box, &num_collisions, &level_resourcespawn, &num_resources, &level_transition, &num_transitions);
	// parse_image_meta(get_level(LEVEL_1));


    // spawn entitites temp
    {
        Entity* en = entity_create();
        setup_entity(en, ENTITY_algae);
    }


    // Timing
	float64 seconds_counter = 0.0;
	s32 frame_count = 0;
	float64 last_time = os_get_elapsed_seconds();

    view_zoom += 0.2;

    world->ux_state = UX_gameplay;
    // world->game_state = GAMESTATE_mainmenu;
	world->game_state = GAMESTATE_editor;


    while (!window.should_close){

		reset_temporary_storage();

		// :Timing
		float64 current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;
		last_time = current_time;
		os_update();


		// ######################################################################################################################
		// ::MAINMENU
        if (world->game_state == GAMESTATE_mainmenu){

            set_screen_space();

            // Vector2 size = v2(mainmenu_bg->width, mainmenu_bg->height);
            Vector2 size = v2(screen_width, screen_height);
			Matrix4 xform = m4_identity;
			// xform = m4_translate(xform, v3(size.x * -0.5, size.y * -0.5, 0));
			// draw_image_xform(mainmenu_bg, xform, v2(mainmenu_bg->width, mainmenu_bg->height), COLOR_WHITE);
            draw_rect_xform(xform, v2(size.x, size.y), COLOR_WHITE);

			xform = m4_translate(xform, v3(size.x * 0.5, size.y * 0.5, 0));
            draw_text_xform(font, STR("MAIN MENU"), font_height, m4_translate(xform, v3(0, 30, 0)), v2(0.2, 0.2), COLOR_BLACK);

			Matrix4 xform_play = m4_identity;
			// xform_play = m4_translate(xform_play, v3((size.x * -0.5) + 658, (size.y * -0.5) + 403, 0));
			xform_play = m4_translate(xform_play, v3((size.x * -0.5), (size.y * -0.5), 0));
			xform_play = m4_translate(xform_play, v3(658, size.y - 506, 0));

			Vector4 col = v4(1, 0, 0, 1.0);

			Draw_Quad* quad_play = draw_rect_xform(xform, v2(20, 10), COLOR_RED);

			if (range2f_contains(quad_to_range(*quad_play), get_mouse_pos_in_ndc())){
				if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                    printf("PLAY\n");
                    // world->ux_state = UX_gameplay;
                    world->game_state = GAMESTATE_hub;
					
					// if (os_is_file_s(STR("world"))) {
					// 	bool result = world_attempt_load_from_disk();
					// 	if (!result) {
					// 		// just setup a new world if it fails. temp
					// 		setup_world();
					// 	}
					// 	setup_world();
					// }
					// else {
					// 	setup_world();
					// }

					// world_save_to_disk();
				}
			}

        }

		// ######################################################################################################################
		// ::HUB
		if (world->game_state == GAMESTATE_hub){

			world_frame = (WorldFrame){0};
	
			// :Frame :update
            draw_frame.enable_z_sorting = true;
            world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

            // camera
            Vector2 target_pos = world->player->en->pos;
            animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

            world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));

            set_world_space();


			Vector2 player_pos = get_player_pos();

			if (player_pos.y <= -300){
				world->game_state = GAMESTATE_level;
				world->level = *get_level(LEVEL_1);
				world->player->en->pos = v2(0, 0);
				camera_pos = target_pos;
			}



			render_background();
            // render_surfaceLine();
            // render_ui();

            render_entities();

			// render workstation ui
            if (world->ux_state != UX_gameplay){
                render_workstation_ui(world->ux_state);
            }




            // :Input ------------------------------------------------------------------------------------>
			
			if (is_key_just_pressed(KEY_toggle_inventory)){
                consume_key_just_pressed(KEY_toggle_inventory);
                world->ux_state = UX_workbench;
            }

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

			if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed * delta_t)); }
			else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }

            // world->player->en->pos.y = clamp(world->player->en->pos.y, -100000, 0);

		}

		// ######################################################################################################################
		// ::LEVEL || ::GAMEPLAY
        if (world->game_state == GAMESTATE_level){


            world_frame = (WorldFrame){0};

            // :Frame :update
            draw_frame.enable_z_sorting = true;
            world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

            // camera
            Vector2 target_pos = world->player->en->pos;
            animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

            world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));



            set_world_space();



			Vector2 player_pos = get_player_pos();


            // oxygen
            if (world->player->en->pos.y < 0){
                world->player->oxygen -= world->player->oxygen_consumption;
                world->player->oxygen = clamp(world->player->oxygen, 0, 100);
            }
            else{
                world->player->oxygen += world->player->oxygen_refill;
                world->player->oxygen = clamp(world->player->oxygen, 0, 100);
            }



			if (player_pos.x <= -200 && player_pos.y >= -5){
				world->game_state = GAMESTATE_hub;
				world->player->en->pos = v2(0, 0);
			}





            render_background();
            render_surfaceLine();
            render_oxygen();
			// render_ui();

            render_entities();




            // printf("Player pos = %.0f, %.0f\n", world->player->pos.x, world->player->pos.y);



            // :Input ------------------------------------------------------------------------------------>


            if (is_key_just_pressed(KEY_toggle_inventory)){
                consume_key_just_pressed(KEY_toggle_inventory);
                world->ux_state = UX_workbench;
            }

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

            if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed * delta_t)); }
            else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }
            // world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed* delta_t));

            world->player->en->pos.y = clamp(world->player->en->pos.y, -100000, 0);

        }

		if (world->game_state == GAMESTATE_editor){
			set_screen_space();


			if (!level_selected){
				
				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(screen_width*0.5, screen_height * 0.5, 0));
				draw_text_xform(font, STR("Level 1"), font_height, xform, v2(0.1, 0.1), COLOR_BLACK);
				xform = m4_translate(xform, v3(50, 0, 0));
				Draw_Quad* quad = draw_rect_xform(xform, v2(10, 10), COLOR_RED);

				if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())){
					if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);
						level_selected = true;
						world->level = *get_level(LEVEL_1);
					}
				}

			}
			else{



				world_frame = (WorldFrame){0};

				// :Frame :update
				draw_frame.enable_z_sorting = true;
				world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

				// camera
				Vector2 target_pos = world->player->en->pos;
				animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

				world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
				world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
				world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));




				set_world_space();

				render_background();
				render_surfaceLine();


				handle_editor_input(&editor, delta_t);

				render_level_editor(&editor);


				Vector2 player_pos = get_player_pos();


				for (u64 i = 0; i < input_frame.number_of_events; i++) {
					Input_Event e = input_frame.events[i];
					switch (e.kind) {
						case (INPUT_EVENT_SCROLL):
						{
							if (e.yscroll > 0){
								view_zoom -= 0.03;
							}
							else{
								view_zoom += 0.03;
							}
							break;
						}
						case (INPUT_EVENT_KEY):{break;}
						case (INPUT_EVENT_TEXT):{break;}
						default:{}break;
					}
				}

				// Sprint
				if (is_key_down(KEY_SHIFT)){ world->player->is_running = true;}
				else { world->player->is_running = false;}

				// ::Player movement || ::Movement
				Vector2 input_axis = v2(0, 0);
				if (is_key_down('W')){input_axis.y += 1.0;}
				if (is_key_down('A')){input_axis.x -= 1.0;}
				if (is_key_down('S')){input_axis.y -= 1.0;}
				if (is_key_down('D')){input_axis.x += 1.0;}


				// normalize
				input_axis = v2_normalize(input_axis);


				if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed * delta_t)); }
				else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }

				world->player->en->pos.y = clamp(world->player->en->pos.y, -100000, 0);
			}

			set_screen_space();
			draw_text_xform(font, STR("LEVEL EDITOR"), font_height, m4_translate(m4_identity, v3(screen_width * 0.5, screen_height-10, 0)), v2(0.1, 0.1), COLOR_RED);

		}

		// ######################################################################################################################
		// ::MAP
		if (world->game_state == GAMESTATE_map){
			set_screen_space();
			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, v3(0, 0, 0));
			// draw_image_xform(worldmap, xform, v2(worldmap->width, worldmap->height), COLOR_WHITE);
			draw_image_xform(worldmap, xform, v2(screen_width, screen_height), COLOR_WHITE);
		}



		// ######################################################################################################################
		// ::UNIVERSAL logic

		// if (IS_DEBUG){
		// 	printf("GAMESTATE = %d\n", world->game_state);
		// }

		if (is_key_just_pressed('T')){
			// load_level(LEVEL_1);
			// change_level(get_level(LEVEL_1));
		}

		// EXIT
		if (is_key_just_pressed('X')){
			window.should_close = true;
		}

		if (is_key_just_pressed('V')){
			if (IS_DEBUG) IS_DEBUG = false;
			else IS_DEBUG = true;
		}

		if (is_key_just_pressed('G')){
			if (world->game_state != GAMESTATE_map) world->game_state = GAMESTATE_map;
			else world->game_state = GAMESTATE_hub;
		}

		if (render_fps){
			set_screen_space();
			draw_text(font, sprint(get_heap_allocator(), STR("%d"), fps), font_height, v2(0,screen_height-3), v2(0.1, 0.1), COLOR_RED);
			set_world_space();
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
					}
					else{
						if (is_key_down(KEY_CTRL)){view_zoom += 0.03;}
					}
					break;
				}
				case (INPUT_EVENT_KEY):{break;}
				case (INPUT_EVENT_TEXT):{break;}
				default:{}break;
			}
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