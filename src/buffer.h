#ifndef BUFFER_H
#define BUFFER_H

#include "editor.h"

// Row modification procedures
void editorUpdateRow(erow *row);
void editorInsertRow(int at, const char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, const char *s, size_t len);
void editorRowDelChar(erow *row, int at);

// Universal editor modifications (operating on current cursor position)
void editorInsertChar(int c);
void editorInsertTab(void);
void editorInsertNewline(void);
void editorDelChar(void);

// Buffer serialization
char *editorRowsToString(int *buflen);

#endif // BUFFER_H
