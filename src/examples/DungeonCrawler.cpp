//#include "../slim/draw/rectangle.h"
//#include "../slim/draw/texture.h"
//#include "../slim/serialization/texture.h"
//#include "../slim/app.h"
// Or using the single-header file:
#include "../slim.h"

constexpr int row_count = 20;
constexpr int column_count = 40;
constexpr int max_lights_count = 16;

struct Tile { bool is_full = false; };
static Tile invalid_tile;
bool isValid(const Tile& tile) {
    return &tile != &invalid_tile;
}

struct Light {
    float pos_x, pos_y, intensity = 1.0f;
};

struct TileMap {
    Tile tiles[row_count][column_count];
    Light lights[max_lights_count];
    int lights_count = 0;

    char* floor_texture_file_name = (char*)"floor.texture";
    char* wall_texture_file_name = (char*)"wall.texture";
    Texture floor_texture;
    Texture wall_texture;
    TexturePack texture_pack{ 2,
                              &floor_texture,
                              &floor_texture_file_name,
                              (char*)__FILE__ };

    float tiles_per_pixel = 0.1f, pixels_per_tile = 10.0f;
    float pan_x = 0, pan_y = 0;

    void zoom(float by) {
        float old_mouse_x_in_tile_space = tileOfPixelX(mouse::pos_x);
        float old_mouse_y_in_tile_space = tileOfPixelY(mouse::pos_y);

        pan_x *= tiles_per_pixel;
        pan_y *= tiles_per_pixel;

        pixels_per_tile += by;
        if (pixels_per_tile < 1.0f) pixels_per_tile = 1.0f;

        tiles_per_pixel = 1.0f / pixels_per_tile;

        pan_x *= pixels_per_tile;
        pan_y *= pixels_per_tile;

        float new_mouse_x_in_tile_space = tileOfPixelX(mouse::pos_x);
        float new_mouse_y_in_tile_space = tileOfPixelY(mouse::pos_y);
        pan_x += pixels_per_tile * (float)(new_mouse_x_in_tile_space - old_mouse_x_in_tile_space);
        pan_y += pixels_per_tile * (float)(new_mouse_y_in_tile_space - old_mouse_y_in_tile_space);
    }

    void pan(int dx, int dy) {
        pan_x += (float)dx;
        pan_y += (float)dy;
    }

    float tileOfPixelX(int x) const { return tiles_per_pixel * ((float)x - pan_x); }
    float tileOfPixelY(int y) const { return tiles_per_pixel * ((float)y - pan_y); }
    int pixelOfTileX(float X) const { return (int)(X * pixels_per_tile + pan_x); }
    int pixelOfTileY(float Y) const { return (int)(Y * pixels_per_tile + pan_y); }

    Tile& tileAt(int x, int y) {
        int tile_x = (int)tileOfPixelX(x);
        int tile_y = (int)tileOfPixelY(y);
        if (tile_x < 0 || tile_x >= column_count ||
            tile_y < 0 || tile_y >= row_count)
            return invalid_tile;
        else
            return tiles[tile_y][tile_x];
    }

    void draw(const Canvas& canvas) {
        for (int y = 0; y < row_count; y++)
            for (int x = 0; x < column_count; x++)
                drawTexture(tiles[y][x].is_full ? wall_texture : floor_texture, canvas, RectI{
                        pixelOfTileX((float)x),
                        pixelOfTileX((float)(x + 1)),
                        pixelOfTileY((float)y),
                        pixelOfTileY((float)(y + 1))
                }, false);

        Pixel* pixel = canvas.pixels;
        for (int y = 0; y < canvas.dimensions.height; y++) {
            for (int x = 0; x < canvas.dimensions.width; x++, pixel++) {
                Tile& tile = tileAt(x, y);
                if (&tile == &invalid_tile)
                    continue;

                if (tile.is_full || lights_count == 0)
                    continue;

                float light = 0;
                for (int l = 0; l < lights_count; l++) {
                    float dx = lights[l].pos_x - tileOfPixelX(x);
                    float dy = lights[l].pos_y - tileOfPixelY(y);
                    float light_attenuation = dx * dx + dy * dy;
                    light += lights[l].intensity / light_attenuation;
                }
                pixel->color = (pixel->color * light).clamped();
            }
        }

        if (controls::is_pressed::ctrl)
            canvas.drawRect(RectI{
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x))),
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x) + 1)),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y))),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y) + 1))
            }, Magenta);
    }
};

