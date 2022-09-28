#include "../slim/draw/circle.h"
#include "../slim/draw/rectangle.h"
#include "../slim/draw/texture.h"
#include "../slim/serialization/texture.h"
#include "../slim/app.h"

// Or using the single-header file:
//#include "../slim.h"

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

struct TileMapApp : SlimApp {
    TileMapApp() {
        initLightPoints();
    }

    Canvas canvas;

    char* floor_texture_file_name = (char*)"floor.texture";
    char* wall_texture_file_name = (char*)"wall.texture";
    Texture floor_texture;
    Texture wall_texture;
    TexturePack texture_pack{ 2,
                              &floor_texture,
                              &floor_texture_file_name,
                              (char*)__FILE__ };

    struct LightPoint {
        float X, Y;
    };
    LightPoint light_points[32];
    int light_points_count = 8;

    struct Tile {
        bool is_full = false;
    };
    Tile map[row_count][column_count];

    struct Light {
        Color color = White;
        float pos_x= 0, pos_y = 0, intensity = 1.0f, point_intensity = 1.0f;
    };
    Light lights[max_lights_count];

    Color colors[3]{BrightRed, BrightGreen, BrightBlue};

    int lights_count = 0;
    bool user_is_adding_a_light = false;
    int light_color_index = 0;

    float tiles_per_pixel = 0.1f, pixels_per_tile = 10.0f;
    float pan_x = 0, pan_y = 0;

    float getCircleFraction(float X, float Y) {
        if (X == 0) return Y > 0 ? 0.25f : 0.75f;
        if (Y == 0) return X > 0 ? 0 : 0.5f;

        float delta = 0;

        if (Y < 0) {
            delta = 0.5f;
            X = -X;
            Y = -Y;
        }

        if (X < 0) {
            delta += 0.25f;
            float tmp = Y;
            Y = -X;
            X = tmp;
        }

        if (X > Y) {
            return delta + (Y / X) * 0.125f;
        } else {
            delta += 0.125f;
            return delta + (1.0f - X / Y) * 0.125f;
        }
    }

    void initLightPoints(int count = 32) {
        light_points_count = count;

        float angle = 0;
        float angle_delta = 2*3.14f / (float)count;
        for (int i = 0; i < count; i++) {
            light_points[i].X = cosf(angle);
            light_points[i].Y = sinf(angle);
            angle += angle_delta;
        }
    };

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

    bool isOutOfBounds(float X, float Y) { return X < 0 || X >= column_count ||
                                                  Y < 0 || Y >= row_count; }

    bool inShadow(float lightX, float lightY, float X, float Y) {
        bool is_light_on_the_right = X < lightX;
        bool is_light_above = lightY < Y;

        int start_x = (int)(is_light_on_the_right ? X : lightX);
        int end_x = (int)(is_light_on_the_right ? lightX : X);

        int start_y = (int)(is_light_above ? lightY : Y);
        int end_y = (int)(is_light_above ? Y : lightY);

        for (int y = start_y; y <= end_y; y++) {
            for (int x = start_x; x <= end_x; x++) {
                Tile &tile = map[y][x];
                if (tile.is_full) {
                    float edge_y = is_light_above ? (float)(y + 1) : y;
                    float hit_x = X + ((edge_y - Y) / (lightY - Y)) * (lightX - X);
                    if (x <= hit_x && hit_x <= (float)(x + 1))
                        return true;

                    float edge_x = is_light_on_the_right ? x : (float)(x + 1);
                    float hit_y = Y + ((edge_x - X) / (lightX - X)) * (lightY- Y);
                    if (y <= hit_y && hit_y <= (float)(y + 1))
                        return true;
                }
            }
        }

        return false;
    }

