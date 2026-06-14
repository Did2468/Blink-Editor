#include <stdio.h>
#include "mouse.h"
#include "editor.h"
#include "buffer.h"

// Enable standard Mouse protocols (SGR mouse mode 1006 is standard on modern terminals)
void enableMouseTracking(void) {
    printf("\x1b[?1000h\x1b[?1006h");
    fflush(stdout);
}

void disableMouseTracking(void) {
    printf("\x1b[?1006l\x1b[?1000l");
    fflush(stdout);
}

// Convert click events on the screen to cursor position offsets in the active row buffer.
void handleMouseInput(int btn, int col, int row) {
    if (btn == 64) {
        // Scroll Up
        if (E.rowoff > 0) {
            E.rowoff--;
            if (E.cy > 0) E.cy--;
        }
        return;
    } else if (btn == 65) {
        // Scroll Down
        if (E.rowoff + E.screenrows < E.numrows) {
            E.rowoff++;
            if (E.cy + 1 < E.numrows) E.cy++;
        }
        return;
    }

    // Only process Left Clicks (button 0 in SGR) on active rows
    if (btn != 0) return;

    // Convert terminal row (1-indexed) to editor buffer row (0-indexed)
    int target_row = E.rowoff + (row - 1);
    
    // Safety check
    if (target_row < 0) target_row = 0;
    if (target_row >= E.numrows) target_row = E.numrows - 1;

    int gutter_width = 4;
    if (E.numrows >= 1000) gutter_width = 6;
    else if (E.numrows >= 100) gutter_width = 5;

    int target_col_visual = E.coloff + (col - 1 - gutter_width);
    if (target_col_visual < 0) target_col_visual = 0;

    E.cy = target_row;

    if (E.cy < E.numrows) {
        erow *r = &E.row[E.cy];
        E.cx = editorRowRxToCx(r, target_col_visual);
        if (E.cx > r->size) E.cx = r->size;
    } else {
        E.cx = 0;
    }
}