void applyRetroShader(const Canvas &canvas, bool use_gameboy_colors, int down_scale = 4) {
    float down_scale_factor = 1.0f / (float)(down_scale * down_scale);
    int down_scaled_width = canvas.dimensions.width / down_scale;
    int down_scaled_height = canvas.dimensions.height / down_scale;
    for (int y = 0; y < down_scaled_height; y++) {
        for (int x = 0; x < down_scaled_width; x++) {
            // Pixelated (down-scale): Average out a square of pixels
            Color color;
            for (int v = 0; v < down_scale; v++)
                for (int u = 0; u < down_scale; u++)
                    color += canvas.pixels[canvas.dimensions.width * (y*down_scale + v) + x*down_scale + u].color;
            color *= down_scale_factor;

            if (use_gameboy_colors) {
                int luminance = 16 + ( // Y of YCrCb (modulated)
                    (((int)(color.r * ( 65.738f * 2.5f)))) +
                    (((int)(color.g * (129.057f * 2.5f)))) +
                    (((int)(color.b * ( 25.064f * 2.5f))))
                );
                switch (luminance >> 6) { // Choose a GameBoy color based on luminance:
                    case 0:  color.setByHex(0x0F380F); break;
                    case 1:  color.setByHex(0x306130); break;
                    case 2:  color.setByHex(0x8AAB0F); break;
                    default: color.setByHex(0xBBCF5D);
                }
            } else { // Reduce bit-depth to 5 bits for red/blue and 6 bits for green:
                color.r = (float)((int)(color.r * (1 << 5))) / (1 << 5);
                color.g = (float)((int)(color.g * (1 << 6))) / (1 << 6);
                color.b = (float)((int)(color.b * (1 << 5))) / (1 << 5);
            }

            //  Pixelated (re-up-scale): Set the whole square of pixels to the new color
            for (int v = 0; v < down_scale; v++)
                for (int u = 0; u < down_scale; u++)
                    canvas.pixels[canvas.dimensions.width * (y*down_scale + v) + x*down_scale + u].color = color;
        }
    }
}

struct DungeonCrawler : SlimApp {
    Canvas canvas;
    TileMap tile_map;

    void OnMouseButtonDown(mouse::Button& mouse_button) override {
        if (&mouse_button == &mouse::left_button && controls::is_pressed::alt)
            tile_map.lights_count++;
    }

    void OnUpdate(f32 delta_time) override {
        if (mouse::wheel_scrolled) {
            if (mouse::left_button.is_pressed && controls::is_pressed::alt) {
                float new_intensity = tile_map.lights[tile_map.lights_count - 1].intensity + mouse::wheel_scroll_amount * 0.005f;
                tile_map.lights[tile_map.lights_count - 1].intensity = new_intensity > 0.1f ? new_intensity : 0.1f;
            } else
                tile_map.zoom(mouse::wheel_scroll_amount * 0.05f);
        }
        if (mouse::moved) {
            if (mouse::left_button.is_pressed && controls::is_pressed::alt) {
                tile_map.lights[tile_map.lights_count - 1].pos_x = tile_map.tileOfPixelX(mouse::pos_x);
                tile_map.lights[tile_map.lights_count - 1].pos_y = tile_map.tileOfPixelY(mouse::pos_y);
            }

            if (mouse::right_button.is_pressed && !controls::is_pressed::ctrl)
                tile_map.pan(mouse::movement_x, mouse::movement_y);
        }

        if (controls::is_pressed::ctrl && (mouse::left_button.is_pressed || mouse::right_button.is_pressed)) {
            Tile& tile_under_mouse = tile_map.tileAt(mouse::pos_x, mouse::pos_y);
            if (isValid(tile_under_mouse))
                tile_under_mouse.is_full = mouse::left_button.is_pressed;
        }
    }

    bool Retro = false;
    bool GameBoy = false;
    int down_scale = 4;

    void OnRender() override {
        canvas.clear();
        tile_map.draw(canvas);
        if (Retro) applyRetroShader(canvas, GameBoy, down_scale);
        canvas.drawToWindow();
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }

    void OnKeyChanged(u8 key, bool pressed) override {
        if (!pressed) {
            if (key == controls::key_map::tab) Retro = !Retro;
            if (key == '1') GameBoy = !GameBoy;
            if (key == '2') down_scale = down_scale == 4 ? 8 : 4;
        }
    }
};

SlimApp* createApp() {
    return new DungeonCrawler();
}