#ifndef CHUNKS2_H
#define CHUNKS2_H


#define CHUNK_RENDER_DISTANCE 1 // how many chunks are loaded into each direction from center. value of 2 is 25 chunks total (5x5). value of 1 is 9 chunks total (3x3) etc. NOTE: atleast with only 1 tree type rendering the total fps drop from values 1 to 2 is about -10%

// chunk offsets. These must be used to store negative values. So now the (0.0) chunk is in the middle of the chunks array
#define CHUNK_OFFSET_X WORLD_WIDTH / 2
#define CHUNK_OFFSET_Y WORLD_HEIGHT / 2

int total_entity_count = 0;

// moved this into types.h
// typedef struct Chunk {
//     BiomeID biome_id;
//     Vector2 pos_in_world;
//     Vector2 pos_in_grid;
//     Vector2 size;
//     int entity_count;
//     Entity entities[MAX_CHUNK_ENTITIES];

//     bool has_been_loaded;
// } Chunk;


void initialize_chunks(DimensionData* dimension) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            dimension->chunks[x][y] = NULL;
        }
    }
    printf("Initialized chunks for DimensionID = %d\n", dimension->id);
}

Chunk* get_chunk(DimensionData* dimension, Vector2 pos) {
    // pos is position in the chunks array
    int x = (int)pos.x + CHUNK_OFFSET_X;
    int y = (int)pos.y + CHUNK_OFFSET_Y;
    return dimension->chunks[x][y];
}

Chunk* get_player_chunk(){
    Vector2 player_pos = get_player_pos();
    int x = (int)(player_pos.x / CHUNK_SIZE) + CHUNK_OFFSET_X;
    int y = (int)(player_pos.y / CHUNK_SIZE) + CHUNK_OFFSET_Y;

    // fix for negative coords
    if (player_pos.x < 0) x -= 1;
    if (player_pos.y < 0) y -= 1;

    return world->dimension->chunks[x][y];
}

Vector2 get_chunk_world_position(int x, int y) {
    // Subtract the chunk offset and multiply by chunk size to get world coordinates
    float world_x = (x - CHUNK_OFFSET_X) * CHUNK_SIZE;
    float world_y = (y - CHUNK_OFFSET_Y) * CHUNK_SIZE;

    // Return the world position as a Vector2
    return v2(world_x, world_y);
}

Vector2 get_chunk_world_position_v2(Vector2 v){
    return get_chunk_world_position(v.x, v.y);
}


Entity* entity_create_to_chunk(Chunk* chunk) {

    // This function is basically the same as "entity_create"

    assert(chunk, "Failed to create entity to chunk, Chunk is NULL");

    Entity* entity_found = 0;

    int asd_x = chunk->pos_in_grid.x + CHUNK_OFFSET_X;
    int asd_y = chunk->pos_in_grid.y + CHUNK_OFFSET_Y;

    for (int i = 0; i < MAX_CHUNK_ENTITIES; i++){
        Entity* existing_entity = &chunk->entities[i];

        // if (existing_entity == NULL){
        //     // printf("EXISTING ENTITY = NULL! Allocating memory...\n");
        //     existing_entity = alloc(get_heap_allocator(), sizeof(Entity));
        // }

        if (!existing_entity->is_valid){
            chunk->entity_count++;
            total_entity_count++;
            chunk->has_been_loaded = true;
            entity_found = existing_entity;
            break;
        }
    }

    // printf("chunk entity count = %d, biomeid = %d\n", chunk->entity_count, chunk->biome_id);
    assert(entity_found, "No more free entities!");
    entity_found->is_valid = true;

    return entity_found;
}

// void create_tree_to_chunk(Chunk* chunk, int amount, Range2f range){
    //     Vector2 positions[amount];

    //     for (int i = 0; i < amount; i++){
    //         float x = get_random_float32_in_range(range.min.x, range.max.x);
    //         float y = get_random_float32_in_range(range.min.y, range.max.y);
    //         positions[i] = v2(x,y);
    //     }

    //     for (int i = 0; i < amount; i++){
    //         Entity* en = entity_create_to_chunk(chunk);
    //         // setup tree
    //         setup_pine_tree(en);
    //         // en->pos = v2(positions[i].x, positions[i].y);
    //         en->pos = v2(positions[i].x + chunk->pos_in_world.x, positions[i].y + chunk->pos_in_world.y);
    //         en->pos = round_v2_to_tile(en->pos);
    //     }


    //     // free the list of positions
    // 	memset(positions, 0, sizeof(positions));
    //     // dealloc(get_heap_allocator(), (Vector2*)&positions);

    //     int res_x = chunk->pos_in_grid.x + CHUNK_OFFSET_X;
    //     int res_y = chunk->pos_in_grid.y + CHUNK_OFFSET_Y;

    //     printf("Created %d trees to chunk[%d][%d]\n", amount, res_x, res_y);
