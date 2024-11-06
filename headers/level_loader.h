#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

Level load_level(LevelID level_id){
// Level load_level(Level level){
    // LevelID level_id = level.id;

    printf("Loading level %d\n", level_id);

    Level* level = alloc(get_heap_allocator(), sizeof(Level));


    if (level_id <= 0 || level_id >= LEVEL_MAX){
        log_error("Failed to load level for %d\n", level_id);
        return *level;
    }

    string path, path_meta;

    switch (level_id){
        
        case LEVEL_1:{
            {
                path = sprint(get_temporary_allocator(), STR("%s/Levels/level_1.png"), res_folder);
                path_meta = sprint(get_temporary_allocator(), STR("%s/Levels/level_1_meta.png"), res_folder);
            }
        } break;

        default: assert(1==0, "switch defaulted @ load_level");
    }
    
    Gfx_Image* level_image = load_image_from_disk(path, get_heap_allocator());
    Gfx_Image* level_meta = load_image_from_disk(path_meta, get_heap_allocator());

    // assert
    if (!level_image || !level_meta){
        printf("\n\n");
        log_error("level image or meta '%d' was not setup correctly!", level_id);
        printf("Prolly because the level.png or level_meta.png is missing!");
        assert(level_image);
        assert(level_meta);
    }

    printf("Succesfully loaded level %d\n", level_id);

    level->id = level_id;
    level->level = level_image;
    level->level_meta = level_meta;

    return *level;

}

void change_level(Level* dest){
    Level loaded = load_level(dest->id);
    world->level = loaded;
    printf("Level changed to '%d'\n", loaded.id);
}

#endif