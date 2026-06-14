#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fileio.h"
#include "buffer.h"
#include "editor.h"
#include "syntax.h"

// Load specified file from disk into editor state buffer
void editorOpen(const char *filename) {
    free(E.filename);
    E.filename = strdup(filename);

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        // File does not exist yet (or read permission issue).
        // This is perfectly expected for new file creation.
        setStatusMessage("New Buffer: %s", filename);
        return;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        // Strip trailing return carriage block or newlines
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
            linelen--;
        }
        editorInsertRow(E.numrows, line, linelen);
    }

    free(line);
    fclose(fp);
    E.dirty = 0; // Clear dirty flag since we loaded a fresh file
    setStatusMessage("Opened %s - %d rows", filename, E.numrows);
}

// Persist current editor buffer rows onto disk
void editorSave(void) {
    if (E.filename == NULL) {
        setStatusMessage("Error: No file is coupled. Cannot save.");
        return;
    }

    int len;
    char *buf = editorRowsToString(&len);

    // Save to disk directly
    FILE *fp = fopen(E.filename, "w");
    if (fp != NULL) {
        if (fwrite(buf, 1, len, fp) == (size_t)len) {
            fclose(fp);
            free(buf);
            E.dirty = 0;
            setStatusMessage("Saved to %s (%d bytes)", E.filename, len);
            return;
        }
        fclose(fp);
    }

    free(buf);
    setStatusMessage("Error saving file! Write failed.");
}
