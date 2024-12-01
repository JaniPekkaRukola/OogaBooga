#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define m4_identity m4_make_scale(v3(1, 1, 1))

// ESSENTIALS ---------------------------------------->

    float sin_breathe(float time, float rate) {	// 0 -> 1
        return (sin(time * rate) + 1.0) / 2.0;
    }

    bool almost_equals(float a, float b, float epsilon) {
        return fabs(a - b) <= epsilon;
    }

    bool animate_f32_to_target(float* value, float target, float delta_t, float rate) {
        *value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
        if (almost_equals(*value, target, 0.001f)) {
            *value = target;
            return true; // reached
        }
        return false;
    }

    void animate_v2_to_target(Vector2* value, Vector2 target, float delta_t, float rate) {
        animate_f32_to_target(&(value->x), target.x, delta_t, rate);
        animate_f32_to_target(&(value->y), target.y, delta_t, rate);
    }

    void set_screen_space() {
        draw_frame.camera_xform = m4_scalar(1.0);
        draw_frame.projection = m4_make_orthographic_projection(0.0, screen_width, 0.0, screen_height, -1, 10);
    }

    void set_world_space() {
        // make the viewport (or whatever) to window size, instead of -1.7, 1.7, -1, 1
        draw_frame.projection = world_frame.world_projection;
        draw_frame.camera_xform = world_frame.world_view;
    }

    Vector2 get_mouse_pos_in_ndc() {
		float mouse_x = input_frame.mouse_x;
		float mouse_y = input_frame.mouse_y;
		Matrix4 proj = draw_frame.projection;
		Matrix4 view = draw_frame.camera_xform;
		float window_w = window.width;
		float window_h = window.height;

		// Normalize the mouse coordinates
		float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
		float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

		return (Vector2) {ndc_x, ndc_y};
	}

	Vector2 get_mouse_pos_in_world_space() {
		float mouse_x = input_frame.mouse_x;
		float mouse_y = input_frame.mouse_y;
		Matrix4 proj = draw_frame.projection;
		// Matrix4 view = draw_frame.view;  		// deprecated
		Matrix4 view = draw_frame.camera_xform;
		float window_w = window.width;
		float window_h = window.height;

		// Normalize the mouse coordinates
		float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
		float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

		// Transform to world coordinates
		Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
		world_pos = m4_transform(m4_inverse(proj), world_pos);
		world_pos = m4_transform(view, world_pos);
		// log("%f, %f", world_pos.x, world_pos.y);

		// Return as 2D vector
		return (Vector2){ world_pos.x, world_pos.y};
	}

	Vector2 get_mouse_pos_in_screen(){
		// just returns the mouse position in screen
		Vector2 pos = get_mouse_pos_in_ndc();
		return v2(pos.x * (0.5 * screen_width) + (0.5 * screen_width), pos.y * (0.5 * screen_height) + (0.5 * screen_height));
	}

    bool range2f_contains(Range2f range, Vector2 v) {
		return v.x >= range.min.x && v.x <= range.max.x && v.y >= range.min.y && v.y <= range.max.y;
	}

    Range2f quad_to_range(Draw_Quad quad) {
		return (Range2f) {quad.bottom_left, quad.top_right};
	}

	float v2_distance(Vector2 a, Vector2 b) {
		float dx = b.x - a.x;
		float dy = b.y - a.y;
		return sqrtf(dx * dx + dy * dy);
	}

    Draw_Quad *draw_rect_with_border(Matrix4 xform_slot, Vector2 inside_size, float border_width, Vector4 slot_col, Vector4 border_col){
		// draws a rect with borders
		// input xfrom is the base xform with no border
		// NOTE: if slot_col has alpha value of < 1, the border_color WILL push through underneath. See for yourself

			Draw_Quad q = ZERO(Draw_Quad);
			q.bottom_left  = v2(0,  0);
			q.top_left     = v2(0,  inside_size.y);
			q.top_right    = v2(inside_size.x, inside_size.y);
			q.bottom_right = v2(inside_size.x, 0);
			q.color = slot_col;
			q.image = 0;
			q.type = QUAD_TYPE_REGULAR;

			draw_rect_xform(m4_translate(xform_slot, v3(border_width * -0.5, border_width * -0.5, 0)), v2(inside_size.x + border_width, inside_size.y + border_width), border_col);
			Draw_Quad *quad = draw_quad_xform(q, xform_slot);

			return quad;
		// draw border
		// draw_rect_xform(m4_translate(xform_slot, v3(border_width * -0.5, border_width * -0.5, 0)), v2(inside_size.x + border_width, inside_size.y + border_width), border_col);
		// draw slot
		// draw_rect_xform(xform_slot, v2(inside_size.y, inside_size.y), slot_col);
    }
	

