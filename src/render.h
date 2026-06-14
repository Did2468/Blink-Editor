#ifndef RENDER_H
#define RENDER_H

#include "editor.h"

// Dynamic append-buffer structure for flicker-free double buffering
struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

void editorRefreshScreen(void);
void editorScroll(void);

#endif // RENDER_H
