//#include "../slim/draw/rectangle.h"
//#include "../slim/draw/texture.h"
//#include "../slim/serialization/texture.h"
//#include "../slim/app.h"
// Or using the single-header file:
#include "../slim.h"

constexpr int row_count = 6;
constexpr int column_count = 8;
constexpr int tile_size = 64;

struct Tile { bool is_full = false; };
static Tile invalid_tile;
bool isValid(const Tile &tile) {
    return &tile != &invalid_tile;
}

struct TileMap {
    Tile tiles[row_count][column_count];

    float light_x, light_y;
    float light_intensity = 1.0f;

    char* floor_texture_file_name = (char*)"floor.texture";
    char* wall_texture_file_name = (char*)"wall.texture";
    Texture floor_texture;
    Texture wall_texture;
    TexturePack texture_pack{2,
                             &floor_texture,
                             &floor_texture_file_name,
                             (char*)__FILE__};

    float pan_x = 0, pan_y = 0, zoom_scale = 1.0f, zoom_amount = 1.0f, to_normalized = 1.0f / tile_size, to_pixel = tile_size;

    void zoom(float amount) {
        float offset_x = toNormalizedX(mouse::pos_x);
        float offset_y = toNormalizedY(mouse::pos_y);

        zoom_amount += amount;
        zoom_scale = zoom_amount > 1 ? zoom_amount : (zoom_amount < -1.0f ? (-1.0f / zoom_amount) : 1.0f);
        to_pixel = zoom_scale * tile_size;
        to_normalized = 1.0f / to_pixel;

        float new_offset_x = toNormalizedX(mouse::pos_x);
        float new_offset_y = toNormalizedY(mouse::pos_y);

        pan_x += new_offset_x - offset_x;
        pan_y += new_offset_y - offset_y;
    }

    void pan(int dx, int dy) {
        pan_x += (float)dx * to_normalized;
        pan_y += (float)dy * to_normalized;
    }

    int toTileX(int x) const { return (int)(toNormalizedX(x)); }
    int toTileY(int y) const { return (int)(toNormalizedY(y)); }
    float toNormalizedX(int x) const { return to_normalized * (float)x - pan_x; }
    float toNormalizedY(int y) const { return to_normalized * (float)y - pan_y; }
    int toPixelX(float x) const { return (int)((x + pan_x) * to_pixel); }
    int toPixelY(float y) const { return (int)((y + pan_y) * to_pixel); }

    Tile& atPixelCoord(int x, int y) {
        int tile_x = toTileX(x);
        int tile_y = toTileY(y);
        if (tile_x < 0 || tile_x >= column_count ||
            tile_y < 0 || tile_y >= row_count)
            return invalid_tile;
        else
            return tiles[tile_y][tile_x];
    }

    void draw(const Canvas &canvas) {
        for (int y = 0; y < row_count; y++)
            for (int x = 0; x < column_count; x++)
                drawTexture(tiles[y][x].is_full ? wall_texture : floor_texture, canvas, RectI{
                    toPixelX((float)x),
                    toPixelX((float)(x + 1)),
                    toPixelY((float)y),
                    toPixelY((float)(y + 1))
                    }, false);

        Pixel *pixel = canvas.pixels;
        for (int y = 0; y < canvas.dimensions.height; y++) {
            for (int x = 0; x < canvas.dimensions.width; x++, pixel++) {
                Tile &tile = atPixelCoord(x, y);
                if (&tile == &invalid_tile)
                    continue;

                if (tile.is_full) {
                    pixel->color /= 2;
                    continue;
                }

                float dx = light_x - toNormalizedX(x);
                float dy = light_y - toNormalizedY(y);
                float light_attenuation = dx*dx + dy*dy;

                pixel->color *= light_intensity / light_attenuation;
            }
        }
    }
};

struct DungeonCrawler : SlimApp {
    Canvas canvas;
    TileMap tile_map;

