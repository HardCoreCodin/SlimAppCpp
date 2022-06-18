#pragma once

#ifdef SLIM_ENABLE_CANVAS_HUD_DRAWING
#include "../core/hud.h"
#endif

enum AntiAliasing {
    NoAA,
    MSAA,
    SSAA
};

struct Canvas {
    Dimensions dimensions;
    Pixel *pixels{nullptr};
    f32 *depths{nullptr};

    AntiAliasing antialias{NoAA};

    Canvas() {
        if (memory::canvas_memory_capacity) {
            pixels = (Pixel*)memory::canvas_memory;
            memory::canvas_memory += CANVAS_PIXELS_SIZE;
            memory::canvas_memory_capacity -= CANVAS_PIXELS_SIZE;

            depths = (f32*)memory::canvas_memory;
            memory::canvas_memory += CANVAS_DEPTHS_SIZE;
            memory::canvas_memory_capacity -= CANVAS_DEPTHS_SIZE;
        } else {
            pixels = nullptr;
            depths = nullptr;
        }
    }

    Canvas(Pixel *pixels, f32 *depths) noexcept : pixels{pixels}, depths{depths} {}

    void clear(f32 red = 0, f32 green = 0, f32 blue = 0, f32 opacity = 0, f32 depth = INFINITY) const {
        i32 pixels_width  = dimensions.width;
        i32 pixels_height = dimensions.height;
        i32 depths_width  = dimensions.width;
        i32 depths_height = dimensions.height;

        if (antialias != NoAA) {
            depths_width *= 2;
            depths_height *= 2;
            if (antialias == SSAA) {
                pixels_width *= 2;
                pixels_height *= 2;
            }
        }

        i32 pixels_count = pixels_width * pixels_height;
        i32 depths_count = depths_width * depths_height;

        Pixel pixel{red, green, blue, opacity};

        if (pixels) for (i32 i = 0; i < pixels_count; i++) pixels[i] = pixel;
        if (depths) for (i32 i = 0; i < depths_count; i++) depths[i] = depth;
    }

    void drawToWindow() {
        u32 *content_value = window::content;
        Pixel *pixel = pixels;
        for (u16 y = 0; y < window::height; y++)
            for (u16 x = 0; x < window::width; x++, content_value++) {
                *content_value = getPixelContent(pixel);

                if (antialias == SSAA)
                    pixel += 4;
                else
                    pixel++;
            }
    }

    INLINE void setPixel(i32 x, i32 y, const Color &color, f32 opacity = 1.0f, f32 depth = 0, f32 z_top = 0, f32 z_bottom = 0, f32 z_right = 0) const {
        u32 offset = antialias == SSAA ? ((dimensions.stride * (y >> 1) + (x >> 1)) * 4 + (2 * (y & 1)) + (x & 1)) : (dimensions.stride * y + x);
        Pixel pixel{color, opacity};
        Pixel *out_pixel = pixels + offset;
        f32 *out_depth = depths ? (depths + (antialias == MSAA ? offset * 4 : offset)) : nullptr;
        if (opacity == 1.0f && depth == 0.0f && z_top == 0.0f && z_bottom == 0.0f && z_right == 0.0f) {
            *out_pixel = pixel;
            if (depths) {
                out_depth[0] = 0;
                if (antialias == MSAA) out_depth[1] = out_depth[2] = out_depth[3] = 0;
            }

            return;
        }

        Pixel *bg{out_pixel}, *fg{&pixel};
        if (antialias == MSAA) {
            Pixel accumulated_pixel{};
            for (u8 i = 0; i < 4; i++) {
                if (depths) {
                    if (i) depth = i == 1 ? z_top : (i == 2 ? z_bottom : z_right);
                    _sortPixelsByDepth(depth, &pixel, out_depth, out_pixel, &bg, &fg);
                    out_depth++;
                }
                accumulated_pixel += fg->opacity == 1 ? *fg : fg->alphaBlendOver(*bg);
            }
            *out_pixel = accumulated_pixel * 0.25f;
        } else {
            if (depths) _sortPixelsByDepth(depth, &pixel, out_depth, out_pixel, &bg, &fg);
            *out_pixel = fg->opacity == 1 ? *fg : fg->alphaBlendOver(*bg);
        }
    }

