// AbyssoPhobia (19.10.2024)

#include "headers/masterheader.h"


void render_background(){
    // Matrix4 xform = m4_identity;
    // xform = m4_translate(xform, v3(0, 0, 0));
    // draw_rect_xform(xform, v2(screen_width, screen_height), v4(0, 0.3, 1, 1));

    Vector2 player_pos = get_player_pos();
    Vector2 size = v2(800, 500);

    draw_rect(v2(player_pos.x - (size.x * 0.5), -size.y), size, v4(0, 0.3, 0.8, 1));
}

void render_surfaceLine(){
    Vector2 pos = get_player_pos();
    int size_x = screen_width * 3;
    draw_rect(v2(pos.x-(size_x*0.5), 0), v2(size_x, 1), COLOR_RED);
}

void render_ui(){

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
                        draw_image_xform(sprite->image, xform, get_sprite_size(sprite), COLOR_WHITE);
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
    window.x = window.point_height * 0.5 - 100;

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
    sprites[SPRITE_nil] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/sprites/nil.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_player] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/sprites/player.png"), res_folder)   ,get_heap_allocator())};
    sprites[SPRITE_algae1] = (Sprite){.image=load_image_from_disk(  sprint(get_temporary_allocator(), STR("%s/sprites/algae1.png"), res_folder)   ,get_heap_allocator())};




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

    setup_world();
    setup_player();


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

    world->ux_state = UX_mainmenu;


    while (!window.should_close){

		reset_temporary_storage();

		// :Timing
		float64 current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;
		last_time = current_time;
		os_update();

        if (world->ux_state == UX_mainmenu){

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
                    world->ux_state = UX_gameplay;
					
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



        if (world->ux_state == UX_gameplay){


            world_frame = (WorldFrame){0};

            // :Frame :update
            draw_frame.enable_z_sorting = true;
            world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

            // camera
            Vector2 target_pos = world->player->player_en->pos;
            animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

            world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
            world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));



            set_world_space();


            // oxygen
            if (world->player->player_en->pos.y < 0){
                world->player->oxygen -= world->player->oxygen_consumption;
                world->player->oxygen = clamp(world->player->oxygen, 0, 100);
            }
            else{
                world->player->oxygen += world->player->oxygen_refill;
                world->player->oxygen = clamp(world->player->oxygen, 0, 100);
            }









            render_background();
            render_surfaceLine();
            render_ui();

            render_entities();

            if (render_fps){
                set_screen_space();
                draw_text(font, sprint(get_heap_allocator(), STR("%d"), fps), font_height, v2(0,screen_height-3), v2(0.1, 0.1), COLOR_RED);
                set_world_space();
            }


            // printf("Player pos = %.0f, %.0f\n", world->player->pos.x, world->player->pos.y);



            // :Input ------------------------------------------------------------------------------------>
            if (is_key_just_pressed('X')){ // EXIT
                window.should_close = true;
            }

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

            // if (world->player->is_running){ worldx->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed_amount * delta_t)); }
            // else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }
            world->player->player_en->pos = v2_add(world->player->player_en->pos, v2_mulf(input_axis, player_walking_speed* delta_t));

            world->player->player_en->pos.y = clamp(world->player->player_en->pos.y, -100000, 0);

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