// }


// void create_chunk_entities(Chunk* chunk, EntityArchetype arch, int amount, Range2f range){
    //     Vector2 entity_positions[amount];

    //     // printf("CREATING %d ENTITITES '%d' to chunk\n", amount, arch);
    //     // printf("Chunk boundaries = (%.0f, %.0f) (%.0f,%.0f)\n\n", range.min.x, range.min.y, range.max.x, range.max.y);

    //     BiomeData biome = get_biome_data_from_id(chunk->biome_id);

    //     // Generate random positions for entities
    //     for (int i = 0; i < amount; i++) {
    //         float x = get_random_float32_in_range(range.min.x, range.max.x);
    //         float y = get_random_float32_in_range(range.min.y, range.max.y);
    //         entity_positions[i] = v2(x, y);
    //     }

    //     // Create entities and set them up based on the archetype
    //     for (int i = 0; i < amount; i++) {
    //         Entity* en = entity_create_to_chunk(chunk);
    //         switch (arch) {
    //             case ARCH_rock:
    //                 setup_rock(en, ROCK_normal_medium);
    //                 break;
    //             case ARCH_tree:
    //                 // if (biome.spawn_pine_trees) setup_pine_tree(en);
    //                 // if (biome.spawn_spruce_trees) setup_spruce_tree(en);
    //                 // if (biome.spawn_magical_trees) setup_magical_tree(en);
    //                 // more tree types here? or separate?
    //                 break;
    //             case ARCH_bush:
    //                 setup_bush(en);
    //                 break;
    //             case ARCH_twig:
    //                 setup_twig(en);
    //                 break;
    //             case ARCH_ore:
    //                 // setup_ore(en, biome); // Adjust this for ore-specific setup's
    //                 break;
    //             case ARCH_mushroom:
    //                 setup_mushroom(en);
    //                 break;

    //             // more cases pls :)

    //             default:
    //                 break;
    //         }

    //         en->pos = v2(entity_positions[i].x + chunk->pos_in_world.x, entity_positions[i].y + chunk->pos_in_world.y);
    //         en->pos = round_v2_to_tile(en->pos);

    //         // Avoid spawning multiple entities in the same position
    //         for (int j = 0; j < i; j++) {
    //             if (entity_positions[i].x == entity_positions[j].x && entity_positions[i].y == entity_positions[j].y) {
    //                 // Handle collision and reposition entity if needed
    //             }
    //         }
    //     }

    //     memset(entity_positions, 0, sizeof(entity_positions));
// }


