#include <stdio.h>
#include <string.h>
#include <time.h>
#include "statusbar.h"
#include "editor.h"

void editorDrawStatusBar(struct abuf *ab) {
    // Invert colors for status styling: "\x1b[7m" activates negative image
    abAppend(ab, "\x1b[7m", 4);

    // Prepare information strings
    char status[80], rstatus[80];
    const char *mode_str = (E.mode == MODE_EDIT) ? "EDIT" : "VIEW";
    
    int len = snprintf(status, sizeof(status), " Blink | %s | %s%s",
                       mode_str,
                       E.filename ? E.filename : "[No File]",
                       E.dirty ? " [Modified]" : "");
    
    int rlen = snprintf(rstatus, sizeof(rstatus), "Ln %d Col %d | %d rows ",
                        E.cy + 1, E.cx + 1, E.numrows);

    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);

    // Padding between left and right components of status strip
    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }

    // Reset rendering attributes: "\x1b[m" returns terminal attributes to default
    abAppend(ab, "\x1b[m\r\n", 5);
}

void editorDrawMessageBar(struct abuf *ab) {
    // Clear status lines: erase sequence "\x1b[K" cleans right of visual cursor
    abAppend(ab, "\x1b[K", 3);
    
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;

    // Output message ONLY if under five seconds old.
    if (msglen && (time(NULL) - E.statusmsg_time < 5)) {
        abAppend(ab, E.statusmsg, msglen);
    }
}
