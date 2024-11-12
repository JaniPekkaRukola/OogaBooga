#ifndef LEVEL_META_PARSER_H
#define LEVEL_META_PARSER_H

#define WIDTH 2000            // Level image width
#define HEIGHT 2000           // Level image height
#define MAX_SEGMENTS 10    // Max number of segments for collision boundaries


Level load_level(LevelID level_id);
void change_level(Level* dest);

void convertMetaImageToSegments(unsigned char* image, Segment* segments, int* segment_count);
bool isRedPixel(unsigned char* image, int x, int y);
void tracePath(unsigned char* image, bool* visited, int x, int y, Point* path, int* path_length);



void change_level(Level* dest) {
    Level loaded = load_level(dest->id);
    world->level = loaded;
    printf("Level changed to '%d' with %d segments\n", loaded.id, loaded.collision_boundaries);
}

Segment trace_boundary(u8* data, int width, int height, int start_x, int start_y, bool* visited) {
    // Starting point of the segment
    Point start = {start_x, start_y};
    visited[start_y * width + start_x] = true;

    // Move right until boundary or end of red pixel streak
    int end_x = start_x;
    while (end_x < width) {
        int index = (start_y * width + end_x) * 4;
        u8 r = data[index];
        u8 g = data[index + 1];
        u8 b = data[index + 2];

        if (r != 255 || g != 0 || b != 0 || visited[start_y * width + end_x]) {
            break;
        }
        visited[start_y * width + end_x] = true;
        end_x++;
    }

    // Ending point of the segment
    Point end = {end_x - 1, start_y};
    return (Segment){start, end};
}

Segment* extract_collision_boundaries(u8* data, int width, int height) {
    bool* visited = (bool*)alloc(get_heap_allocator(), width * height * sizeof(bool));
    memset(visited, 0, width * height * sizeof(bool));

    // Segment* segments = (Segment*)alloc(get_heap_allocator(), MAX_SEGMENTS);
    Segment* segments = (Segment*)alloc(get_heap_allocator(), MAX_SEGMENTS * sizeof(Segment));
    int segment_count = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {


            int index = (y * width + x) * 4;  // Assuming RGBA format
            u8 r = data[index];
            u8 g = data[index + 1];
            u8 b = data[index + 2];

            if (r == 255 && g == 0 && b == 0 && !visited[y * width + x]) {
                // Trace a boundary starting at this pixel
                Segment boundary_segment = trace_boundary(data, width, height, x, y, visited);
                segments[segment_count++] = boundary_segment;
            }
        }
        if (segment_count >= MAX_SEGMENTS){
            printf("MAX SEGMENTS REACHED\n");
            break;
        }
    }

    dealloc(get_heap_allocator(), visited);
    return segments;
}



// Check if a pixel is red (assumes 4 bytes per pixel: RGBA)
bool isRedPixel(unsigned char* image, int x, int y) {
    int index = (y * WIDTH + x) * 4;
    return image[index] == 255 && image[index + 1] == 0 && image[index + 2] == 0;
}

// Trace the path of connected red pixels, forming a segment path
void tracePath(unsigned char* image, bool* visited, int x, int y, Point* path, int* path_length) {
    int index = 0;
    // DFS or BFS traversal could be used here; for simplicity, adding pixels directly to the path
    // Only demonstrate one step to avoid extensive code here
    path[index++] = (Point) {x, y};
    visited[y * WIDTH + x] = true;
    *path_length = index;
}

// Convert the meta image's red pixels into line segments for collision
void convertMetaImageToSegments(unsigned char* image, Segment* segments, int* segment_count) {
    bool visited[WIDTH * HEIGHT] = {false};
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (isRedPixel(image, x, y) && !visited[y * WIDTH + x]) {
                Point path[1000];
                int path_length = 0;

                tracePath(image, visited, x, y, path, &path_length);

                for (int i = 0; i < path_length - 1; i++) {
                    if (*segment_count < MAX_SEGMENTS) {
                        segments[(*segment_count)++] = (Segment){path[i], path[i + 1]};
                    }
                }
            }
        }
    }
}

Level load_level(LevelID level_id) {
    printf("Loading level %d\n", level_id);

    Level* level = alloc(get_heap_allocator(), sizeof(Level));
    if (level_id <= 0 || level_id >= LEVEL_MAX) {
        log_error("Failed to load level for %d\n", level_id);
        return *level;
    }

    string path, path_meta;
    switch (level_id) {
        case LEVEL_1:
            path = sprint(get_temporary_allocator(), STR("%s/Levels/level_1.png"), res_folder);
            path_meta = sprint(get_temporary_allocator(), STR("%s/Levels/level_1_meta.png"), res_folder);
            break;
        default:
            assert(1 == 0, "switch defaulted @ load_level");
    }

    Gfx_Image* level_image = load_image_from_disk(path, get_heap_allocator());
    if (!level_image) {
        log_error("Failed to load main level image for %d\n", level_id);
        return *level;
    }

    // Load meta image for collision boundaries
    string png;
    bool ok = os_read_entire_file(path_meta, &png, get_heap_allocator());
    assert(ok, "Failed to read meta image file");

    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    third_party_allocator = get_heap_allocator();
    u8* stb_data = stbi_load_from_memory(png.data, png.count, &width, &height, &channels, STBI_rgb_alpha);
    assert(stb_data);

    // Extract collision boundaries from red pixels
    Segment* collision_boundaries = extract_collision_boundaries(stb_data, width, height);
    level->collision_boundaries = collision_boundaries;
    // level->boundary_count = /* however many segments were extracted */;

    // Clean up and finalize level loading
    third_party_allocator = ZERO(Allocator);
    level->id = level_id;
    level->level = level_image;

    printf("Successfully loaded level %d\n", level_id);
    return *level;
}

#endif
