#define MAX_POINTS 100

typedef struct {
    Vector2 points[MAX_POINTS];
    int point_count;
    int selected_point_index;  //for dragging points
    float zoom;
} LevelEditor;


float v2_distance(Vector2 a, Vector2 b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx * dx + dy * dy);
}

void init_level_editor(LevelEditor* editor) {
    editor->point_count = 0;
    editor->selected_point_index = -1;
    editor->zoom = 1.0f;
}

void add_point(LevelEditor* editor, Vector2 pos) {
    if (editor->point_count < MAX_POINTS) {
        editor->points[editor->point_count++] = pos;
    }
}

void render_level_editor(LevelEditor* editor) {
    Vector2 point_size = v2(5, 5);

    for (int i = 0; i < editor->point_count; ++i) {
        Vector2 point = editor->points[i];
        draw_circle(v2(point.x - (point_size.x * 0.5), point.y - (point_size.y * 0.5)), point_size, (i == editor->selected_point_index) ? COLOR_YELLOW : COLOR_RED);
        
        if (i > 0) {
            draw_line(editor->points[i - 1], point, 1, COLOR_WHITE);  // draw lines between points
        }
    }
}

void handle_editor_input(LevelEditor* editor, float delta_t) {
    Vector2 mouse_pos = get_mouse_pos_in_world_space();

    if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
        consume_key_just_pressed(MOUSE_BUTTON_LEFT);
        int point_index = -1;

        // Check if a point is already selected for dragging
        for (int i = 0; i < editor->point_count; ++i) {
            if (v2_distance(editor->points[i], mouse_pos) < 10.0f) {
                editor->selected_point_index = i;
                point_index = i;
                break;
            }
        }

        if (point_index == -1) {  // Add a new point if none selected
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
}
