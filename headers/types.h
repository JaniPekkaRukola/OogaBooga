#ifndef TYPES_H
#define TYPES_H


#define MAX_ENTITY_COUNT 512
const float screen_width = 240.0;
const float screen_height = 135.0;

// font
Gfx_Font* font;
u32 font_height = 48;

// globals
float64 delta_t;
float64 current_time;
int fps = 0;
Vector2 camera_pos = {0, 0};
float view_zoom = 0.1875;   // view zoom ratio x (pixelart layer width / window width = 240 / 1280 = 0.1875)
const char res_folder[17] = "res/abyssophobia/";


// ENUMS --------------------------------------------->

    typedef enum EntityID {
        ENTITY_nil,

        ENTITY_player,
        ENTITY_item, // EntityID for all items
        ENTITY_rock,
        ENTITY_algae,



        ENTITY_MAX,
    } EntityID;

    typedef enum SpriteID{
        SPRITE_nil,

        SPRITE_player,
        SPRITE_algae1,

        SPRITE_MAX,
    } SpriteID;

    typedef enum UXState{
        UX_nil,

        UX_gameplay,
        UX_map,
        UX_mainmenu,

        UX_MAX,
    } UXState;

// 

// STRUCTS ------------------------------------------->

    typedef struct WorldFrame {
        // Entity* selected_entity;
        Matrix4 world_projection;
        Matrix4 world_view;
        bool hover_consumed;
        // Entity* player;
    } WorldFrame;


    typedef struct Sprite {
        Gfx_Image* image;
    } Sprite;
    Sprite sprites[SPRITE_MAX];


    typedef enum ItemID{
        ITEM_nil,

        ITEM_rock,
        ITEM_algae,


        ITEM_MAX,
    } ItemID;


    typedef struct ItemData{
        string name;
        int amount;
        SpriteID sprite;
        ItemID item_id;
        string tooltip;
    } ItemData;


    typedef struct Entity{
        string name;
        EntityID entity_id;
        Vector2 pos;
        SpriteID sprite;

        bool is_item;
        ItemID item_id;


        int health;
        bool is_destroyable;
        bool is_selectable;
        // bool enabled;
        bool is_valid;
    } Entity;


    typedef struct Player{
        Entity* player_en;

        int health;
        float oxygen;
        float oxygen_consumption;   // ogygen consumption rate
        float oxygen_refill;        // oxygen refill rate
    } Player;


    typedef struct World{
        int entity_count;
        Entity entities[MAX_ENTITY_COUNT];
        // Entity* player;
        Player* player;
        // UXState ux_state;
    } World;


// 


// globals

    World* world = 0;
    WorldFrame world_frame;

// 



#endif