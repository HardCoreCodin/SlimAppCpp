#define SLIMMER

#include "../slim/math/vec2.h"
#include "../slim/math/mat2.h"
#include "../slim/draw/circle.h"
#include "../slim/draw/triangle.h"
#include "../slim/draw/hud.h"
#include "../slim/draw/image.h"
#include "../slim/serialization/image.h"
#include "../slim/serialization/texture.h"
#include "../slim/app.h"

// Or using the single-header file:
//#include "../slim.h"

#define NAVIGATION_DEFAULT__MAX_VELOCITY 50
#define NAVIGATION_DEFAULT__ACCELERATION 100
#define NAVIGATION_SPEED_DEFAULT__TURN   1
#define NAVIGATION_SPEED_DEFAULT__ORIENT 0.002f
#define NAVIGATION_SPEED_DEFAULT__ORBIT  0.002f
#define NAVIGATION_SPEED_DEFAULT__ZOOM   0.003f
#define NAVIGATION_SPEED_DEFAULT__DOLLY  1
#define NAVIGATION_SPEED_DEFAULT__PAN    0.03f

struct App : SlimApp {
    Canvas canvas;

    // HUD:
    HUDLine X_HUD{ (char*)"X  : "};
    HUDLine Y_HUD{(char*)"Y : "};
    HUDLine ElevationHUD{ (char*)"Elevation  : "};
    HUDLine AimHUD{(char*)"Aim : "};
    HUDSettings hud_settings{4};
    HUD hud{hud_settings, &X_HUD};

    struct {
        struct {
            f32 turn{   NAVIGATION_SPEED_DEFAULT__TURN  };
            f32 zoom{   NAVIGATION_SPEED_DEFAULT__ZOOM  };
            f32 dolly{  NAVIGATION_SPEED_DEFAULT__DOLLY };
            f32 pan{    NAVIGATION_SPEED_DEFAULT__PAN   };
            f32 orbit{  NAVIGATION_SPEED_DEFAULT__ORBIT };
            f32 orient{ NAVIGATION_SPEED_DEFAULT__ORIENT};
        } speed;
        f32 max_velocity{NAVIGATION_DEFAULT__MAX_VELOCITY};
        f32 acceleration{NAVIGATION_DEFAULT__ACCELERATION};
    } settings;

    Move move{};
    Turn turn{};

    mat2 rotation{};
    vec2 &right = rotation.X;
    vec2 &forward = rotation.Y;
    vec2 position{118.555f, 232.405f}, current_velocity{0.0f}, far_right, far_left;
    f32 elevation = 12100.0f, vertical_aim = -37.2f, focal_length = 2.0f;
    u16 far_distance = 1000;

    bool moved{false};
    bool turned{false};
    bool use_images{false};

    char* color_texture_file_name = (char*)"color_map.texture";
    char* height_texture_file_name = (char*)"height_map.texture";
    char* color_map_file_name = (char*)"color_map.image";
    char* height_map_file_name = (char*)"height_map.image";
    ByteColorImage color_map, height_map;
    ImagePack<ByteColor> images{2, &color_map, &color_map_file_name, (char*)__FILE__};
    Texture color_texture, height_texture;
    TexturePack textures{2, &color_texture, &color_texture_file_name, (char*)__FILE__, Terabytes(4)};

    void OnUpdate(float delta_time) override {
        // Navigate:
        vec2 target_velocity;
        if (move.right)    target_velocity.x += settings.max_velocity;
        if (move.left)     target_velocity.x -= settings.max_velocity;
        if (move.forward)  target_velocity.y += settings.max_velocity;
        if (move.backward) target_velocity.y -= settings.max_velocity;
        if (move.up)       elevation += 3.0f * settings.max_velocity;
        if (move.down)     elevation -= 3.0f * settings.max_velocity;

        if (turn.left) {
            rotation.rotate(delta_time * settings.speed.turn);
            turned = true;
        }
        if (turn.right) {
            rotation.rotate(delta_time * -settings.speed.turn);
            turned = true;
        }

        // Update the current speed_x and position_x:
        current_velocity = current_velocity.approachTo(target_velocity, settings.acceleration * delta_time);
        vec2 movement = current_velocity * delta_time;
        moved = movement.nonZero();
        if (moved) position += rotation * movement;

        if (mouse::moved) {
            // Orient:
            rotation.rotate(settings.speed.orient * (f32)mouse::pos_raw_diff_x);
            vertical_aim += settings.speed.orient * 100.0f * -(f32)mouse::pos_raw_diff_y;
            mouse::moved = false;
            turned = true;
        }

        far_right = forward.scaleAdd(focal_length * far_distance, right.scaleAdd((f32)far_distance, position));
        far_left = forward.scaleAdd(focal_length * far_distance, right.scaleAdd(-(f32)far_distance, position));

        X_HUD.value = position.x;
        Y_HUD.value = position.y;
        ElevationHUD.value = elevation;
        AimHUD.value = vertical_aim;
    }

