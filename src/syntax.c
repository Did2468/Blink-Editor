#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"

// Define language configuration arrays
static char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
static char *C_HL_keywords[] = {
    // Keywords (HL_KEYWORD1)
    "switch", "if", "else", "while", "for", "break", "continue", "return", "case", "default",
    "goto", "struct", "union", "typedef", "enum", "class", "sizeof", "const", "static", "extern",
    // Types (HL_KEYWORD2)
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", "size_t|",
    "bool|", "short|", "unsigned char|", "unsigned int|", "unsigned long|", "int32_t|", "uint32_t|",
    "int64_t|", "uint64_t|", NULL
};

static char *Py_HL_extensions[] = { ".py", NULL };
static char *Py_HL_keywords[] = {
    // Keywords (HL_KEYWORD1)
    "def", "class", "if", "elif", "else", "while", "for", "in", "return", "import", "from", "as",
    "try", "except", "finally", "raise", "assert", "and", "or", "not", "is", "lambda", "global",
    "nonlocal", "with", "yield", "pass", "None", "True", "False",
    // Built-ins (HL_BUILTIN)
    "print|", "len|", "range|", "str|", "int|", "float|", "list|", "dict|", "set|", "tuple|",
    "dir|", "help|", "open|", "input|", "type|", "enumerate|", "zip|", "any|", "all|", NULL
};

// Global Highlight DB register
editorSyntax HLDB[] = {
    {
        "c",
        C_HL_extensions,
        C_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "python",
        Py_HL_extensions,
        Py_HL_keywords,
        "#", NULL, NULL,
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    }
};

const int HLDB_ENTRIES = sizeof(HLDB) / sizeof(HLDB[0]);

// Determine if a character is a delimiter (separates tokens)
static int is_separator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];:&|!{}", c) != NULL;
}

// Update the syntax-highlight byte array of a given erow.
void editorUpdateSyntax(erow *row) {
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    if (E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;
    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1;
    int in_string = 0;
    int in_comment = (row->cy > 0) ? E.row[row->cy - 1].hl_open_comment : 0;

    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

        //  Handle Single line comments 
        if (scs_len && !in_string && !in_comment) {
            if (strncmp(&row->render[i], scs, scs_len) == 0) {
                memset(&row->hl[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        // Handle Multi-line comments
        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                row->hl[i] = HL_MLCOMMENT;
                if (strncmp(&row->render[i], mce, mce_len) == 0) {
                    memset(&row->hl[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                }
                i++;
                continue;
            } else if (strncmp(&row->render[i], mcs, mcs_len) == 0) {
                memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        //  Handle Strings 
        if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
            if (in_string) {
                row->hl[i] = HL_STRING;
                if (c == '\\' && i + 1 < row->rsize) {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string) in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            } else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        // Handle Numbers and Preprocessors
        if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
            if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
                (c == '.' && prev_hl == HL_NUMBER)) {
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        // C preprocessor highlighting (lines beginning starting with #)
        if (strcmp(E.syntax->filetype, "c") == 0 && c == '#' && prev_sep) {
            // Color sequence to end of identifier or line
            while (i < row->rsize && !isspace(row->render[i])) {
                row->hl[i] = HL_PREPROC;
                i++;
            }
            prev_sep = 0;
            continue;
        }

        // Handle Keywords & Built-ins
        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int is_kw2 = keywords[j][klen - 1] == '|';
                if (is_kw2) klen--;

                if (strncmp(&row->render[i], keywords[j], klen) == 0 &&
                    is_separator(row->render[i + klen])) {
                    memset(&row->hl[i], is_kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(c);
        i++;
    }

    // Save final state of this row's comment termination for processing the next row recursively
    int changed = (row->hl_open_comment != in_comment);
    row->hl_open_comment = in_comment;
    if (changed && row->cy + 1 < E.numrows) {
        editorUpdateSyntax(&E.row[row->cy + 1]);
    }
}

// Convert Syntax ID type into corresponding CSS/ANSI terminal color tokens
const char *editorSyntaxToColor(int hl) {
    switch (hl) {
        case HL_COMMENT:
        case HL_MLCOMMENT:
            return FG_DARK_GRAY; // Gray tones
        case HL_KEYWORD1:
            return FG_YELLOW;    // Yellow
        case HL_KEYWORD2:
            return FG_GREEN;     // Green
        case HL_STRING:
            return FG_CYAN;      // Cyan
        case HL_NUMBER:
            return FG_MAGENTA;   // Magenta
        case HL_PREPROC:
            return FG_RED;       // Red
        case HL_BUILTIN:
            return FG_BLUE;      // Blue
        default:
            return FG_WHITE;
    }
}

// Select matching syntax highlighting configuration based on file matches
void editorSelectSyntaxHighlight(void) {
    E.syntax = NULL;
    if (E.filename == NULL) return;

    char *ext = strrchr(E.filename, '.');

    for (int j = 0; j < HLDB_ENTRIES; j++) {
        editorSyntax *s = &HLDB[j];
        int i = 0;
        while (s->filematch[i]) {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && strcmp(ext, s->filematch[i]) == 0) ||
                (!is_ext && strstr(E.filename, s->filematch[i]))) {
                E.syntax = s;
                // Re-calculate highlights of all existing lines
                for (int r = 0; r < E.numrows; r++) {
                    editorUpdateSyntax(&E.row[r]);
                }
                return;
            }
            i++;
        }
    }
}
