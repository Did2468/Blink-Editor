#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "terminal.h"
#include "editor.h"

// Restore standard Cooked mode on exit or crash.
void disableRawMode(void) {
    // Disable mouse tracking before leaving raw terminal mode
    printf("\x1b[?1002l"); // Disable all mouse events
    printf("\x1b[?1000l"); // Disable click tracking
    fflush(stdout);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        perror("disableRawMode: tcsetattr failed");
    }
}

// Enable unbuffered raw binary inputs using termios.
void enableRawMode(void) {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
        perror("enableRawMode: tcgetattr failed");
        exit(1);
    }

    // Register clean up function to exit gracefully
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Disables:
    //  OPOST: Output post-processing (translates \n to \r\n, etc)
    raw.c_oflag &= ~(OPOST);

    // Sets standard 8 bits per character block
    raw.c_cflag |= (CS8);

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Read control parameters: non-blocking configuration.
    raw.c_cc[VMIN] = 0;  // Read returning immediately with 0 bytes minimum or...
    raw.c_cc[VTIME] = 1; // timeout after 100 milliseconds (1/10 second)

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("enableRawMode: tcsetattr failed");
        exit(1);
    }
}

// Measure cursor position manually as shell fallback to find viewport dimensions
static int getCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    // Query terminal positioning
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

// Fetch viewport geometry.
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Query terminal cursor position fallback
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

// Clear visual screen and reset hardware terminal positioning
void clearScreen(void) {
    if (write(STDOUT_FILENO, "\x1b[2J", 4) != 4) {}
    if (write(STDOUT_FILENO, "\x1b[H", 3) != 3) {}
}
