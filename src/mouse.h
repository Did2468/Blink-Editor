#ifndef MOUSE_H
#define MOUSE_H

void enableMouseTracking(void);
void disableMouseTracking(void);

// Handle mouse clicks/scrolls coordinates and buttons mapping
void handleMouseInput(int btn, int col, int row);

#endif // MOUSE_H
