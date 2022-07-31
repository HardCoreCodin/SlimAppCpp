//#include "../slim/draw/rectangle.h"
//#include "../slim/draw/texture.h"
//#include "../slim/serialization/texture.h"
//#include "../slim/app.h"
// Or using the single-header file:
#include "../slim.h"

constexpr int row_count = 20;
constexpr int column_count = 40;
constexpr int max_lights_count = 8;

namespace circle {
    constexpr int max_point_count = 64;

    int point_count, semicircle_point_count;

    struct Point { float X, Y; };
    Point points[max_point_count];

    void setPoints(int count = 2) {
        if (count < 2) count = 2;
        if (count > max_point_count) count = max_point_count;
        point_count = count;
        semicircle_point_count = point_count / 2;

        float angle_increment = TAU / (float)point_count;
        float angle = 0;
        for (int i = 0; i < point_count; i++) {
            points[i].X = cosf(angle);
            points[i].Y = sinf(angle);
            angle += angle_increment;
        }
    }

    INLINE static float getCircleFraction(float dX, float dY) {
        if (dX == 0) return dY < 0 ? 0.75f : 0.25f;
        if (dY == 0) return dX < 0 ? 0.5f : 0.0f;

        float offset = 0;
        if (dY < 0) { // At the bottom, rotate 180 degrees:
            dY = -dY;
            dX = -dX;
            offset = 0.5f;
        }

        // Top half from now on:

        if (dX < 0) { // On the left, rotate -90 degrees:
            float tmp = dY;
            dY = -dX;
            dX = tmp;
            offset += 0.25f;
        }

        // Top-right quadrant from now on:
        return offset + (dX < dY ? (0.125f + (0.125f * (1.0f - dX/dY))) : (0.125f * dY/dX));
    }
}

struct Tile { bool is_full = false; };

struct Light {
    Color color = White;
    float pos_x, pos_y, intensity = 1.0f, point_intensity = 1.0f;
};

