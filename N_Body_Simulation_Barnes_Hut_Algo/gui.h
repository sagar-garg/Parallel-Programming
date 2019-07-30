#ifndef _GUI_H
#define _GUI_H

// Create and show a window. You should not need to call this.
void gui_create_window(int argc, char** argv);

// Draw a single frame. This draws circles_count circles onto the screen, where each circle consists
// of three floats: (x, y, r), with x and y being the position and r the radius.
// You also should not need to call this, as it is called already from within move_objects. (Sneaky,
// I know.)
void gui_draw(float* circles, int circles_count);

#endif