    void OnRender() override {
        canvas.clear();
        for (int y = 0; y < row_count; y++) {
            for (int x = 0; x < column_count; x++)
                drawTexture(map[y][x].is_full ? wall_texture : floor_texture, canvas, RectI{
                        pixelOfTileX((float)x),
                        pixelOfTileX((float)(x + 1)),
                        pixelOfTileY((float)y),
                        pixelOfTileY((float)(y + 1))
                }, false);
        }

        if (lights_count) {
            for (int y = 0; y < canvas.dimensions.height; y++) {
                for (int x = 0; x < canvas.dimensions.width; x++) {
                    float X = tileOfPixelX(x);
                    float Y = tileOfPixelY(y);
                    if (isOutOfBounds(X, Y) || map[(int)Y][(int)X].is_full)
                        continue;

                    Color accumulated_light;
                    for (int l = 0; l < lights_count; l++) {
                        Light &light = lights[l];

                        float circle_fraction = getCircleFraction(light.pos_x - X,
                                                                  light.pos_y - Y);
                        int first_index = 1 + (int)((circle_fraction + 0.25f) * light_points_count);
                        int last_index = first_index + (light_points_count / 2);
                        int lit_point_count = 0;
                        for (int i = first_index; i < last_index; i++) {
                            float lightPointX = light.pos_x + light_points[i % light_points_count].X;
                            float lightPointY = light.pos_y + light_points[i % light_points_count].Y;

                            if (!inShadow(lightPointX, lightPointY, X, Y))
                                lit_point_count++;
                        }
                        if (lit_point_count == 0)
                            continue;

                        float dX = light.pos_x - X;
                        float dY = light.pos_y - Y;
                        float squared_distance = dX*dX + dY*dY;

                        accumulated_light += light.color * ((light.intensity / squared_distance
                                ) * ((float)lit_point_count / (float)(light_points_count / 2)));
                    }
                    Pixel &pixel = canvas.pixels[canvas.dimensions.width * y + x];
                    pixel.color = (pixel.color * accumulated_light).clamped();
                }
            }

            float X = tileOfPixelX(mouse::pos_x);
            float Y = tileOfPixelY(mouse::pos_y);
            for (int l = 0; l < lights_count; l++) {
                Light &light = lights[l];
                float circle_fraction = getCircleFraction(light.pos_x - X,
                                                          light.pos_y - Y);
                int first_index = 1 + (int)((circle_fraction + 0.25f) * light_points_count);
                int last_index = first_index + (light_points_count / 2);
                for (int i = first_index; i < last_index; i++) {
                    float lightPointX = light.pos_x + light_points[i % light_points_count].X;
                    float lightPointY = light.pos_y + light_points[i % light_points_count].Y;

                    canvas.fillCircle(pixelOfTileX(lightPointX),
                                      pixelOfTileY(lightPointY), 3, Red);
                }
            }
        }

        if (controls::is_pressed::ctrl) {
            canvas.drawRect(RectI{
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x))),
                    pixelOfTileX((float)((int)tileOfPixelX(mouse::pos_x) + 1)),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y))),
                    pixelOfTileY((float)((int)tileOfPixelY(mouse::pos_y) + 1))
            }, Magenta);
        }
        canvas.drawToWindow();
    }

    void OnMouseButtonDown(mouse::Button& mouse_button) override {
        if (&mouse_button == &mouse::left_button && controls::is_pressed::alt) {
            user_is_adding_a_light = true;
            light_color_index = 0;
            lights_count++;
        }
    }

    void OnMouseButtonUp(mouse::Button& mouse_button) override {
        if (&mouse_button == &mouse::left_button)
            user_is_adding_a_light = false;
    }

    void OnUpdate(f32 delta_time) override {
        if (mouse::wheel_scrolled) {
            if (user_is_adding_a_light) {
                Light &light = lights[lights_count - 1];
                if (controls::is_pressed::shift) {
                    light_color_index += (int)(mouse::wheel_scroll_amount * 0.01f);
                    if (light_color_index < 0)
                        light_color_index = 0;
                    else
                        light_color_index %= 3;
                    light.color = colors[light_color_index];
                } else {
                    light.intensity += mouse::wheel_scroll_amount * 0.005f;
                    if (light.intensity < 0.1f) light.intensity = 0.1f;
                }
            } else
                zoom(mouse::wheel_scroll_amount * 0.05f);
        }
        if (mouse::moved) {
            if (user_is_adding_a_light) {
                Light &light = lights[lights_count - 1];
                light.pos_x = tileOfPixelX(mouse::pos_x);
                light.pos_y = tileOfPixelY(mouse::pos_y);
            }

            if (mouse::right_button.is_pressed && !controls::is_pressed::ctrl)
                pan(mouse::movement_x, mouse::movement_y);
        }

        if (controls::is_pressed::ctrl && (mouse::left_button.is_pressed ||
                                           mouse::right_button.is_pressed)) {
            float X = tileOfPixelX(mouse::pos_x);
            float Y = tileOfPixelY(mouse::pos_y);
            if (!isOutOfBounds(X, Y))
                map[(int)Y][(int)X].is_full = mouse::left_button.is_pressed;
        }
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new TileMapApp();
}