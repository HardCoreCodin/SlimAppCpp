#include "../slim/app.h"
#include "../slim/math/vec2.h"
#include "../slim/core/hud.h"
#include "../slim/draw/hud.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
//#include "../slim.h"

struct GameWithHUDApp : SlimApp {
    struct Player {
        f32 size = 10;
        f32 speed = 80;
        vec2 pos{300.0f, 300.0f};
    } player;
    Move move{};

    // HUD:
    HUDLine Fps{   (char*)"Fps    : "};
    HUDLine Mfs{   (char*)"Mic-s/f: "};
    HUDLine Width{ (char*)"Width  : "};
    HUDLine Height{(char*)"Height : "};
    HUDLine PlayerX{(char*)"PlayerX: "};
    HUDLine PlayerY{(char*)"PlayerY: "};
    HUDSettings hud_settings{
            6,
            1.2f,
            Green
    };
    HUD hud{hud_settings, &Fps};

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (key == 'W') move.up       = is_pressed;
        if (key == 'S') move.down     = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'D') move.right    = is_pressed;

        if (!is_pressed && key == controls::key_map::tab)
            hud.enabled = !hud.enabled;
    }
    void OnWindowResize(u16 width, u16 height) override {
        Width.value = (i32)width;
        Height.value = (i32)height;
    }
    void OnUpdate(f32 delta_time) override {
        Fps.value = (i32)(
            update_timer.average_frames_per_second +
            render_timer.average_frames_per_second
        );
        Mfs.value = (i32)(
            update_timer.average_microseconds_per_frame +
            render_timer.average_microseconds_per_frame
        );
        PlayerX.value = player.pos.x;
        PlayerY.value = player.pos.y;

        f32 amount = player.speed * delta_time;

        if (move.left)  player.pos.x -= amount;
        if (move.right) player.pos.x += amount;
        if (move.up)    player.pos.y -= amount;
        if (move.down)  player.pos.y += amount;
    }

    void OnRender() override {
        fill({
             (int)(player.pos.x - player.size),
             (int)(player.pos.x + player.size),
             (int)(player.pos.y - player.size),
             (int)(player.pos.y + player.size)
             },
             window::canvas, Blue
        );

        if (hud.enabled) draw(hud, window::canvas);
    }
};

SlimApp* createApp() {
    return new GameWithHUDApp();
}



