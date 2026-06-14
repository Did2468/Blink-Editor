#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"
#include "syntax.h"

// Synchronize raw contents to expanded tab render string and run syntax highlights
void editorUpdateRow(erow *row) {
    int tabs = 0;
    for (int j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') tabs++;
    }

    free(row->render);
    // Each tab can expand up to TAB_STOP spaces
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int idx = 0;
    for (int j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;

    editorUpdateSyntax(row);
}

// Allocates and populates a new row structure in row array of state E.
void editorInsertRow(int at, const char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
    
    // Assign parent line offset indices for recursive syntax tracking
    for (int j = at + 1; j <= E.numrows; j++) E.row[j].cy++;

    E.row[at].cy = at;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;
    E.row[at].hl_open_comment = 0;
    editorUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

void editorFreeRow(erow *row) {
    free(row->chars);
    free(row->render);
    free(row->hl);
}

// Deletes row from E.row array
void editorDelRow(int at) {
    if (at < 0 || at >= E.numrows) return;
    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    
    for (int j = at; j < E.numrows - 1; j++) E.row[j].cy--;
    
    E.numrows--;
    E.dirty++;
}

// Inserts single character into current row at index `at`.
void editorRowInsertChar(erow *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

// Appends string of length `len` onto the end of row raw buffer
void editorRowAppendString(erow *row, const char *s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

// Deletes single character out of current row at index `at`.
void editorRowDelChar(erow *row, int at) {
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

// Key actions on cursor context.
void editorInsertChar(int c) {
    if (E.cy == E.numrows) {
        editorInsertRow(E.numrows, "", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

// Standard Tab key expansions helper
void editorInsertTab(void) {
    for (int i = 0; i < TAB_STOP; i++) {
        editorInsertChar(' ');
    }
}

// Enter Key: splits a line or makes empty rows.
void editorInsertNewline(void) {
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
        E.cy++;
    } else {
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy]; // Re-fetch since row address might have realloc-moved
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
        E.cy++;
        E.cx = 0;
    }
}

// Handles deletion of character left of visual cursor context (Backspace / Delete)
void editorDelChar(void) {
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow *row = &E.row[E.cy];
    if (E.cx > 0) {
        editorRowDelChar(row, E.cx - 1);
        E.cx--;
    } else {
        // Merge this row with previous row
        erow *prev_row = &E.row[E.cy - 1];
        E.cx = prev_row->size;
        editorRowAppendString(prev_row, row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}

// Join the list of row string buffers into one large flat C-string for file saving.
char *editorRowsToString(int *buflen) {
    int totlen = 0;
    for (int j = 0; j < E.numrows; j++) {
        totlen += E.row[j].size + 1;
    }
    *buflen = totlen;

    char *buf = malloc(totlen + 1);
    char *p = buf;
    for (int j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}