    INLINE u32 getPixelContent(Pixel *pixel) const {
        return antialias == SSAA ?
            _isTransparentPixelQuad(pixel) ? 0 : _blendPixelQuad(pixel).asContent() :
            pixel->opacity ? pixel->asContent(true) : 0;
    }

#ifdef SLIM_ENABLE_CANVAS_TEXT_DRAWING
    INLINE void drawText(char *str, i32 x, i32 y, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #ifdef SLIM_VEC2
    INLINE void drawText(char *str, vec2i position, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawText(char *str, vec2 position, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #endif
#endif

#ifdef SLIM_ENABLE_CANVAS_NUMBER_DRAWING
    INLINE void drawNumber(i32 number, i32 x, i32 y, const Color &color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #ifdef SLIM_VEC2
    INLINE void drawNumber(i32 number, vec2i position, const Color &color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawNumber(i32 number, vec2 position, const Color &color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #endif
#endif

#ifdef SLIM_ENABLE_CANVAS_HUD_DRAWING
    INLINE void drawHUD(const HUD &hud, const RectI *viewport_bounds = nullptr);
#endif

#ifdef SLIM_ENABLE_CANVAS_LINE_DRAWING
    INLINE void drawHLine(RangeI x_range, i32 y, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawHLine(i32 x_start, i32 x_end, i32 y, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawVLine(RangeI y_range, i32 x, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawVLine(i32 y_start, i32 y_end, i32 x, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawLine(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, Color color = White, f32 opacity = 1.0f, u8 line_width = 1, const RectI *viewport_bounds = nullptr);
    INLINE void drawLine(f32 x1, f32 y1, f32 x2, f32 y2, Color color = White, f32 opacity = 1.0f, u8 line_width = 1, const RectI *viewport_bounds = nullptr);

    #ifdef SLIM_VEC2
    INLINE void drawLine(vec2 from, vec2 to, Color color = White, f32 opacity = 1.0f, u8 line_width = 1, const RectI *viewport_bounds = nullptr);
    #endif
#endif

#ifdef SLIM_ENABLE_CANVAS_RECTANGLE_DRAWING
    INLINE void drawRect(RectI rect, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawRect(Rect rect, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void fillRect(RectI rect, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void fillRect(Rect rect, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
#endif

#ifdef SLIM_ENABLE_CANVAS_TRIANGLE_DRAWING
    INLINE void drawTriangle(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, Color color = White, f32 opacity = 1.0f, u8 line_width = 1, const RectI *viewport_bounds = nullptr);
    INLINE void drawTriangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color = White, f32 opacity = 0.5f, u8 line_width = 0, const RectI *viewport_bounds = nullptr);
    INLINE void fillTriangle(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void fillTriangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);

    #ifdef SLIM_VEC2
    INLINE void drawTriangle(vec2 p1, vec2 p2, vec2 p3, Color color = White, f32 opacity = 0.5f, u8 line_width = 0, const RectI *viewport_bounds = nullptr);
    INLINE void fillTriangle(vec2 p1, vec2 p2, vec2 p3, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawTriangle(vec2i p1, vec2i p2, vec2i p3, Color color = White, f32 opacity = 0.5f, u8 line_width = 0, const RectI *viewport_bounds = nullptr);
    INLINE void fillTriangle(vec2i p1, vec2i p2, vec2i p3, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #endif
#endif

#ifdef SLIM_ENABLE_CANVAS_CIRCLE_DRAWING
    INLINE void fillCircle(i32 center_x, i32 center_y, i32 radius, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void drawCircle(i32 center_x, i32 center_y, i32 radius, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #ifdef SLIM_VEC2
    INLINE void drawCircle(vec2i center, i32 radius, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    INLINE void fillCircle(vec2i center, i32 radius, Color color = White, f32 opacity = 1.0f, const RectI *viewport_bounds = nullptr);
    #endif
#endif

private:
    static INLINE bool _isTransparentPixelQuad(Pixel *pixel_quad) {
        return pixel_quad->opacity == 0.0f && pixel_quad[1].opacity == 0.0f  && pixel_quad[2].opacity == 0.0f  && pixel_quad[3].opacity == 0.0f;
    }

    static INLINE Pixel _blendPixelQuad(Pixel *pixel_quad) {
        Pixel TL{pixel_quad[0]}, TR{pixel_quad[1]}, BL{pixel_quad[2]}, BR{pixel_quad[3]};
        TL.opacity *= 0.25f;
        TR.opacity *= 0.25f;
        BL.opacity *= 0.25f;
        BR.opacity *= 0.25f;
        return {
            {
                (
                    (TL.color.r * TL.opacity) +
                    (TR.color.r * TR.opacity) +
                    (BL.color.r * BL.opacity) +
                    (BR.color.r * BR.opacity)
                ),
                (
                    (TL.color.g * TL.opacity) +
                    (TR.color.g * TR.opacity) +
                    (BL.color.g * BL.opacity) +
                    (BR.color.g * BR.opacity)
                ),
                (
                    (TL.color.b * TL.opacity) +
                    (TR.color.b * TR.opacity) +
                    (BL.color.b * BL.opacity) +
                    (BR.color.b * BR.opacity)
                )
            },
            (
                TL.opacity +
                TR.opacity +
                BL.opacity +
                BR.opacity
            )
        };
    }

    static INLINE void _sortPixelsByDepth(f32 depth, Pixel *pixel, f32 *out_depth, Pixel *out_pixel, Pixel **background, Pixel **foreground) {
        if (depth == 0.0f || depth < *out_depth) {
            *out_depth = depth;
            *background = out_pixel;
            *foreground = pixel;
        } else {
            *background = pixel;
            *foreground = out_pixel;
        }
    }
};