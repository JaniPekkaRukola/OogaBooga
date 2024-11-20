#define MAX_POINTS 100

bool selecting_resource = false;

typedef struct LevelEditor{
    Vector2 points[MAX_POINTS];
    int point_count;
    int selected_point_index;  //for dragging points
    float zoom;
    // bool point_selected; // true if point is currently selected
    SpriteID selected_resource;
} LevelEditor;


Vector2 screen_drag_start = {0, 0};

typedef enum ResourceID{
    RESOURCE_nil,

    RESOURCE_algae,
    RESOURCE_rock,
    RESOURCE_pearl,

    RESOURCE_MAX,
} ResourceID;


void save_points_to_file(Vector2* points, int point_count, string path, string filename) {
    // Full file path
    string full_path = sprint(get_heap_allocator(), STR("%s%s"), path, filename);
    // bool to_continue = false;

    if (os_is_file(full_path) && !is_key_down(KEY_CTRL)){
        
        printf("Savefile exists... Nothing written\n");
        printf("press ctrl + mouse1 to overwrite\n");
        // draw_text_xform(font, sprint(get_heap_allocator(), STR("File '%s' already exists. Wanna overwrite?"), filename), font_height, m4_translate(m4_identity, v3(screen_width * 0.5, screen_height * 0.5, 0)), v2(0.2, 0.2), COLOR_RED);

        // Draw_Quad* yes = draw_rect_xform(m4_translate(m4_identity, v3(screen_width * 0.5 - 10, screen_height * 0.5 - 10, 0)), v2(5, 5), COLOR_GREEN);
        // Draw_Quad* no = draw_rect_xform(m4_translate(m4_identity, v3(screen_width * 0.5 + 10, screen_height * 0.5 - 10, 0)), v2(5, 5), COLOR_RED);



        // Vector2 mouse_pos = get_mouse_pos_in_ndc();

        // if (range2f_contains(quad_to_range(*yes), mouse_pos)){
        //     if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
        //         consume_key_just_pressed(MOUSE_BUTTON_LEFT);
        //         to_continue = true;
        //     }
        // }
        // if (range2f_contains(quad_to_range(*no), mouse_pos)){
        //     if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
        //         consume_key_just_pressed(MOUSE_BUTTON_LEFT);
        //         printf("file not saved\n");
        //         return;
        //     }
        // }
        return;
    }
    // Allocate a temporary buffer for the JSON data
    string json_data = STR("{\n\t\"points\": [\n");

    // Append each point to the JSON data
    for (int i = 0; i < point_count; ++i) {
        json_data = sprint(get_heap_allocator(), STR("%s\t\t{\"x\": %.2f, \"y\": %.2f}%s\n"), json_data, points[i].x, points[i].y, (i < point_count - 1) ? "," : "");
    }

    // Finalize the JSON string
    json_data = sprint(get_heap_allocator(), STR("%s\t]\n}\n"), json_data);


    // Write JSON data to file
    bool file_written = os_write_entire_file(full_path, json_data);
    assert(file_written, "Failed to write to file");

    printf("Successfully saved points to a file\n");

    // Optionally, free any allocated memory here if your allocator requires it
}

void init_level_editor(LevelEditor* editor) {
    editor->point_count = 0;
    editor->selected_point_index = -1;
    editor->zoom = 1.0f;
}

// void init_level_editor_with_level(LevelEditor* editor, int level){
//     int max_points = 100;
//     bool suc = load_points_from_file((Vector2*)editor->points, max_points, STR("res\\Abyssophobia\\Levels\\"), sprint(get_heap_allocator(), STR("level_%d_meta.txt"), level), get_heap_allocator());
//     assert(suc, "failed");
// }

void add_point(LevelEditor* editor, Vector2 pos) {
    if (editor->point_count < MAX_POINTS) {
        editor->points[editor->point_count++] = pos;
    }
}

