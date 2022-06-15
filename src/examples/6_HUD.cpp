#include "../slim/app.h"
#include "../slim/core/hud.h"
#include "../slim/draw/hud.h"
// Or using the single-header file:
//#include "../slim.h"

struct HUDApp : SlimApp {
    // HUD:
    HUDLine Fps{   (char*)"Fps    : "};
    HUDLine Mfs{   (char*)"Mic-s/f: "};
    HUDLine Width{ (char*)"Width  : "};
    HUDLine Height{(char*)"Height : "};
    HUDSettings hud_settings{
            4,
            1.2f,
            Green
    };
    HUD hud{hud_settings, &Fps};

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (!is_pressed && key == controls::key_map::tab)
            hud.enabled = !hud.enabled;
    }
    void OnRender() override {
        if (hud.enabled)
            draw(hud, window::canvas);
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
    }
};

SlimApp* createApp() {
    return new HUDApp();
}



