#ifndef EDITOR_H
#define EDITOR_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <time.h>
#include <stdarg.h>
#include "config.h"

// Define basic syntax structure forward-declaration to avoid cyclic dependency
typedef struct editorSyntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    int flags;
} editorSyntax;

// Row representation
typedef struct {
    int cy;            // Row index in file (needed for recursive syntax highlighting)
    int size;          // Raw characters count
    int rsize;         // Rendered characters count (tabs expanded)
    char *chars;       // Pointer to raw row string
    char *render;      // Pointer to rendered character string
    unsigned char *hl; // Highlights array
    int hl_open_comment; 
} erow;

// Global editor state
typedef struct {
    int cx, cy;            
    int rx;             
    int rowoff;            
    int coloff;            
    int screenrows;        
    int screencols;       
    int numrows;          
    erow *row;           
    int dirty;             
    char *filename;        
    char statusmsg[120]; 
    time_t statusmsg_time;
    Mode mode;             
    editorSyntax *syntax;  // Language highlight configuration
    struct termios orig_termios; // original termios terminal configurations
} EditorState;

// External global declaration of Singleton State.
extern EditorState E;

void initEditor(void);
void setStatusMessage(const char *fmt, ...);
int editorRowCxToRx(erow *row, int cx);
int editorRowRxToCx(erow *row, int rx);

#endif 


