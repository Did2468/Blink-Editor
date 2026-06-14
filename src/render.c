#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "render.h"
#include "config.h"
#include "statusbar.h"
#include "syntax.h"

// Append buffer utilities for double buffering
void abAppend(struct abuf *ab, const char *s, int len) {
    char *new_b = realloc(ab->b, ab->len + len);
    if (new_b == NULL) return;
    memcpy(&new_b[ab->len], s, len);
    ab->b = new_b;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}

// Compute vertical and horizontal scroll bounds relative to active cursor index
void editorScroll(void) {
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    // Vertical Scroll
    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }

    // Horizontal Scroll
    int gutter_width = 4;
    if (E.numrows >= 1000) gutter_width = 6;
    else if (E.numrows >= 100) gutter_width = 5;

    int usable_cols = E.screencols - gutter_width;

    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + usable_cols) {
        E.coloff = E.rx - usable_cols + 1;
    }
}

// Refresh visual viewport utilizing advanced ANSI sequences and double buffering.
void editorRefreshScreen(void) {
    editorScroll();

    struct abuf ab = ABUF_INIT;

    // \x1b[?25l: Hide cursor to prevent flicker
    abAppend(&ab, "\x1b[?25l", 6);
    // \x1b[H: Move cursor to home (top-left) position
    abAppend(&ab, "\x1b[H", 3);

    // Calculate gutter width based on document rows count
    int gutter_width = 4;
    if (E.numrows >= 1000) gutter_width = 6;
    else if (E.numrows >= 100) gutter_width = 5;

    for (int y = 0; y < E.screenrows; y++) {
        int file_row = y + E.rowoff;

        if (file_row >= E.numrows) {
            // Draw welcoming splash page when opening empty buffer
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                    "Blink Text Editor -- Version %s", BLINK_VERSION);
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(&ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(&ab, " ", 1);
                abAppend(&ab, welcome, welcomelen);
            } else {
                abAppend(&ab, "~", 1);
            }
        } else {
            erow *row = &E.row[file_row];

            //  Draw C-styled Line Number Row with high contrast gray gutter format
            char num_buf[16];
            snprintf(num_buf, sizeof(num_buf), "%*d │ ", gutter_width - 3, file_row + 1);
            abAppend(&ab, FG_DARK_GRAY, 5);
            abAppend(&ab, num_buf, strlen(num_buf));
            abAppend(&ab, ANSI_RESET, 4);   // Restores default formatting

            // Render sliced characters with active syntax highlighting logic
            int len = row->rsize - E.coloff;
            if (len < 0) len = 0;
            int usable_cols = E.screencols - gutter_width;
            if (len > usable_cols) len = usable_cols;

            char *c = &row->render[E.coloff];
            unsigned char *hl = &row->hl[E.coloff];
            const char *current_color = NULL;

            for (int i = 0; i < len; i++) {
                if (iscntrl(c[i])) {
                    // Controls characters visual feedback: Translate control characters to legible letters
                    char sym = (c[i] <= 26) ? '@' + c[i] : '?';
                    abAppend(&ab, "\x1b[7m", 4); // Reverse video
                    abAppend(&ab, &sym, 1);
                    abAppend(&ab, "\x1b[m", 3);
                    if (current_color) {
                        abAppend(&ab, current_color, strlen(current_color));
                    }
                } else {
                    const char *color = editorSyntaxToColor(hl[i]);
                    if (color != current_color) {
                        abAppend(&ab, color, strlen(color));
                        current_color = color;
                    }
                    abAppend(&ab, &c[i], 1);
                }
            }
            abAppend(&ab, ANSI_RESET, 4); // Reset coloring tags for safety
        }

        // Erase right of current terminal cursor line
        abAppend(&ab, "\x1b[K", 3);
        abAppend(&ab, "\r\n", 2);
    }

    //  Draw status and temporary terminal alerts
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    //  Reposition HW visual cursor to active (cx, cy) visual terminal alignment
    char buf[32];
    int visual_cx = E.rx - E.coloff + gutter_width + 1;
    int visual_cy = E.cy - E.rowoff + 1;
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", visual_cy, visual_cx);
    abAppend(&ab, buf, strlen(buf));

    // \x1b[?25h: Re-enable terminal cursor visibility
    abAppend(&ab, "\x1b[?25h", 6);

    // Write double-buffered rendering array string as one flat transaction
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
