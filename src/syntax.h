#ifndef SYNTAX_H
#define SYNTAX_H

#include "editor.h"

// Highlight types
#define HL_NORMAL 0
#define HL_COMMENT 1
#define HL_MLCOMMENT 2
#define HL_KEYWORD1 3  // Main statement keywords (if, while, etc.)
#define HL_KEYWORD2 4  // Types and primitives (int, float, etc.)
#define HL_STRING 5
#define HL_NUMBER 6
#define HL_PREPROC 7   // Preprocessor macros (#include, etc)
#define HL_BUILTIN 8   // Built-in API names (print, etc)

// Flag masks for configuration
#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

// Syntax database accessor
extern editorSyntax HLDB[];
extern const int HLDB_ENTRIES;

void editorUpdateSyntax(erow *row);
const char *editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight(void);

#endif // SYNTAX_H
