#include "./brushXPU.h"

struct DisplacementPainter : SlimApp {
    static constexpr f32 MouseBrushSensitivity = 0.05f;
    static constexpr f32 BrushOpacitySensitivity = 0.01f;

    bool run_on_GPU = true;
    timers::Timer timer;

    HUDLine OperationLine{(char*)"Brush Type    : ", (char*)"Displace", Black};
    HUDLine TimerLine{    (char*)"Micro Seconds : ", Black};
    HUDLine XPULine{      (char*)"CUDA GPU Mode : ", (char*)"On",(char*)"Off",
                      &run_on_GPU, false, Green, Red, Black};
    HUDSettings hud_settings{3};
    HUD hud{hud_settings, &OperationLine};

    Canvas canvas;
    ParticleBrush brush;
    vec2 brush_particle_positions[ParticleBrush::MAX_PARTICLES];

    char *files[2]{(char*)"input.image", (char*)"input.image"};
    Image image, current;
    ImagePack image_pack{2, &image, files, (char*)__FILE__};
    RectI image_bounds{0, (i32)image.width - 1, 0, (i32)image.height - 1};
    vec2 *displacement_map = new vec2[image.width * image.height];


    DisplacementPainter() {
        brush.particle_positions = brush_particle_positions;
        allocateDeviceMemory(image);
        uploadImage(image);
        uploadCurrent(current, displacement_map);
    }

    void OnWindowResize(u16 width, u16 height) override {
        canvas.dimensions.update(width, height);
    }

    void OnRender() override {
        canvas.clear();

        drawImage(current, canvas, image_bounds);

        f32 magnitude;
        vec2 displacement;
        Color color = Black;
        for (u32 y = 0; y < image.height; y++)
            for (u32 x = 0; x < image.width; x++) {
                displacement = displacement_map[(i32)image.width * y + x];
                if (displacement.nonZero()) {
                    magnitude = displacement.length();
                    displacement /= magnitude;
                    color.red   = displacement.x * 0.5f + 0.5f;
                    color.green = displacement.y * 0.5f + 0.5f;
                    color *= magnitude * 0.01f;
                    canvas.pixels[canvas.dimensions.width * (image.height + y) + x].color = color.clamped();
                }
            }

        if (image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            color = controls::is_pressed::ctrl ? Green : Red;
            vec2i at{mouse::pos_x, mouse::pos_y};
            if (mouse::left_button.is_pressed) {
                canvas.drawCircle(mouse::pos_x, mouse::pos_y, (i32)brush.radius, color);
                color = Blue;
                f32 particle_count_factor = 1.0f / (f32)brush.active_particles;
                for (u8 i = 0; i < brush.active_particles; i++) {
                    at.x = (i32)brush.particle_positions[i].x;
                    at.y = (i32)brush.particle_positions[i].y + (i32)image.height;
                    brush.drawToCanvas(canvas, at, color, particle_count_factor);
                }
            } else
                brush.drawToCanvas(canvas, at, color);
        }

        if (hud.enabled) drawHUD(hud, canvas);
        canvas.drawToWindow();

        RectI draw_bounds;
        draw_bounds.left = (i32)image.width;
        draw_bounds.bottom = ParticleBrush::MAX_RADIUS * 2;
        draw_bounds.right = draw_bounds.left + draw_bounds.bottom;
        brush.drawToWindow(draw_bounds);
    }

    void OnUpdate(float delta_time) override {
        if (mouse::wheel_scrolled) {
            f32 by = mouse::wheel_scroll_amount * MouseBrushSensitivity;
            if (controls::is_pressed::shift) {
                by *= BrushOpacitySensitivity;
                brush.opacity =  clampedValue(by + brush.opacity);
            } else
                brush.radius = clampedValue(by + brush.radius, (f32)ParticleBrush::MAX_RADIUS);
        }

        if ((controls::is_pressed::ctrl || mouse::moved) && mouse::left_button.is_pressed && image_bounds.contains(mouse::pos_x, mouse::pos_y)) {
            brush.movement = {mouse::movement_x, mouse::movement_y};
            vec2 position{mouse::pos_x, mouse::pos_y};
            position -= brush.movement;

            i32 count = (i32)brush.movement.length();
            if (count > ParticleBrush::MAX_PARTICLES) count = ParticleBrush::MAX_PARTICLES;
            if (count < 1) count = 1;
            brush.movement /= (f32)count;

            for (u8 i = 0; i < count; i++, position += brush.movement) brush.particle_positions[i] = position;
            if (run_on_GPU) uploadBrushParticlePositions(brush.particle_positions);

            brush.movement *= brush.opacity;
            brush.active_particles = (u8)count;

            RectI relevant_bounds{
                mouse::pos_x - mouse::movement_x,
                mouse::pos_x,
                mouse::pos_y - mouse::movement_y,
                mouse::pos_y,
            };
            relevant_bounds.left -= (i32)brush.radius;
            relevant_bounds.right += (i32)brush.radius;
            relevant_bounds.top -= (i32)brush.radius;
            relevant_bounds.bottom += (i32)brush.radius;
            relevant_bounds -= image_bounds;
            if (!relevant_bounds) return;

            timer.beginFrame();
            runOnXPU(image, current, displacement_map, relevant_bounds, brush, run_on_GPU);
            timer.endFrame();
            TimerLine.value = (i32)timer.microseconds;
        }
    }

    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (!is_pressed) {
            if (     key == '1') brush.operation = ParticleBrush::Operation::Displace;
            else if (key == '2') brush.operation = ParticleBrush::Operation::Undisplace;
            else if (key == '3') brush.operation = ParticleBrush::Operation::Pinch;
            else if (key == '4') brush.operation = ParticleBrush::Operation::Expand;
            else if (key == '5') brush.operation = ParticleBrush::Operation::Twirl;
            else if (key == controls::key_map::tab)
                hud.enabled = !hud.enabled;
            else if (key == 'X') {
                run_on_GPU = (USE_GPU_BY_DEFAULT ? !run_on_GPU : false);
                if (run_on_GPU) {
                    uploadCurrent(current, displacement_map);
                    uploadBrushParticlePositions(brush.particle_positions);
                }
            }
            switch (brush.operation) {
                case ParticleBrush::Operation::Displace: OperationLine.value.string.char_ptr = (char*)"Displace"; break;
                case ParticleBrush::Operation::Undisplace: OperationLine.value.string.char_ptr = (char*)"Undisplace"; break;
                case ParticleBrush::Operation::Pinch: OperationLine.value.string.char_ptr = (char*)"Pinch"; break;
                case ParticleBrush::Operation::Expand: OperationLine.value.string.char_ptr = (char*)"Expand"; break;
                case ParticleBrush::Operation::Twirl: OperationLine.value.string.char_ptr = (char*)"Twirl"; break;
            }
        }
    }
};

SlimApp* createApp() {
    return new DisplacementPainter();
}