    void OnRender() override {
        Color background_color_mid = White;
        Color background_color_top = BrightCyan;
        Color background_color;
        for (u16 y = 0; y < canvas.dimensions.height; y++) {
            background_color = background_color_top.lerpTo(background_color_mid,  y / (0.3f * canvas.dimensions.f_height) + 5.0f / vertical_aim);// - 0.5f / vertical_aim);
            for (u16 x = 0; x < canvas.dimensions.width; x++) {
                canvas.pixels[y * canvas.dimensions.width + x].color = background_color;
            }
        }

        ByteColor* heights = height_map.content;
        ByteColor* colors = color_map.content;
        ByteColor color;

        i32 Hh = height_map.height;
        i32 Hw = height_map.width;
        i32 Ch = color_map.height;
        i32 Cw = color_map.width;
        f32 u, v;

        vec2 column = far_left;
        vec2 column_step = (far_right - far_left) / (f32)canvas.dimensions.width;// * (canvas.dimensions.f_width / (2 * far_distance));
        vec2 step, current;
        i32 X, Y;
        f32 max_projected_elevation, projected_elevation, sampled_elevation;
        for (u16 x = 0; x < canvas.dimensions.width; x++) {
            max_projected_elevation = 0;
            step = (column - position) / (f32)far_distance;
            current = position;
            for (u16 z = 1; z <= far_distance; z++) {
                if (use_images) {
                    X = (i32)current.x;
                    Y = (i32)current.y;
                    if (X < 0 || X >= Hw) X = (X + 100 * Hw) % Hw;
                    if (Y < 0 || Y >= Hh) Y = (Y + 100 * Hh) % Hh;
                    color = heights[Hw * Y + X];
                } else {
                    u = current.x / Hw;
                    v = current.y / Hh;
                    if (u < 0) u += 100.0f;
                    if (u > 1) u -= (f32) ((i32) u);
                    if (v < 0) v += 100.0f;
                    if (v > 1) v -= (f32) ((i32) v);
                    color = height_texture.mips[0].sample(u, v).color.toByteColor();
                }
                sampled_elevation = (f32)color.R * 50.0f;// - (vertical_aim * 10.0f * z);
                projected_elevation = ((f32)sampled_elevation - elevation) / (f32)z - vertical_aim*10.f;
                if (projected_elevation > max_projected_elevation) {
                    if (use_images) {
                        X = (i32)current.x * 2;
                        Y = (i32)current.y * 2;
                        if (X < 0 || X >= Cw) X = (X + 100 * Cw) % Cw;
                        if (Y < 0 || Y >= Ch) Y = (Y + 100 * Ch) % Ch;
                        color = colors[Cw * Y + X];
                    } else {
                        u = current.x * 2.0f / Cw;
                        v = current.y * 2.0f / Ch;
                        if (u < 0) u += 100.0f; if (u > 1) u -= (f32)((i32)u);
                        if (v < 0) v += 100.0f; if (v > 1) v -= (f32)((i32)v);
                        color  = color_texture.mips[1].sample(u, v).color.toByteColor();
                    }

                    for (i32 y = (i32)max_projected_elevation; y < (i32)projected_elevation; y++) {
                        if (y >= 0 && y < canvas.dimensions.height)
                            canvas.pixels[(canvas.dimensions.height - y) * canvas.dimensions.width + x].color = color;
                    }

                    max_projected_elevation = projected_elevation;
                }

                current += step;
            }
            column += column_step;
        }

        canvas.drawToWindow();
    }

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (key == 'Q') turn.left     = is_pressed;
        if (key == 'E') turn.right    = is_pressed;
        if (key == 'R') move.up       = is_pressed;
        if (key == 'F') move.down     = is_pressed;
        if (key == 'W') move.forward  = is_pressed;
        if (key == 'S') move.backward = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'D') move.right    = is_pressed;
        if (key == 'T' && !is_pressed) use_images = !use_images;
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }
};

SlimApp* createApp() {
    return new App();
}