// 


// FUNCTIONS ----------------------------------------->
// ::SPRITE
    Sprite* get_sprite(SpriteID id){
        if (id <= SPRITE_nil) return &sprites[SPRITE_nil];
        else if (id > SPRITE_MAX) return &sprites[SPRITE_nil];
        else return &sprites[id];
    }

    // NOTE: this func is dumb as shit
   	SpriteID get_sprite_from_itemID(ItemID item) {
		switch (item) {

            case ITEM_rock: return SPRITE_nil; break;
            case ITEM_algae: return SPRITE_algae1; break;

			default: {log_error("Error @ 'get_sprite_from_itemID' missing case\n"); return 0;} break;
		}
	}

    Vector2 get_sprite_size(Sprite* sprite) {
        if (sprite != NULL){
            return (Vector2) {sprite->image->width, sprite->image->height};
        }
        else{
            log_error("ERROR @ get_sprite_size. sprite is NULL\n");
            return v2(0,0);
        }
    }

    Sprite* get_category_sprite(EntityID id){
		switch (id){
			case ENTITY_nil:{  return get_sprite(SPRITE_CATEGORY_all); } break;
			case ENTITY_item:{ return get_sprite(SPRITE_CATEGORY_items); } break;
			case ENTITY_tool:{ return get_sprite(SPRITE_CATEGORY_tools); } break;
			case ENTITY_workstation:{ return get_sprite(SPRITE_CATEGORY_workstations); } break;
			default: {log_error("missing case @ 'get_category_sprite'\n");} break;
		}
		log_error("Failed to get category sprite @ 'get_category_sprite'");
		return get_sprite(SPRITE_nil);
    }


	Background* get_background(BackgroundID id){
		if (id <= 0 || id >= BACKGROUND_MAX) return &backgrounds[0];
		return &backgrounds[id];
	}
	

// 

// ::ENTITY
    Entity* entity_create() {
        Entity* entity_found = 0;

        for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
            Entity* existing_entity = &world->entities[i];
            if (!existing_entity->is_valid) {
                world->entity_count++;
                entity_found = existing_entity;
                break;
            }
        }

        assert(entity_found, "No more free entities!");
        entity_found->is_valid = true;

        return entity_found;
    }

    void setup_entity(Entity* en, EntityID id){
        switch (id){
            case ENTITY_nil: break;

            case ENTITY_algae:{
                {
                    en->entity_id = id;
                    en->health = 1;
                    en->is_destroyable = true;
                    en->is_item = false;
                    en->is_selectable = true;
                    en->name = STR("Algae");
                    en->pos = v2(0, -100);
                    en->sprite = SPRITE_algae1;
                }
            } break;

            default:{
                printf("'Setup_entity' defaulted with id '%d'\n", id);
                assert(1==0, "see print above!");
            } break;
        }

        world->entities[world->entity_count++] = *en;

    }

// 