void render_level_editor(LevelEditor* editor) {

    set_world_space();

    Vector2 point_size = v2(5, 5);

    for (int i = 0; i < editor->point_count; ++i) {
        Vector2 point = editor->points[i];
        

        draw_circle(v2(point.x - (point_size.x * 0.5), point.y - (point_size.y * 0.5)), point_size, (i == editor->selected_point_index) ? COLOR_YELLOW : COLOR_RED);
        
        if (i > 0) {
            draw_line(editor->points[i - 1], point, 1, COLOR_WHITE);  // draw lines between points
        }
    }

    set_screen_space();

    Vector2 button_size = v2(5, 5);
    int margin_top = screen_height;

    // points save button
    {
        Matrix4 save_button = m4_identity;
        save_button = m4_translate(save_button, v3(0, margin_top -= 10, 0));
        Draw_Quad* save_quad = draw_rect_xform(save_button, button_size, COLOR_GREEN);
        draw_text_xform(font, STR("Save"), font_height, m4_translate(save_button, v3(10, 0, 0)), v2(0.1, 0.1), COLOR_BLACK);

        if (range2f_contains(quad_to_range(*save_quad), get_mouse_pos_in_ndc())){
            if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                consume_key_just_pressed(MOUSE_BUTTON_LEFT);
                printf("Saving to a file...\n");
                save_points_to_file(editor->points, editor->point_count, STR("res\\abyssophobia\\Levels\\"), STR("level_1_meta.txt"));
            }
        }
    }

    // points reset button
    {
        Matrix4 xform = m4_identity;
        xform = m4_translate(xform, v3(0, margin_top -= 10, 0));
        Draw_Quad* quad = draw_rect_xform(xform, button_size, COLOR_RED);
        draw_text_xform(font, STR("Reset points (ctrl + mouse1)"), font_height, m4_translate(xform, v3(10, 0, 0)), v2(0.1, 0.1), COLOR_BLACK);

        if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())){
            if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                consume_key_just_pressed(MOUSE_BUTTON_LEFT);

                if (is_key_down(KEY_CTRL)){
                    printf("Resetting...\n");
                    for (int i = 0; i < editor->point_count; i++){
                        editor->points[i] = v2(0, 0);
                    }
                    editor->point_count = 0;
                }
            }
        }
    }

    // portal button
    {
        Matrix4 xform = m4_identity;
        xform = m4_translate(xform, v3(0, margin_top -= 15, 0));
        Draw_Quad* quad = draw_rect_xform(xform, button_size, COLOR_BLUE);
        draw_text_xform(font, STR("Portal"), font_height, m4_translate(xform, v3(10, 0, 0)), v2(0.1, 0.1), COLOR_BLACK);


        if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())){
            if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                consume_key_just_pressed(MOUSE_BUTTON_LEFT);
                printf("Selected portal\n");
            }
        }
    }

    // select resource button
    {
        Matrix4 xform = m4_identity;
        xform = m4_translate(xform, v3(0, margin_top -= 15, 0));
        Draw_Quad* quad = draw_rect_xform(xform, button_size, COLOR_GREEN);
        draw_text_xform(font, STR("Select resource"), font_height, m4_translate(xform, v3(10, 0, 0)), v2(0.1, 0.1), COLOR_BLACK);

        if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())){
            if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                consume_key_just_pressed(MOUSE_BUTTON_LEFT);

                printf("Selecting resource\n");
                selecting_resource = !selecting_resource;
            }
        }
    }

    // resource selection
    if (selecting_resource)
    {
        Vector2 bg_size = v2(RESOURCE_MAX * 10, 20);
        Matrix4 bg = m4_identity;
        bg = m4_translate(bg, v3(10, margin_top - (bg_size.y), 0));
        draw_rect_xform(bg, bg_size, v4(0.5, 0.5, 0.5, 1));

        int y_start = margin_top - bg_size.y;
        int x_start = 10;
        Vector2 slot_size = v2(10, 10);

        // draw resource icons
        for (SpriteID id = 1; id < SPRITE_MAX; id++){
            Matrix4 xform = m4_identity;
            xform = m4_translate(xform, v3(x_start + (id * slot_size.x), y_start, 0));
            Sprite* sprite = get_sprite(id);
            Draw_Quad* quad = draw_image_xform(sprite->image, xform, slot_size, COLOR_WHITE);

            if (range2f_contains(quad_to_range(*quad), get_mouse_pos_in_ndc())){
                if (is_key_just_pressed(MOUSE_BUTTON_LEFT)){
                    consume_key_just_pressed(MOUSE_BUTTON_LEFT);
                    editor->selected_resource = id;
                    printf("Selected %d\n", id);
                }
            }
        }

    }


}



