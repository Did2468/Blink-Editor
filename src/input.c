#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "input.h"
#include "editor.h"
#include "buffer.h"
#include "fileio.h"
#include "mouse.h"
#include "render.h"
#include "terminal.h"

// Define custom mapping code numbers for virtual non-printable events
enum editorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    MOUSE_EVENT
};

// Store mouse coordinate variables locally within the input transaction
static int mouse_btn = 0;
static int mouse_col = 0;
static int mouse_row = 0;

// Read a single raw keypress or parse a sequential escape array blocking up to VTIME.
int editorReadKey(void) {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            perror("editorReadKey: read error");
            exit(1);
        }
    }

    if (c == '\x1b') {
        char seq[32];
        
        // Non-blocking read to see if there are pending escape sequences
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                } else if (seq[2] == ';') { // More complex modifiers sequence
                    // Read out remaining bytes safely so they do not jam the input buffer
                    char discard[8];
                    int idx = 0;
                    while (idx < 5 && read(STDIN_FILENO, &discard[idx], 1) == 1) {
                        if (discard[idx] == '~' || (discard[idx] >= 'A' && discard[idx] <= 'Z') || (discard[idx] >= 'a' && discard[idx] <= 'z')) {
                            break;
                        }
                        idx++;
                    }
                }
            } else if (seq[1] == '<') {
                // SGR mouse protocols parsing format: \x1b[<button>;<col>;<row>;M/m
                int i = 0;
                char m_ch;
                while (i < (int)sizeof(seq) - 1) {
                    if (read(STDIN_FILENO, &seq[i], 1) != 1) break;
                    if (seq[i] == 'M' || seq[i] == 'm') {
                        m_ch = seq[i];
                        seq[i] = '\0';
                        break;
                    }
                    i++;
                }
                
                // Decode coordinates
                if (sscanf(seq, "%d;%d;%d", &mouse_btn, &mouse_col, &mouse_row) == 3) {
                    if (m_ch == 'M') {
                        return MOUSE_EVENT;
                    }
                }
                return '\0'; // Ignore releases or corrupted mouse sequences
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\0'; // Return null char for unhandled escape tracks
    }

    return c;
}

// Moves cursor within E state bounds depending on direction key pressed.
static void editorMoveCursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx > 0) {
                E.cx--;
            } else if (E.cy > 0) {
                // Wrap left to end of previous line
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size && E.cy < E.numrows - 1) {
                // Wrap right to start of next line
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy > 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows - 1) {
                E.cy++;
            }
            break;
    }

    // Snap cursor x position to line end if line has changed
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}

// Maintain variables to measure sequential Esc keys count and clear alerts
static int escape_press_count = 0;

// High level dispatcher that parses events, coordinates modes, and edits buffers.
void editorProcessKeypress(void) {
    int c = editorReadKey();

    if (c == '\0') return; // Ignore null buffer returns

    // Clear escape sequences tracking if another button is clicked
    if (c != '\x1b') {
        escape_press_count = 0;
    }

    // Handle mouse event immediately
    if (c == MOUSE_EVENT) {
        handleMouseInput(mouse_btn, mouse_col, mouse_row);
        return;
    }

    // Mode-Based Dispatching Logic
    if (E.mode == MODE_VIEW) {
        switch (c) {
            case 'e':
            case 'E':
                E.mode = MODE_EDIT;
                setStatusMessage("-- EDIT MODE -- Press ESC to return to View Mode");
                break;

            case 's':
            case 'S':
                // Safe save trigger under VIEW mode
                editorSave();
                break;

            case '\x1b':
                escape_press_count++;
                if (escape_press_count == 1) {
                    setStatusMessage("Double ESC to QUIT | Press 's' to Save");
                } else if (escape_press_count >= 2) {
                    // Quit editor
                    clearScreen();
                    exit(0);
                }
                break;

            // Handle standard navigation controls
            case ARROW_LEFT:
            case ARROW_RIGHT:
            case ARROW_UP:
            case ARROW_DOWN:
                editorMoveCursor(c);
                break;

            case PAGE_UP:
            case PAGE_DOWN: {
                if (c == PAGE_UP) {
                    E.cy = E.rowoff;
                } else {
                    E.cy = E.rowoff + E.screenrows - 1;
                    if (E.cy >= E.numrows) E.cy = E.numrows - 1;
                }
                int times = E.screenrows;
                while (times--) {
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
                break;
            }

            case HOME_KEY:
                E.cx = 0;
                break;
            case END_KEY:
                if (E.cy < E.numrows) {
                    E.cx = E.row[E.cy].size;
                }
                break;

            default:
                // View mode is read only. Inform the user in the status bar
                setStatusMessage("VIEW Mode (Read Only) - Press 'e' to Edit, Double ESC to Quit");
                break;
        }
    } else if (E.mode == MODE_EDIT) {
        switch (c) {
            case '\x1b':
                // Return to View Mode
                E.mode = MODE_VIEW;
                setStatusMessage("-- VIEW MODE -- Press 'e' to Edit, Double ESC to Quit");
                escape_press_count = 0;
                break;

            case '\r':
                // Enter Key
                editorInsertNewline();
                break;

            case BACKSPACE:
                editorDelChar();
                break;

            case '\t':
                // Expand Tab Character
                editorInsertTab();
                break;

            case DEL_KEY:
                // Delete Key: moves right first and backspaces, removing character right of cursor
                editorMoveCursor(ARROW_RIGHT);
                editorDelChar();
                break;

            // Handle navigation controls
            case ARROW_LEFT:
            case ARROW_RIGHT:
            case ARROW_UP:
            case ARROW_DOWN:
                editorMoveCursor(c);
                break;

            case PAGE_UP:
            case PAGE_DOWN: {
                if (c == PAGE_UP) {
                    E.cy = E.rowoff;
                } else {
                    E.cy = E.rowoff + E.screenrows - 1;
                    if (E.cy >= E.numrows) E.cy = E.numrows - 1;
                }
                int times = E.screenrows;
                while (times--) {
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
                break;
            }

            case HOME_KEY:
                E.cx = 0;
                break;
            case END_KEY:
                if (E.cy < E.numrows) {
                    E.cx = E.row[E.cy].size;
                }
                break;

            default:
                // Filter printable ASCII codes
                if (c >= 32 && c <= 126) {
                    editorInsertChar(c);
                }
                break;
        }
    }
}
