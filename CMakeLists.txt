cmake_minimum_required(VERSION 3.8)

project(0_barebone)
add_executable(0_barebone WIN32 src/examples/0_barebone.cpp)

project(1_defaults)
add_executable(1_defaults WIN32 src/examples/1_defaults.cpp)

project(2_time)
add_executable(2_time WIN32 src/examples/2_time.cpp)

project(3_canvas)
add_executable(3_canvas WIN32 src/examples/3_canvas.cpp)

project(4_text)
add_executable(4_text WIN32 src/examples/4_text.cpp)

project(5_files)
add_executable(5_files WIN32 src/examples/5_files.cpp)

project(6_HUD)
add_executable(6_HUD WIN32 src/examples/6_HUD.cpp)

project(7_mouse)
add_executable(7_mouse WIN32 src/examples/7_mouse.cpp)

project(8_keyboard)
add_executable(8_keyboard WIN32 src/examples/8_keyboard.cpp)

project(9_game)
add_executable(9_game WIN32 src/examples/9_game.cpp)

project(painting)
add_executable(painting WIN32 src/examples/painting.cpp)

project(viz)
add_executable(viz WIN32 src/examples/ray_circle_visualization/viz.cpp)

project(curves)
add_executable(curves WIN32 src/examples/curves.cpp)

project(bmp2texture)
add_executable(bmp2texture src/bmp2texture.cpp)

project(bmp2image)
add_executable(bmp2image src/bmp2image.cpp)

project(TileMap)
add_executable(TileMap WIN32 src/examples/TileMap.cpp)

project(DisplacementPainter)
add_executable(DisplacementPainter WIN32 src/examples/DisplacementPainter/app.cpp)

# For CUDA compilation, uncomment the following lines as-needed
set(CMAKE_CUDA_STANDARD 11)
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 52)
endif()
set(CUDA_VERBOSE_BUILD ON)

project(DisplacementPainterXPU CUDA)
add_executable(DisplacementPainterXPU WIN32 src/examples/DisplacementPainter/app.cu)
set_target_properties(DisplacementPainterXPU PROPERTIES CUDA_SEPARABLE_COMPILATION ON)