// ::PLAYER
    Vector2 get_player_pos(){
        return world->player->en->pos;
    }

    int get_player_inventory_item_count(ItemID item_id){
		for (int i = 0; i < world->player->inventory_items_count; i++){
			// InventoryItemData* item = &world->player->inventory[i];
            ItemData* item = &world->player->inventory[i];
			if (item->item_id == item_id){
				return item->amount;
			}
		}
		return 0;
    }

    int check_player_inventory_for_items(ItemID item, int count){
		// checks if the player has enough (count) of items (item) in their inventory
		// returns "true" if player has the items
		// returns "false" if player doesn't have the items

		for (int i = 0; i < world->player->inventory_items_count; i++){
			// InventoryItemData* inventory_item = &world->player->inventory[i];
            ItemData* inventory_item = &world->player->inventory[i];
			if (inventory_item->item_id == item){
				if (inventory_item->amount >= count){
					return 1;
				}
			}
		}
		return 0;
    }

	void delete_item_from_inventory(ItemID item_id, int amount){
		// printf("DELETED ITEM %d FROM INVENTORY\n", item_id);

		for (int i = 0; i < ITEM_MAX; i++){
			// InventoryItemData* inventory_item = &world->player->inventory[i];
            ItemData* inventory_item = &world->player->inventory[i];
			if (inventory_item->item_id == item_id){
				if (inventory_item->amount > 0){
					inventory_item->amount -= amount;
				}
				if (inventory_item->amount <= 0){
					// dealloc(temp_allocator, &inventory_item); // prolly does none
					world->player->inventory[i].name.count = 0;
					world->player->inventory[i].name.data = NULL;
					world->player->inventory[i].id = 0;
					world->player->inventory[i].sprite_id = 0;
					// world->player->inventory[i].tool_id = 0;
					world->player->inventory[i].item_id = 0;
					world->player->inventory[i].valid = 0;

					// Shift items down to fill the empty slot
					for (int j = i; j < ITEM_MAX - 1; j++) {
						world->player->inventory[j] = world->player->inventory[j + 1];
					}

					// Clear the last slot after shifting
					world->player->inventory[ITEM_MAX - 1].name.count = 0;
					world->player->inventory[ITEM_MAX - 1].name.data = NULL;
					world->player->inventory[ITEM_MAX - 1].id = 0;
					world->player->inventory[ITEM_MAX - 1].sprite_id = 0;
					// world->player->inventory[ITEM_MAX - 1].tool_id = 0;
					world->player->inventory[ITEM_MAX - 1].item_id = 0;
					world->player->inventory[ITEM_MAX - 1].valid = 0;

					break;
				}
				break;
			}
		}
	}
	

    void delete_recipe_items_from_inventory(ItemData recipe){
		for (int i = 0; i < recipe.crafting_recipe_count; i++){
			ItemAmount* item = &recipe.crafting_recipe[i];
			delete_item_from_inventory(item->id, item->amount);
		}
    }

// 

// ::ITEM
    string get_item_name(ItemID id){
        return STR("asd");
    }

	

// 

// :COLLISION
// Determines the orientation of the triplet (p, q, r)
// Returns 0 if collinear, 1 if clockwise, and 2 if counterclockwise
float orientation(Vector2 p, Vector2 q, Vector2 r) {
    float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (fabs(val) < 1e-6) return 0; // Collinear
    return (val > 0) ? 1 : 2;       // Clockwise or counterclockwise
}

bool on_segment(Vector2 p1, Vector2 p2, Vector2 p) {
    // Check if point p lies on the line segment from p1 to p2
    return (p.x >= fmin(p1.x, p2.x) && p.x <= fmax(p1.x, p2.x)) &&
           (p.y >= fmin(p1.y, p2.y) && p.y <= fmax(p1.y, p2.y));
}
// Function to determine if two line segments intersect
bool line_segment_intersect(Vector2 p1, Vector2 p2, Vector2 q1, Vector2 q2) {
    float o1 = orientation(p1, p2, q1);
    float o2 = orientation(p1, p2, q2);
    float o3 = orientation(q1, q2, p1);
    float o4 = orientation(q1, q2, p2);

    // General case: line segments intersect if orientations differ
    if (o1 != o2 && o3 != o4) {
        return true;
    }

    // Special cases: check for collinear points and overlap
    return (o1 == 0 && on_segment(p1, q1, p2)) ||
           (o2 == 0 && on_segment(p1, q2, p2)) ||
           (o3 == 0 && on_segment(q1, p1, q2)) ||
           (o4 == 0 && on_segment(q1, p2, q2));
}


