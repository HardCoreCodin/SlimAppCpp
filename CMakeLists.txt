cmake_minimum_required(VERSION 3.8)

project(0_barebone)
add_executable(0_barebone WIN32 src/examples/0_barebone.cpp)

project(1_defaults)
add_executable(1_defaults WIN32 src/examples/1_defaults.cpp)

project(2_time)
add_executable(2_time WIN32 src/examples/2_time.cpp)

project(3_shapes)
add_executable(3_shapes WIN32 src/examples/3_shapes.cpp)

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

project(9_interactive)
add_executable(9_interactive WIN32 src/examples/9_game.cpp)