void handle_editor_input(LevelEditor* editor, float delta_t) {
    set_world_space();

    Vector2 mouse_pos = get_mouse_pos_in_world_space();
    float point_selection_dis = 20.0f;

    if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
        consume_key_just_pressed(MOUSE_BUTTON_LEFT);
        int point_index = -1;

        // Check if a point is already selected for dragging
        for (int i = 0; i < editor->point_count; ++i) {
            if (v2_distance(editor->points[i], mouse_pos) < point_selection_dis) {
                editor->selected_point_index = i;
                point_index = i;
                break;
            }
        }

        // printf("selected point index = %d\n", editor->selected_point_index);

        if (point_index == -1 && editor->selected_point_index == -1) {  // Add a new point if none selected
            add_point(editor, mouse_pos);
            printf("Added point\n");
        }
    }

    // Release point on right-click
    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
        consume_key_just_pressed(MOUSE_BUTTON_RIGHT);
        editor->selected_point_index = -1;
    }

    // Dragging the selected point
    if (editor->selected_point_index != -1 && is_key_down(MOUSE_BUTTON_LEFT)) {
        editor->points[editor->selected_point_index] = mouse_pos;
    }

    if (is_key_just_pressed(MOUSE_BUTTON_MIDDLE)){
        screen_drag_start = get_mouse_pos_in_screen();
        printf("pressed\n");
    }

    else if (is_key_down(MOUSE_BUTTON_MIDDLE)){
        printf("down\t drag start = %f, %f\n", screen_drag_start.x, screen_drag_start.y);
        // Vector2 mouse_pos_ndc = get_mouse_pos_in_ndc();
        // Vector2 mouse_pos_inverted = mouse_pos_ndc;
        // mouse_pos_inverted.x *= -1;
        // mouse_pos_inverted.y *= -1;


        // Vector2 start_mouse = get_mouse_pos_in_world_space();

        // Vector2 start_player = world->player->en->pos;

        world->player->en->pos = screen_drag_start;

    }
    if (is_key_just_released(MOUSE_BUTTON_MIDDLE)){
        screen_drag_start = v2(0, 0);
        printf("Release\n");
    }

    // zooom
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

        float running_speed = 400;
        float walking_speed = 150;


        if (world->player->is_running){ world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, running_speed * delta_t)); }
        else { world->player->en->pos = v2_add(world->player->en->pos, v2_mulf(input_axis, walking_speed * delta_t)); }

}

// @ghost_points
bool load_points_from_file(Vector2* points, int max_points, string path, string filename, Allocator allocator) {
    // Full file path
    string full_path = sprint(allocator, STR("%s%s"), path, filename);

    // Read file content into result string
    string file_data;
    if (!os_read_entire_file(full_path, &file_data, allocator)) {
        assert(false, "Failed to read file");
        return false;
    }

    // Initialize variables for parsing
    int point_count = 0;
    // char* current = file_data.data;  // Access raw char data from the string
    char* current = (char*)file_data.data;

    // Parse each point manually
    while (point_count < max_points && (current = strstr(current, "\"x\":"))) {
        points[point_count].x = strtof(current + 4, &current); // Parse x coordinate
        current = strstr(current, "\"y\":");
        if (!current) break; // If y-coordinate is missing, break

        points[point_count].y = strtof(current + 4, &current); // Parse y coordinate
        point_count++;
    }

    int pause = 1;

    return true;
}