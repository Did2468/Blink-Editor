#ifndef INPUT_H
#define INPUT_H

// Read a single code or escape sequence from the terminal
int editorReadKey(void);

// Process a received terminal input action (keyboard/mouse)
void editorProcessKeypress(void);

#endif // INPUT_H