    void OnUpdate(f32 delta_time) override {
        if (mouse::wheel_scrolled) {
            float delta = mouse::wheel_scroll_amount * 0.01f;
            if (controls::is_pressed::ctrl)
                tile_map.light_intensity += delta;
            else
                tile_map.zoom(delta);
        }
        if (mouse::moved) {
            tile_map.light_x = tile_map.toNormalizedX(mouse::pos_x);
            tile_map.light_y = tile_map.toNormalizedY(mouse::pos_y);

            if (mouse::right_button.is_pressed)
                tile_map.pan(mouse::movement_x, mouse::movement_y);
        }

        if (mouse::left_button.is_pressed) {
            Tile &tile_under_mouse = tile_map.atPixelCoord(mouse::pos_x, mouse::pos_y);
            if (isValid(tile_under_mouse))
                tile_under_mouse.is_full = !controls::is_pressed::ctrl;
        }
    }

    void OnRender() override {
        canvas.clear();

        tile_map.draw(canvas);
//        for (int y = 0; y <= row_count; y++) canvas.drawHLine(0, tile_size * column_count, y * tile_size);
//        for (int x = 0; x <= column_count; x++) canvas.drawVLine(0, tile_size * row_count, x * tile_size);
//        drawRay();

        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }

    void OnKeyChanged(u8 key, bool is_pressed) override {

    }

//    bool isTileFull(int tile_x, int tile_y) {
//        return (tile_x >= 0 && tile_x < column_count &&
//                tile_y >= 0 && tile_y < row_count &&
//                !map[tile_y][tile_x].is_full);
//    }
//
//    bool hitSomething(float src_x, float src_y, float trg_x, float trg_y, int &hit_tile_x, int &hit_tile_y, float &hit_x, float &hit_y) {
//        if (src_x < 0 || src_y < 0 || trg_x < 0 || trg_y < 0 ||
//            src_x >= column_count || trg_x >= column_count || src_y >= row_count || trg_y >= row_count)
//            return false;
//
//        hit_tile_x = (int)src_x;
//        hit_tile_y = (int)src_y;
//        if (map[hit_tile_y][hit_tile_x].is_full) {
//            hit_x = src_x;
//            hit_y = src_y;
//            return true;
//        }
//
//        bool aiming_right = src_x < trg_x;
//        bool aiming_down  = src_y < trg_y;
//        bool aiming_left = trg_x < src_x;
//        bool aiming_up   = trg_y < src_y;
//        bool vertical   = src_x == trg_x;
//        bool horizontal = src_y == trg_y;
//
//        float total_dx = trg_x - src_x;
//        float total_dy = trg_y - src_y;
//        float dy_per_x = total_dy / total_dx;
//        float dx_per_y = total_dx / total_dy;
//        int tile_inc_x = vertical ? 0 : (aiming_right ? 1 : -1);
//        int tile_inc_y = horizontal ? 0 : (aiming_down ? 1 : -1);
//
//        float start_x = src_x;
//        float start_y = src_y;
//
//        bool diagonal = !vertical && !horizontal;
//        if (diagonal) {
//            float x = (float)hit_tile_x;
//            float y = (float)hit_tile_y;
//            float dx = src_x - x;
//            float dy = src_y - y;
//
//            if (aiming_left)
//                start_y += dy_per_x * (1.0f - dx);
//            else
//                start_y -= dy_per_x * dx;
//
//            if (aiming_up) dy -= 1.0f;
//            start_x -= dx_per_y * dy;
//        }
//
//        int horizontal_hit_tile_x = (int)start_x;
//        int horizontal_hit_tile_y = (int)start_y;
//        float horizontal_hit_x = start_x;
//        float horizontal_hit_y = start_y;
//
//        bool horizontal_hit = false;
//        while (!horizontal_hit) {
//            horizontal_hit_tile_x += tile_inc_x;
//            if (horizontal_hit_tile_x < 0 ||
//                horizontal_hit_tile_x >= column_count)
//                break;
//
//            horizontal_hit_x += (float)tile_inc_x;
//            if (!horizontal)
//                horizontal_hit_y += dy_per_x;
//            horizontal_hit_tile_y = (int)horizontal_hit_y;
//            if (horizontal_hit_tile_y < 0 ||
//                horizontal_hit_tile_y >= row_count)
//                break;
//
//            horizontal_hit = map[horizontal_hit_tile_y][horizontal_hit_tile_x].is_full;
//        }
//
//        int vertical_hit_tile_x = (int)start_x;
//        int vertical_hit_tile_y = (int)start_y;
//        float vertical_hit_x = start_x;
//        float vertical_hit_y = start_y;
//
//        bool vertical_hit = false;
//        while (!vertical_hit) {
//            vertical_hit_tile_y += tile_inc_y;
//            if (vertical_hit_tile_y < 0 ||
//                vertical_hit_tile_y >= row_count)
//                break;
//
//            vertical_hit_y += (float)tile_inc_y;
//            if (!vertical)
//                vertical_hit_x += dx_per_y;
//            vertical_hit_tile_x = (int)vertical_hit_x;
//            if (vertical_hit_tile_x < 0 ||
//                vertical_hit_tile_x >= column_count)
//                break;
//
//            vertical_hit = map[vertical_hit_tile_y][vertical_hit_tile_x].is_full;
//        }
//
//        if (!vertical_hit && !horizontal_hit)
//            return false;
//
//        if (vertical_hit && horizontal_hit) {
//            float vertical_hit_dx = vertical_hit_x - src_x;
//            float vertical_hit_dy = vertical_hit_y - src_y;
//            float vertical_hit_distance_squared = vertical_hit_dx * vertical_hit_dx + vertical_hit_dy * vertical_hit_dy;
//
//            float horizontal_hit_dx = horizontal_hit_x - src_x;
//            float horizontal_hit_dy = horizontal_hit_y - src_y;
//            float horizontal_hit_distance_squared = horizontal_hit_dx * horizontal_hit_dx + horizontal_hit_dy * horizontal_hit_dy;
//
//            if (vertical_hit_distance_squared < horizontal_hit_distance_squared) {
//                hit_tile_x = vertical_hit_tile_x;
//                hit_tile_y = vertical_hit_tile_y;
//                hit_x = vertical_hit_x;
//                hit_y = vertical_hit_y;
//            } else {
//                hit_tile_x = horizontal_hit_tile_x;
//                hit_tile_y = horizontal_hit_tile_y;
//                hit_x = horizontal_hit_x;
//                hit_y = horizontal_hit_y;
//            }
//        } else if (vertical_hit) {
//            hit_tile_x = vertical_hit_tile_x;
//            hit_tile_y = vertical_hit_tile_y;
//            hit_x = vertical_hit_x;
//            hit_y = vertical_hit_y;
//        } else {
//            hit_tile_x = horizontal_hit_tile_x;
//            hit_tile_y = horizontal_hit_tile_y;
//            hit_x = horizontal_hit_x;
//            hit_y = horizontal_hit_y;
//        }
//
//        return true;
//    }
//
//    void drawRay() {
//        float x = 75;
//        float y = 55;
//        float x2 = (float)mouse::pos_x;
//        float y2 = (float)mouse::pos_y;
//
//        float trg_x =  x / tile_size;
//        float trg_y =  y / tile_size;
//        float src_x = mouse_x;
//        float src_y = mouse_y;
//
//        int hit_tile_x;
//        int hit_tile_y;
//        float hit_x;
//        float hit_y;
//        bool hit = hitSomething(src_x, src_y, trg_x, trg_y, hit_tile_x, hit_tile_y, hit_x, hit_y);
//        if (hit) {
//            x2 = hit_x * tile_size;
//            y2 = hit_y * tile_size;
//
//            canvas.drawRect(RectI{
//                    hit_tile_x * tile_size,
//                    (hit_tile_x + 1) * tile_size,
//                    hit_tile_y * tile_size,
//                    (hit_tile_y + 1) * tile_size,
//            }, Magenta);
//        }
//
//        canvas.drawLine(x, y, x2, y2, Yellow);
//    }

};

SlimApp* createApp() {
    return new DungeonCrawler();
}