void spawn_chunk_entities(Chunk* chunk){

    if (chunk->biome_id == BIOME_nil) return;

    // chunks pos in worldspace
    Vector2 chunk_pos_in_world;
    chunk_pos_in_world.x = (chunk->pos_in_grid.x - CHUNK_OFFSET_X) / CHUNK_SIZE;
    chunk_pos_in_world.y = (chunk->pos_in_grid.y - CHUNK_OFFSET_Y) / CHUNK_SIZE;

    // chunk boundaries in worldspace
    Range2f chunk_boundaries;
    chunk_boundaries.min.x = chunk_pos_in_world.x;
    chunk_boundaries.min.y = chunk_pos_in_world.y;
    chunk_boundaries.max.x = chunk_pos_in_world.x + CHUNK_SIZE;
    chunk_boundaries.max.y = chunk_pos_in_world.y + CHUNK_SIZE;

    BiomeData d = get_biome_data_from_id(chunk->biome_id);

    // Generate random positions for entities
    int entity_count = 0;

    for (int k = 0; k < d.spawn_table.entity_count; k++) {
        entity_count += d.spawn_table.entities[k].weight;
    }

    Vector2 entity_positions[entity_count];
    for (int i = 0; i < entity_count; i++) {
        float x = get_random_float32_in_range(chunk_boundaries.min.x, chunk_boundaries.max.x);
        float y = get_random_float32_in_range(chunk_boundaries.min.y, chunk_boundaries.max.y);
        entity_positions[i] = v2(x, y);
    }

    int entity_pos_index = 0;

    for (int i = 0; i < d.spawn_table.entity_count; i++){
        Spawnable spawnable = d.spawn_table.entities[i];
        if (spawnable.arch == ARCH_nil) continue;
        
        for (int weight = 0; weight < spawnable.weight; weight++){

            Entity* en = entity_create_to_chunk(chunk);
    
            switch (spawnable.arch){
                case ARCH_tree: setup_tree(en, spawnable.tree_type); break;
                case ARCH_rock: setup_rock(en, spawnable.rock_type); break;
                case ARCH_foliage: setup_foliage(en, spawnable.foliage_type); break;
                case ARCH_ore: setup_ore(en, spawnable.ore_type); break;
                default: log_error("Missing case @ 'spawn_chunk_entities'"); break;
            }
            en->pos = v2(entity_positions[entity_pos_index].x + chunk->pos_in_world.x, entity_positions[entity_pos_index].y + chunk->pos_in_world.y);
            // en->pos.x -= get_sprite(en->sprite_id)->image->width * 0.5;

            if (spawnable.color_adj){
                en->col_adj = true;
                en->col_adj_val = spawnable.color_adj_val;
            }
            entity_pos_index++;
        }
    }

    memset(entity_positions, 0, sizeof(entity_positions));
}

void load_chunk(DimensionData* dimension, Vector2 pos) {
    // NOTE: the "CHUNK_OFFSET" should already be applied to par "pos", when calling this func
    int x = pos.x;
    int y = pos.y;

    if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
        Chunk* chunk = dimension->chunks[x][y];
        if (chunk == NULL) {
            chunk = alloc(get_heap_allocator(), sizeof(Chunk));
            chunk->pos_in_grid = pos;
            chunk->entity_count = 0;
            chunk->biome_id = biome_at_position(x,y);
            chunk->pos_in_world = get_chunk_world_position(x, y);
            world->dimension->chunk_count++;
            world->dimension->chunks[x][y] = chunk;

            printf("Loaded a chunk[%d][%d]\n", x, y);

            ENTITIES_NEED_SORTING = true;

            if (chunk->biome_id != BIOME_nil){
                spawn_chunk_entities(dimension->chunks[x][y]);
            }
            else{
                printf("Failed to spawn chunk entities. chunk biomeID = BIOME_NIL\n");
            }
        }
    }
}

void load_chunk_with_offset(DimensionData* dimension, Vector2 pos){
    pos.x += CHUNK_OFFSET_X;
    pos.y += CHUNK_OFFSET_Y;
    load_chunk(dimension, pos);
}

void unload_chunk(DimensionData* dimension, Vector2 pos) {
    // NOTE: the "CHUNK_OFFSET" should already be applied to par "pos", when calling this func
    int x = pos.x;
    int y = pos.y;

    // TODO: does the entities list also need to be freed???

    if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
        if (dimension->chunks[x][y] != NULL) {

            if (dimension->chunks[x][y]->has_been_modified == true){
                // save chunk to disc
            }

            else{ // this else is temp. remove it when saving chunks is working
                total_entity_count -= dimension->chunks[x][y]->entity_count;
                dealloc(get_heap_allocator(), dimension->chunks[x][y]);
                dimension->chunks[x][y] = NULL;
                world->dimension->chunk_count--;

                printf(" UN-Loaded chunk[%d][%d]\n", x, y);
            }
        }
    }
}

