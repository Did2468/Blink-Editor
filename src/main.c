#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "terminal.h"
#include "fileio.h"
#include "mouse.h"
#include "render.h"
#include "input.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Blink: Distraction-free Terminal Text Editor\n");
        fprintf(stderr, "Usage: blink <filename>\n");
        exit(1);
    }

    // Toggle raw unbuffered input on standard input descriptor
    enableRawMode();

    // Initialize bounds, sizes, modes, and parameters
    initEditor();

    // Loading the file
    editorOpen(argv[1]);

    //Mouse functionalities
    enableMouseTracking();

    
    setStatusMessage("Blink Ready! VIEW mode. Press 'e' to edit, Double ESC to Close.");

    // loop
    while (1) {
        // Double-buffered screen refresh transaction
        editorRefreshScreen();

        // Check and parse incoming keyboard triggers
        editorProcessKeypress();
    }

    return 0;
}
