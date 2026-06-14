#ifndef TERMINAL_H
#define TERMINAL_H

void enableRawMode(void);
void disableRawMode(void);
int getWindowSize(int *rows, int *cols);
void clearScreen(void);

#endif // TERMINAL_H