void load_chunks_renderdistance() {

    DimensionData* dimension = world->dimension;

    Vector2 player_pos = get_player_pos();

    // Convert player world position to chunk coordinates
    int player_chunk_x = (int)(player_pos.x / CHUNK_SIZE);
    int player_chunk_y = (int)(player_pos.y / CHUNK_SIZE);

    // Iterate over the chunks within the render distance
    for (int dx = -CHUNK_RENDER_DISTANCE; dx <= CHUNK_RENDER_DISTANCE; dx++) {
        for (int dy = -CHUNK_RENDER_DISTANCE; dy <= CHUNK_RENDER_DISTANCE; dy++) {
            int chunk_x = player_chunk_x + dx;
            int chunk_y = player_chunk_y + dy;

            chunk_x += CHUNK_OFFSET_X;
            chunk_y += CHUNK_OFFSET_Y;

            if (player_pos.x < 0) chunk_x -= 1;
            if (player_pos.y < 0) chunk_y -= 1;

            // make sure chunk coordinates are within world bounds
            if (chunk_x < WORLD_WIDTH && chunk_y < WORLD_HEIGHT) {
                load_chunk(dimension, v2(chunk_x, chunk_y));
                // printf("LOADED CHUNK AT (grid pos) = %d, %d\n", chunk_x, chunk_y);
            }
        }
    }
}

void unload_chunks_renderdistance() {

    DimensionData* dimension = world->dimension;
    Vector2 player_pos = get_player_pos();

    // Convert player world position to chunk coordinates
    int player_chunk_x = (int)(player_pos.x / CHUNK_SIZE) + CHUNK_OFFSET_X;
    int player_chunk_y = (int)(player_pos.y / CHUNK_SIZE) + CHUNK_OFFSET_Y;

    if (player_pos.x < 0) player_chunk_x -= 1;
    if (player_pos.y < 0) player_chunk_y -= 1;

    // Iterate over all chunks in the world
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            Chunk* chunk = dimension->chunks[x][y];

            if (chunk != NULL) {
                int dx = abs(x - player_chunk_x);
                int dy = abs(y - player_chunk_y);

                // If the chunk is outside the render distance, unload it
                if (dx > CHUNK_RENDER_DISTANCE || dy > CHUNK_RENDER_DISTANCE) {
                    unload_chunk(dimension, v2(x,y));
                }
            }
        }
    }
}

void render_chunk_ground(){
    // render chunk ground texture
    for (int x = 0; x < WORLD_WIDTH; x++){
        for (int y = 0; y < WORLD_HEIGHT; y++){
            Chunk* chunk = world->dimension->chunks[x][y];
            if (chunk != NULL){
                Texture* texture = get_texture(get_biome_data_from_id(chunk->biome_id).ground_texture);
                Vector4 ground_color = get_biome_data_from_id(chunk->biome_id).ground_color;

                Vector2 chunk_pos_world = get_chunk_world_position(x,y);

                Vector2 assurance;
                assurance.x = (x - CHUNK_OFFSET_X) * CHUNK_SIZE;
                assurance.y = (y - CHUNK_OFFSET_Y) * CHUNK_SIZE;

                if (chunk_pos_world.x != assurance.x && chunk_pos_world.y != assurance.y){
                    assert(1==0, "PERKELE\n");
                }

                if (IS_DEBUG){
                    if (chunk->has_been_modified) ground_color = v4(0, 1, 0, 1);
                    else ground_color = v4(1, 0, 0, 1);
                }

                Matrix4 xform = m4_identity;
                xform = m4_translate(xform, v3(chunk_pos_world.x, chunk_pos_world.y, 0));
                draw_image_xform(texture->image, xform, v2(CHUNK_SIZE, CHUNK_SIZE), ground_color);

            }
        }
    }
}