// collision v2
bool collision(Vector2* points, int points_count, Vector2 player_pos, Vector2* wall_vector) {
    Vector2 sprite_size = get_sprite_size(get_sprite(SPRITE_player));

    Range2f player_bounds = {
        .min = player_pos,
        .max = { player_pos.x + (sprite_size.x * 2), player_pos.y + (sprite_size.y * 2)}
    };

    Vector2 bottom_left = player_bounds.min;
    Vector2 top_left = { player_bounds.min.x, player_bounds.max.y };
    Vector2 bottom_right = { player_bounds.max.x, player_bounds.min.y };
    Vector2 top_right = player_bounds.max;

    for (int i = 0; i < points_count - 1; i++) {
        Vector2 pointA = points[i];
        Vector2 pointB = points[i + 1];

        if (line_segment_intersect(bottom_left, top_left, pointA, pointB) ||
            line_segment_intersect(top_left, top_right, pointA, pointB) ||
            line_segment_intersect(top_right, bottom_right, pointA, pointB) ||
            line_segment_intersect(bottom_right, bottom_left, pointA, pointB)) {

            // Output the wall's direction vector
            if (wall_vector) {
                *wall_vector = v2_sub(pointB, pointA); // Direction of the wall
            }
            return true;
        }
    }

    return false;
}


// SETUPS -------------------------------------------->
    void setup_world(){
        world->entity_count = 0;
        // world->entities = alloc(get_heap_allocator(), sizeof(Entity) * MAX_ENTITY_COUNT);
    }

    Entity* setup_player_en(){
        Entity* en = entity_create();
        en->entity_id = ENTITY_player;
        en->name = STR("PLAYER");
        en->pos = v2(0,0);
        en->sprite = SPRITE_player;
		en->hitbox = range2f_make(v2(10, 10), v2(20, 20)); // this is wip
        return en;
    }

    void setup_player(){

        // Player* player = world->player;

        world->player = alloc(get_heap_allocator(), sizeof(Player));
        world->player->en = setup_player_en();
        world->player->health = 100;
        world->player->oxygen = 100;
        world->player->oxygen_consumption = 0.1;
        world->player->oxygen_refill = 0.2;
		world->player->walking_speed = 100;
		world->player->running_speed = 250;
		world->player->is_running = false;
    }

	Level* levels[LEVEL_MAX];

	Level* get_level(LevelID id){
		if (id <= 0 || id >= LEVEL_MAX){
			assert(1==0, "'get_level()' input id out of bounds");
			return NULL;
		}
		else{
			return levels[id];
		}
	}

	void setup_levels(){
		for (LevelID id = 0; id < LEVEL_MAX; id++){
			Level* new_level = alloc(get_heap_allocator(), sizeof(Level));
			new_level->id = id;
			new_level->level = (Gfx_Image*)load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/Levels/level_%d.png"), res_folder, id), get_heap_allocator());
			new_level->level_meta = (Gfx_Image*)load_image_from_disk(sprint(get_temporary_allocator(), STR("%s/Levels/level_%d_meta.png"), res_folder, id), get_heap_allocator());

			new_level->level_path = sprint(get_temporary_allocator(), STR("%s/Levels/level_%d.png"), res_folder, id);
			new_level->level_meta_path = sprint(get_temporary_allocator(), STR("%s/Levels/level_%d_meta.png"), res_folder, id);
	
			new_level->loaded = false;

			levels[id] = new_level;
		}
	}

// 


#endif