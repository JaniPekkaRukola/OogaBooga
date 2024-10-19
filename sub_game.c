// Sub_game (19.10.2024)

bool IS_DEBUG = false;

bool render_fps = true;
bool print_fps = false;


// settings
int player_walking_speed = 150;
const float screen_width = 240.0;
const float screen_height = 135.0;
#define MAX_ENTITY_COUNT 512
#define m4_identity m4_make_scale(v3(1, 1, 1))


// globals
float64 delta_t;
float64 current_time;
int fps = 0;
Vector2 camera_pos = {0, 0};
float view_zoom = 0.1875;   // view zoom ratio x (pixelart layer width / window width = 240 / 1280 = 0.1875)




// font
Gfx_Font* font;
u32 font_height = 48;


// :DataStructs ---------------------->
typedef struct WorldFrame {
    // Entity* selected_entity;
    Matrix4 world_projection;
    Matrix4 world_view;
    bool hover_consumed;
    // Entity* player;
} WorldFrame;
WorldFrame world_frame;


typedef enum EntityID {
    ENTITYID_nil,

    ENTITY_player,


    ENTITYID_MAX,
} EntityID;

typedef struct Entity{
    string name;
    EntityID entity_id;
    Vector2 pos;


    // int health;
    // bool is_destroyable;
    // bool is_selectable;
    // bool enabled;
    bool is_valid;
} Entity;


typedef struct World{
    int entity_count;
    Entity entities[MAX_ENTITY_COUNT];
    Entity* player;
    // UXState ux_state;
} World;
World* world = 0;



// FUNCTIONS --------------------->

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

void setup_world(){
    world->entity_count = 0;
    // world->entities = alloc(get_heap_allocator(), sizeof(Entity) * MAX_ENTITY_COUNT);
}

void setup_player(){
    Entity* en = entity_create();
    en->entity_id = ENTITY_player;
    en->name = STR("PLAYER");
    en->pos = v2(0,0);
    world->player = en;
}

void render_background(){
    Matrix4 xform = m4_identity;
    xform = m4_translate(xform, v3(0, 0, 0));
    draw_rect_xform(xform, v2(screen_width, screen_height), v4(0, 0.3, 1, 1));
}


void render_entities(){

    for (int i = 0; i < MAX_ENTITY_COUNT; i++){
        Entity* en = &world->entities[i];

        if (en && en->is_valid){
            switch (en->entity_id){

                case ENTITY_player:{
                    {
                        Matrix4 xform = m4_identity;
                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
                        draw_rect_xform(xform, v2(20, 20), COLOR_BLACK);
                    }
                } break;

                default:{
                    {
                        Matrix4 xform = m4_identity;
                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
                        draw_rect_xform(xform, v2(10, 10), COLOR_BLACK);
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

    window.clear_color = v4(1, 1, 1, 1);
    window.force_topmost = false;
    window.allow_resize = false;

    // init world
	world = alloc(get_heap_allocator(), sizeof(World));
	memset(world, 0, sizeof(World));

    // Font
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	render_atlas_if_not_yet_rendered(font, font_height, 'A'); // fix for the stuttering bug for first time text rendering courtesy of charlie (Q&A #3)


    setup_world();
    setup_player();


    // Timing
	float64 seconds_counter = 0.0;
	s32 frame_count = 0;
	float64 last_time = os_get_elapsed_seconds();


    while (!window.should_close){

		reset_temporary_storage();

		// :Timing
		float64 current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;
		last_time = current_time;
		os_update();

        world_frame = (WorldFrame){0};

        // :Frame :update
		draw_frame.enable_z_sorting = true;
		world_frame.world_projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

        // camera
        Vector2 target_pos = world->player->pos;
        animate_v2_to_target(&camera_pos, target_pos, delta_t, 10.0f); // 4th value controls how smooth the camera transition is to the player (lower = slower)

        world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
        world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
        world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));



        set_world_space();










        render_background();

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

		// if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->running_speed_amount * delta_t)); }
		// else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, world->player->walking_speed * delta_t)); }
        world->player->pos = v2_add(world->player->pos, v2_mulf(input_axis, player_walking_speed* delta_t));


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