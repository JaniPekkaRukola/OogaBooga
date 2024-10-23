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

// 


// FUNCTIONS ----------------------------------------->
// ::SPRITE
    Sprite* get_sprite(SpriteID id){
        if (id <= SPRITE_nil) return &sprites[SPRITE_nil];
        else if (id > SPRITE_MAX) return &sprites[SPRITE_nil];
        else return &sprites[id];
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
        return world->player->player_en->pos;
    }

// 


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
        return en;
    }

    void setup_player(){

        // Player* player = world->player;

        world->player = alloc(get_heap_allocator(), sizeof(Player));
        world->player->player_en = setup_player_en();
        world->player->health = 100;
        world->player->oxygen = 100;
        world->player->oxygen_consumption = 0.1;
        world->player->oxygen_refill = 0.2;
    }

// 


#endif