void sort_entities(int* indices, Entity* entities, int count) {
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


void render_chunk_entities(){
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            Chunk* chunk = world->dimension->chunks[x][y];

            if (chunk != NULL) {
                int indices[chunk->entity_count];

                if (ENTITIES_NEED_SORTING){
                    // ENTITIES_NEED_SORTING = false;
                    int entity_count = chunk->entity_count;
                    // int indices[entity_count];
                    for (int i = 0; i < entity_count; i++) {
                        indices[i] = i;
                    }

                    sort_entities(indices, chunk->entities, entity_count);
                }

                // render chunk entities
                for (int i = 0; i < chunk->entity_count; i++){
                    int index = indices[i];
                    Entity* en = &chunk->entities[index];

                    // Entity* en = &chunk->entities[i];
                    if (en != NULL && en->is_valid){

                        // frustrum culling
                        float entity_dist_from_player = fabsf(v2_dist(en->pos, get_player_pos()));

                        if (entity_dist_from_player <= entity_render_distance){

                            switch (en->arch){

                                case ARCH_item: {
                                    {
                                        Sprite* sprite = get_sprite(en->sprite_id);
							            Matrix4 xform = m4_identity;

                                        Vector4 item_shadow_color = {0, 0, 0, 0.2};
							
                                        xform = m4_translate(xform, v3(0, 2.0 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0)); // bob item up and down
                                        
                                        // shadow position
                                        Vector2 position = en->pos;
                                        position.x = position.x - (0.5 * get_sprite_size(sprite).x);
                                        position.y = position.y - (0.5 * get_sprite_size(sprite).y) - 1;
                                        
                                        // item shadow
                                        draw_circle(position, v2(get_sprite_size(sprite).x, get_sprite_size(sprite).y * 0.5), item_shadow_color);

                                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
            							xform = m4_translate(xform, v3(sprite->image->width * -0.5, 0.0, 0));

            							draw_image_xform(sprite->image, xform, get_sprite_size(sprite), COLOR_WHITE);


                                    }
                                } break;

                                case ARCH_building: break;

                                default:{
                                    {

                                        if (en->arch == ARCH_building){
                                            int asd = 0;
                                        }

                                        Sprite* sprite = get_sprite(en->sprite_id);

                                        Vector4 col = COLOR_WHITE;

                                        if (en->col_adj){
                                            col = en->col_adj_val;
                                        }

                                        if (world_frame.selected_entity == en && !en->unselectable){
                                            // col = v4(0.7, 0.7, 0.7, 1.0);
                                            col.r -= 0.3;
                                            col.g -= 0.3;
                                            col.b -= 0.3;
                                        }

                                        Matrix4 xform = m4_identity;
                                        xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
                                        // xform = m4_translate(xform, v3(sprite->image->width*-0.5,0,0));

                                        if (en->has_custom_rendering_size) draw_image_xform(sprite->image, xform, en->custom_rendering_size, col);
                                        else draw_image_xform(sprite->image, xform, get_sprite_size(sprite), col);
                                    }
                                }
                            }

                            if (en->arch == ARCH_building){
                                Sprite* sprite = get_sprite(en->sprite_id);

                                Vector4 col = COLOR_WHITE;
                                if (en->col_adj) col = en->col_adj_val;

                                Matrix4 xform = m4_identity;
                                xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));

                                if (en->has_custom_rendering_size) draw_image_xform(sprite->image, xform, en->custom_rendering_size, col);
                                else draw_image_xform(sprite->image, xform, get_sprite_size(sprite), col);

                                // Range2f hitbox = en->hitbox;
                                // draw_rect(v2(hitbox.min.x, hitbox.min.y), v2(hitbox.max.x, hitbox.max.y), COLOR_WHITE);
                            }
                        }
                    }
                }
            }
        }
    }
}



void chunk_debug_print(){
    set_screen_space();
    Matrix4 xform = m4_identity;
    xform = m4_translate(xform, v3(0, screen_height - 10, 0));
    draw_text_xform(font, sprint(get_heap_allocator(), STR("chunk count = %d"), world->dimension->chunk_count), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);

    xform = m4_translate(xform, v3(0, -5, 0));
    draw_text_xform(font, sprint(get_heap_allocator(), STR("Total Chunks entity count = %d"), total_entity_count), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);

    xform = m4_translate(xform, v3(0, -5, 0));
    Vector2 pos = get_player_pos();
    pos.x = (pos.x / CHUNK_SIZE) + CHUNK_OFFSET_X;
    pos.y = (pos.y / CHUNK_SIZE) + CHUNK_OFFSET_Y;
    int x = (int)pos.x;
    int y = (int)pos.y;
    if (world->dimension->chunks[x][y]){
        draw_text_xform(font, sprint(get_heap_allocator(), STR("current chunk[%d][%d], biomeid = %d"), x, y, world->dimension->chunks[x][y]->biome_id), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);
    }
    else{
        draw_text_xform(font, sprint(get_heap_allocator(), STR("current chunk[%d][%d] = NULL\n"), x, y), font_height, xform, v2(0.1, 0.1), COLOR_WHITE);
    }


    set_world_space();
}

void chunk_manager(){

    // TODO: maybe create a thing here where the chunks load / unload only when player x,y is multiplies of CHUNK_SIZE. this way they arent constantly running

    load_chunks_renderdistance();

    unload_chunks_renderdistance();

    render_chunk_ground();

    // moved these calls into main.c
    // render_chunk_entities();
    // chunk_debug_print();
}

#endif