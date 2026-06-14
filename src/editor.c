#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "editor.h"
#include "terminal.h"

// Initialize singleton instance of EditorState
EditorState E;

void initEditor(void) {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.mode = MODE_VIEW;
    E.syntax = NULL;

    int rows, cols;
    if (getWindowSize(&rows, &cols) == -1) {
        // Fallback default grid
        rows = 24;
        cols = 80;
    }
    // Reserve bottom lines: 1 for the status bar, 1 for status messages
    E.screenrows = rows - 2;
    E.screencols = cols;
}

// Convert a row character index (cx) into visual rendering index (rx)
// by measuring tab widths.
int editorRowCxToRx(erow *row, int cx) {
    int rx = 0;
    for (int j = 0; j < cx; j++) {
        if (row->chars[j] == '\t') {
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        }
        rx++;
    }
    return rx;
}

// Translate visual Column offset (rx) back into raw row buffer byte offset (cx)
int editorRowRxToCx(erow *row, int rx) {
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t') {
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        }
        cur_rx++;
        if (cur_rx > rx) return cx;
    }
    return cx;
}

// Raise formatted temporary user alerts at the bottom of the editor
void setStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}
