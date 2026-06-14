#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "render.h"

// Draws the primary double-inverted color editor status strip
void editorDrawStatusBar(struct abuf *ab);

// Draws the secondary editor temporary message logs / interactive guides strip
void editorDrawMessageBar(struct abuf *ab);

#endif // STATUSBAR_H
