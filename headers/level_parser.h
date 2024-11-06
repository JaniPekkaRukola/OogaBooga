#ifndef LEVEL_PARSER_H
#define LEVEL_PARSER_H

// shit name i know

typedef struct {
    float x, y, width, height;
} CollisionBox;

typedef struct {
    float x, y;
    char* resource_type;
} ResourceSpawn;

typedef struct {
    float x, y, width, height;
    char* next_level;
} LevelTransition;

void detect_bounding_box(u8* stb_data, bool* visited, int width, int height, int start_x, int start_y, u8 r, u8 g, u8 b, int* min_x, int* max_x, int* min_y, int* max_y) {
    *min_x = start_x; *max_x = start_x;
    *min_y = start_y; *max_y = start_y;

    // Flood fill to find all connected pixels of the same color
    for (int y = start_y; y < height; y++) {
        for (int x = start_x; x < width; x++) {
            int index = y * width + x;
            if (!visited[index]) {
                u8* pixel = stb_data + index * 4;
                if (pixel[0] == r && pixel[1] == g && pixel[2] == b) {
                    visited[index] = true;
                    *min_x = fmin(*min_x, x);
                    *max_x = fmax(*max_x, x);
                    *min_y = fmin(*min_y, y);
                    *max_y = fmax(*max_y, y);
                }
            }
        }
    }
}

void parse_image(const char* filename, CollisionBox** collisions, int* num_collisions, ResourceSpawn** resources, int* num_resources, LevelTransition** transitions, int* num_transitions) {
    *num_collisions = 0;
    *num_resources = 0;
    *num_transitions = 0;

    int collision_capacity = 100;
    int resource_capacity = 100;
    int transition_capacity = 50;

    *collisions = (CollisionBox*)alloc(get_heap_allocator(), collision_capacity * sizeof(CollisionBox));
    *resources = (ResourceSpawn*)alloc(get_heap_allocator(), resource_capacity * sizeof(ResourceSpawn));
    *transitions = (LevelTransition*)alloc(get_heap_allocator(), transition_capacity * sizeof(LevelTransition));

    string png;
    bool ok = os_read_entire_file(filename, &png, get_heap_allocator());
    assert(ok, "Failed to read image");

    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    third_party_allocator = get_heap_allocator();
    u8* stb_data = stbi_load_from_memory(png.data, png.count, &width, &height, &channels, STBI_rgb_alpha);
    assert(stb_data);
    third_party_allocator = ZERO(Allocator);

    bool* visited = (bool*)alloc(get_heap_allocator(), width * height * sizeof(bool));
    memset(visited, 0, width * height * sizeof(bool));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            u8* pixel = stb_data + index * 4;

            if (visited[index]) continue;

            u8 r = pixel[0];
            u8 g = pixel[1];
            u8 b = pixel[2];

            if (r == 255 && g == 0 && b == 0) {
                int min_x, max_x, min_y, max_y;
                detect_bounding_box(stb_data, visited, width, height, x, y, 255, 0, 0, &min_x, &max_x, &min_y, &max_y);
                CollisionBox collision = { min_x, min_y, max_x - min_x + 1, max_y - min_y + 1 };
                (*collisions)[*num_collisions] = collision;
                (*num_collisions)++;
            }
            else if (r == 0 && g == 255 && b == 0) {
                int min_x, max_x, min_y, max_y;
                detect_bounding_box(stb_data, visited, width, height, x, y, 0, 255, 0, &min_x, &max_x, &min_y, &max_y);
                ResourceSpawn resource = { min_x + (max_x - min_x) / 2.0f, min_y + (max_y - min_y) / 2.0f, "mineral" };
                (*resources)[*num_resources] = resource;
                (*num_resources)++;
            }
            else if (r == 0 && g == 0 && b == 255) {
                int min_x, max_x, min_y, max_y;
                detect_bounding_box(stb_data, visited, width, height, x, y, 0, 0, 255, &min_x, &max_x, &min_y, &max_y);
                LevelTransition transition = { min_x, min_y, max_x - min_x + 1, max_y - min_y + 1, "next_level.json" };
                (*transitions)[*num_transitions] = transition;
                (*num_transitions)++;
            }
        }
    }

    // stbi_image_free(stb_data);
    dealloc(get_heap_allocator(), visited);
}



#endif