# Blink Terminal Text Editor

`Blink` is a modular, high-performance, distraction-free terminal text editor written entirely in C (compliant with C99). Designed to feel like a modern, simplified Vim, it is keyboard-first, lightweight, highly readable, and easily extensible text editor inspired from already existing editors like vim and nano but making it really intuitive for first time users.

---

## Technical Architecture & Core Design

Blink is built from the ground up for strict encapsulation and zero-flicker terminal rendering. Key tenets include:
* **Double-Buffered Redraws**: Standard writes to stdout are grouped and double-buffered into a clean memory-appender structure, sending screen modifications as a single packet.
* **Decoupled Language Highlighter**: Supports a robust, registration-based syntax engine. Languages registering keywords, types, strings, numbers, and preprocessor delimiters are automatically formatted without touch-coupling the rendering module.
* **Dynamic Buffering**: Leverages dynamic rows structures translating bytes, horizontal tab layouts, and syntax caches dynamically.

---

## General Features

- **Standard Modes**: Standard start in View mode, switching seamlessly with `e` to Edit mode .
- **Line Numbers**: Left gutter displaying matching line lines, scaling margins dynamically.
- **Visual Status Strip**: Layout details: Mode tracking, unsaved dirty markers, rowcounts, and cursor markers.
- **Dual Syntax Highlighters**: Integrated syntax coloring definitions for **C** (keywords, primitives, preprocessors, strings, comments, decimals) and **Python** (control commands, comments, standard print-types, floats).

---

## Build Actions & Compiling

Blink requires standard standard build utilities (`make`, `gcc`).

### 1. Standard Compiler Recipes
To compile a high-performance, optimized **release release profile**:
```bash
make release
```
Dont worry if it throws some error they are just warnings and not real errors(some linux systems do throw errors)
*Outputs a release build bin `blink` inside the directory.*

### 2. Global Installation
To install `blink` globally on your system so that you can run it from anywhere like `blink <filename>`:
```bash
sudo make install
```
*By default, this installs the executable to `/usr/local/bin/blink`. If you want to install it to a different location (e.g. `~/.local/bin`), you can override the `PREFIX` variable:*
```bash
make install PREFIX=~/.local
```

### 3. Uninstall
To remove the globally installed executable from your system:
```bash
sudo make uninstall
```

### 4. Clean Up
To remove intermediate compile caches and executable files:
```bash
make clean
```

---

## Launch & Usage

Syntax:
```bash
blink <filename>
```

Examples:
```bash
# Open an existing C source file:
blink main.c

# Create or work with a Python script:
blink script.py
```

---

## Editor Keybindings Summary

| Action | VIEW Mode | EDIT Mode |
|---|---|---|
| **Edit Mode Toggle** | Press `e` / `E` | *(Already in Edit Mode)* |
| **Return to View Mode** | *(Already in View Mode)* | Press `ESC` |
| **Insert Text** | *Blocked* | Type any standard letters |
| **Save File** | Press `s` / `S` | *Blocked* (Return to View Mode) |
| **Safe Exit** | Double-press `ESC` | *Blocked* (Return to View Mode) |
| **Navigation** | Arrow keys / Mouse Click | Arrow keys / Mouse Click |

---

## Future Projects Roadmap

We plan to implement the following modular features:
1. **Search & Replace**: Regex-enabled queries and multi-line matching.
2. **Tabbed Buffers**: Cycle through multiple open buffers or split-views.
3. **Theming Core**: Configuration files to map visual color palettes.
4. **Additional Languages**: Registry listings for Go, Rust, Ruby, and Shell scripts.
5. **Flushing**: Currently after exiting the editor the residue can be seen in the terminal screen. I am aim to remove it in next iterations