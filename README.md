<img src="SlimAppCpp_logo.png" alt="SlimAppCpp_logo"><br>

Minimalist and platform-agnostic application layer for interactive graphical applications.<br>
Executables for simple applications are very small - here are the sizes of all the provided example apps:<br><br>
<img src="src/examples/executables.png"><br><br>
Available as either a single header file or a directory of headers (a "unity build" setup).<br>
Written in plain C++. For the original C variant, see [SlimApp](https://github.com/HardCoreCodin/SlimApp)<br>
Focuses on simplicity, ease of use and setup with no dependencies and an optional single header file variant<br>
<br>
Used in: [SlimEngine](https://github.com/HardCoreCodin/SlimEngineCpp) and [SlimRaster](https://github.com/HardCoreCodin/SlimRasterCpp) <br>

For related projects, see [my portfolio](https://hardcorecodin.com/portfolio)

Architecture:
-
SlimApp is platform-agnostic by design, though currently only supports Windows.<br>
The platform layer only uses operating-system headers - no standard library used.<br>
The application layer itself has no 3rd-party dependencies - only uses standard math headers.<br>
It is just a library that the platform layer uses - it has no knowledge of the platform.<br>

More details on this architecture [here](https://youtu.be/Ev_TeQmus68).

Usage:
-
The single header file variant includes everything.<br>
Alternatively, specific headers can be included from the directory of headers.<br>
<br>

SlimApp comes with pre-configured CMake targets for all examples.<br>
For manual builds on Windows, the typical system libraries need to be linked<br>
(winmm.lib, gdi32.lib, shell32.lib, user32.lib) and the SUBSYSTEM needs to be set to WINDOWS<br>

All examples were tested in all combinations of:<br>
Compiler: MSVC, MinGW, CLang<br>
Language: C++<br>
CPU Arch: x86, x64<br>

Features:
-

Well documented example applications cover the features:<br><br>

* <b><u>Barebone Application:</b></u><br><br>
  <img src="src/examples/0_barebone.gif"><br><br>
  All features are opt-in, only the entry point needs to be defined.<br><br>
  It just needs to return a pointer to an instance of a SlimApp derived class:<br><br>
  <img src="src/examples/0_barebone.png"><br><br>
  On its own that yield a valid application with a well-behaved blank window.<br><br>
  <br>

* <b><u>Overridable Defaults: </b></u><br><br>
  <img src="src/examples/1_defaults.gif"><br><br>
  Initial dimensions and title:<br><br>
  <img src="src/examples/1_defaults.png"><br><br>
  <br>

* <b><u>Timers:</b></u><br><br>
  <img src="src/examples/2_time.gif"><br><br>
  To customize application behaviours, override specific methods:<br><br>
  <img src="src/examples/2_time.png"><br><br>
  Timers can be used to track the time it takes to run a block of code.<br><br>
  SlimApp comes with 2 pre-exiting timers for update and render.<br><br>
  <br>

* <b><u>Canvas</b>:</u><br><br>
  <img src="src/examples/3_canvas.gif"><br>
  Canvas objects may be used for basic 2D drawing (lines, rectangles, triangles and circles):<br><br>
  <img src="src/examples/3_shapes.png"><br><br>
  Canvas objects needs to be manually cleared and drawn to the window, as well as be resized along with it:<br><br>
  <img src="src/examples/3_canvas.png"><br><br>
  <br>

* <b><u>Text</b>:</u><br><br>
  <img src="src/examples/4_text.gif"><br><br>
  String and numbers can be drawn as text:<br><br>
  <img src="src/examples/4_text.png"><br><br>
  <br>

* <b><u>Files I/O</b>:</u><br><br>
  <img src="src/examples/5_files.gif"><br><br>
  Files can be read-from and written-to in a platform-agnostic way:<br><br>
  <img src="src/examples/5_files.png"><br><br>
  <br>
 
* <b><u>HUD:</b></u><br><br>
  <img src="src/examples/6_HUD.gif"><br><br>
  A heads-up display (HUD) can be set-up for showing textual overlay information.<br><br>
  HUDs contain a table of lines with Title and Value columns:<br><br> 
  <img src="src/examples/6_HUD_lines.png"><br><br>
  Values can be numeric or strings and can be bound to a boolean for toggling<br><br>
  <img src="src/examples/6_HUD.png"><br><br>
  Line height and text color can be controlled globally or per line<br><br>
  <img src="src/examples/6_HUD_settings.png"><br><br>
  A typical setup might look like this:<br><br>
  <img src="src/examples/6_HUD_setup.png"><br><br>
  Updating the values may look like this:<br><br>
  <img src="src/examples/6_HUD_update.png"><br><br>
  Toggled rendering might look like this:<br><br>
  <img src="src/examples/6_HUD_render.png"><br><br>
  <br>

* <b><u>Mouse</b>:</u><br><br>
  <img src="src/examples/7_mouse.gif"><br><br>
  Mouse input tracks buttons and the wheel, as well as cursor position and movement:<br><br>
  <img src="src/examples/7_mouse.png"><br><br>
  Raw mouse input also tracks mouse movement outside the window:<br><br>  
  <img src="src/examples/7_mouse_raw.png"><br><br>
  <br>

* <b><u>Keyboard</b>:</u><br><br>
  <img src="src/examples/8_keyboard.gif"><br><br>
  <img src="src/examples/8_keyboard_setup.png"><br><br>
  General key tracking + built-in tracking of control keys status (Ctrl, Alt, Shift, Space):<br><br>
  <img src="src/examples/8_keyboard_render.png"><br><br>
  <br>

* <b><u>Game</b>:</u><br><br>
  <img src="src/examples/9_game.gif"><br><br>
  Update and Render loop for game-like programming:<br><br>
  <img src="src/examples/9_game.png"><br><br>
  <br>
 
<b>SlimApp</b> does not come with any GUI functionality at this point.<br>
Some example apps have an optional HUD that shows additional information.<br>
It can be toggled on or off using the`tab` key.<br>
