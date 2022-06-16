#define SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING

#include "../slim/math/vec2.h"
#include "../slim/app.h"
#include "../slim/draw/rectangle.h"
// Or using the single-header file:
//#include "../slim.h"

struct GameApp : SlimApp {
    struct Player {
        f32 size = 10;
        f32 speed = 80;
        vec2 pos{20.0f, 20.0f};
    } player;
    Move move{};

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (key == 'W') move.up       = is_pressed;
        if (key == 'S') move.down     = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'D') move.right    = is_pressed;
    }
    void OnUpdate(f32 delta_time) override {
        f32 amount = player.speed * delta_time;

        if (move.left)  player.pos.x -= amount;
        if (move.right) player.pos.x += amount;
        if (move.up)    player.pos.y -= amount;
        if (move.down)  player.pos.y += amount;
    }
    void OnRender() override {
        RectI rect{
            (int)(player.pos.x - player.size),
            (int)(player.pos.x + player.size),
            (int)(player.pos.y - player.size),
            (int)(player.pos.y + player.size)
        };
        window::canvas.fillRect(rect, Blue);
    }
};

SlimApp* createApp() {
    return new GameApp();
}