Color colors[3]{BrightRed, BrightGreen, BrightBlue};

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

    void updateLightPosition(Light *light, int x, int y) {
        float X = light->pos_x = tileOfPixelX(x);
        float Y = light->pos_y = tileOfPixelY(y);
        float radius = light->intensity * 0.1f;
        float diameter = radius * 2;

        light->point_intensity = 0;

        X += radius;
        if (X >= column_count || tiles[(int)Y][(int)X].is_full)
            return;

        X -= diameter;
        if (X < 0 || tiles[(int)Y][(int)X].is_full)
            return;

        X += radius;
        Y += radius;
        if (Y >= row_count || tiles[(int)Y][(int)X].is_full)
            return;

        Y -= diameter;
        if (Y < 0 || tiles[(int)Y][(int)X].is_full)
            return;

        light->point_intensity = light->intensity / (float)circle::semicircle_point_count;
    }

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

    bool isOutOfBounds(float X, float Y) { return X < 0 || X >= column_count || Y < 0 || Y >= row_count; }

    bool isInShadow(float X, float Y, float lightX, float lightY, float dX, float dY) {
        bool light_is_on_the_left = lightX < X;
        bool light_is_above      = lightY < Y;

        int start_x = (int)X;
        int end_x   = (int)lightX;
        if (light_is_on_the_left) {
            start_x = (int)lightX;
            end_x = (int)X;
        }

        int start_y = (int)Y;
        int end_y   = (int)lightY;
        if (light_is_above) {
            start_y = (int)lightY;
            end_y = (int)Y;
        }

        if ((start_x == end_x && ((start_y == end_y) || ((end_y - start_y) == 1))) ||
            (start_y == end_y && ((start_x == end_x) || ((end_x - start_x) == 1))))
            return false;

        for (int y = start_y; y <= end_y; y++) {
            float top = (float)y;
            float bottom = (float)(y + 1);
            float boundY = light_is_above ? bottom : top;
            for (int x = start_x; x <= end_x; x++) {
                if (tiles[y][x].is_full) {
                    float left = (float)x;
                    float right = (float)(x + 1);
                    float boundX = light_is_on_the_left ? right : left;

                    float hitY = Y + dY * ((boundX - X) / dX);
                    if (hitY >= top && hitY <= bottom)
                        return true;

                    float hitX = X + dX * ((boundY - Y) / dY);
                    if (hitX >= left && hitX <= right)
                        return true;
                }
            }
        }

        return false;
    }

    void updateLightIntensities() {
        Light *light = lights;
        for (int l = 0; l < lights_count; l++, light++) {
            if (light->point_intensity != 0) {
                light->point_intensity = light->intensity / (float)(circle::semicircle_point_count);
            }
        }
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

        if (lights_count) {
            Pixel* pixel = canvas.pixels;
            Color accumulated_light;
            const float light_point_contribution = 1.0f / (float)circle::semicircle_point_count;
            for (int y = 0; y < canvas.dimensions.height; y++) {
                for (int x = 0; x < canvas.dimensions.width; x++, pixel++) {
                    float X = tileOfPixelX(x);
                    float Y = tileOfPixelY(y);
                    if (isOutOfBounds(X, Y) || tiles[(int)Y][(int)X].is_full)
                        continue;

                    accumulated_light = Black;
                    Light *light = lights;
                    for (int l = 0; l < lights_count; l++, light++) {
                        if (light->point_intensity == 0.0f)
                            continue;

                        float dX = light->pos_x - X;
                        float dY = light->pos_y - Y;
                        float squared_distance = dX*dX + dY*dY;

                        if (circle::semicircle_point_count == 1) {
                            if (isInShadow(X, Y, light->pos_x, light->pos_y, dX, dY))
                                continue;

                            accumulated_light += light->color * (light->intensity / squared_distance);
                        } else {
                            float radius = light->intensity * 0.1f;
                            float squared_radius = radius * radius;

                            float acculumlated_light_points_contribution = 0;
                            int start_point_index = (int)((float)circle::point_count * (circle::getCircleFraction(dX, dY) + 0.25f)) + 1;
                            int end_point_index = start_point_index + circle::semicircle_point_count;
                            for (int p = start_point_index; p < end_point_index; p++) {
                                int i = p % circle::point_count;
                                float lightPointX = light->pos_x + circle::points[i].X * radius;
                                float lightPointY = light->pos_y + circle::points[i].Y * radius;

                                dX = lightPointX - X;
                                dY = lightPointY - Y;

                                if (squared_distance > squared_radius && isInShadow(X, Y, lightPointX, lightPointY, dX, dY))
                                    continue;

                                acculumlated_light_points_contribution += light_point_contribution;
                            }
                            acculumlated_light_points_contribution *= acculumlated_light_points_contribution;
                            accumulated_light += light->color * (light->intensity * acculumlated_light_points_contribution / squared_distance);
                        }
                    }
                    pixel->color = (pixel->color * accumulated_light).clamped();
                }
            }
        }

        if (controls::is_pressed::ctrl)
            canvas.drawRect(RectI{
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x))),
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x) + 1)),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y))),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y) + 1))
            }, Magenta);

        if (false) {
            Light *light = lights;
            float X = tileOfPixelX(mouse::pos_x);
            float Y = tileOfPixelY(mouse::pos_y);

            for (int i = 0; i < lights_count; i++, light++) {
                if (light->point_intensity == 0)
                    continue;

                float dX = light->pos_x - X;
                float dY = light->pos_y - Y;
                if (circle::semicircle_point_count == 1) {
                    int x = pixelOfTileX(light->pos_x);
                    int y = pixelOfTileY(light->pos_y);
                    canvas.fillCircle(x, y, 3, Green);

                    bool in_shadow = isInShadow(X, Y, light->pos_x, light->pos_y, dX, dY);
                    canvas.drawLine((float)mouse::pos_x, (float)mouse::pos_y, (float)x, (float)y,
                                    Color{in_shadow ? DarkRed : BrightYellow}, 1, 3);
                } else {
                    float radius = light->intensity * 0.2f;

                    int start_point_index = (int)((float)circle::point_count * (circle::getCircleFraction(dX, dY) + 0.25f)) + 1;
                    int end_point_index = start_point_index + circle::semicircle_point_count;
                    for (int p = start_point_index; p < end_point_index; p++) {
                        int i = p % circle::point_count;
                        float lightPointX = light->pos_x + circle::points[i].X * radius;
                        float lightPointY = light->pos_y + circle::points[i].Y * radius;
                        int x = pixelOfTileX(lightPointX);
                        int y = pixelOfTileY(lightPointY);
                        canvas.fillCircle(x, y, 3, Green);

                        bool in_shadow = isInShadow(X, Y, lightPointX, lightPointY, lightPointX - X, lightPointY - Y);
                        canvas.drawLine((float)mouse::pos_x, (float)mouse::pos_y, (float)x, (float)y,
                                        Color{in_shadow ? DarkRed : BrightYellow}, 1, 3);
                    }
                }
            }
        }
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

    bool user_is_adding_a_light = false;
    int light_color_index = 0;
    int shadow_softness = 1;

    void OnMouseButtonDown(mouse::Button& mouse_button) override {
        if (&mouse_button == &mouse::left_button && controls::is_pressed::alt) {
            user_is_adding_a_light = true;
            light_color_index = 0;
            tile_map.lights_count++;
        }
    }

    void OnMouseButtonUp(mouse::Button& mouse_button) override {
        if (&mouse_button == &mouse::left_button)
            user_is_adding_a_light = false;
    }

    void OnUpdate(f32 delta_time) override {
        Light *light = user_is_adding_a_light ? &tile_map.lights[tile_map.lights_count - 1] : nullptr;

        if (mouse::wheel_scrolled) {
            if (light) {
                if (controls::is_pressed::shift) {
                    light_color_index += (int)(mouse::wheel_scroll_amount * 0.01f);
                    if (light_color_index < 0)
                        light_color_index = 0;
                    else
                        light_color_index %= 3;
                    light->color = colors[light_color_index];
                } else {
                    light->intensity += mouse::wheel_scroll_amount * 0.005f;
                    if (light->intensity < 0.1f) light->intensity = 0.1f;
                }
            } else
                tile_map.zoom(mouse::wheel_scroll_amount * 0.05f);
        }
        if (mouse::moved) {
            if (light)
                tile_map.updateLightPosition(light, mouse::pos_x, mouse::pos_y);

            if (mouse::right_button.is_pressed && !controls::is_pressed::ctrl)
                tile_map.pan(mouse::movement_x, mouse::movement_y);
        }

        if (controls::is_pressed::ctrl && (mouse::left_button.is_pressed || mouse::right_button.is_pressed)) {
            float X = tile_map.tileOfPixelX(mouse::pos_x);
            float Y = tile_map.tileOfPixelY(mouse::pos_y);
            if (!tile_map.isOutOfBounds(X, Y))
                tile_map.tiles[(int)Y][(int)X].is_full = mouse::left_button.is_pressed;
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
            if (key == '3' || key == '4') {
                if (key == '3')
                    shadow_softness--;
                else
                    shadow_softness++;
                if (shadow_softness < 1)
                    shadow_softness = 1;
                if (shadow_softness > 5)
                    shadow_softness = 5;
                circle::setPoints(1 << shadow_softness);
                tile_map.updateLightIntensities();
            }
        }
    }
};

SlimApp* createApp() {
    circle::setPoints();
    return new DungeonCrawler();
}