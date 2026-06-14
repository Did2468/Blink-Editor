# Blink Editor Architecture

This document details the modular systems and design abstractions that make up **Blink**, a modern terminal text editor.

The design emphasizes strict modularity, clean separation of concerns, and an educational, low-overhead construction in standard C99.

---

## Modularity & Components Structure

Blink separates systems cleanly to prevent monolithic cross-coupling. No feature is coupled to the rendering engine; everything uses strict interfaces.

```
                  ┌──────────────────┐
                  │      main.c      │ (Entry Point / Init)
                  └────────┬─────────┘
                           │
             ┌─────────────┼─────────────┐
             ▼             ▼             ▼
      ┌────────────┐ ┌────────────┐ ┌────────────┐
      │ terminal.c │ │  fileio.c  │ │  input.c   │ (Raw Keyboard / Mouse)
      └────────────┘ └─────┬──────┘ └────┬───────┘
                           │             │
                    ┌──────▼──────┐      │
                    │  buffer.c   │◄─────┘ (Row state & modifications)
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  syntax.c   │ (Language Highlight Engine)
                    └──────┬──────┘
                           │
             ┌─────────────┴─────────────┐
             ▼                           ▼
      ┌────────────┐              ┌────────────┐
      │  render.c  │              │statusbar.c │ (Visual Overlays)
      └────────────┘              └────────────┘
```

### Module Descriptions

1. **`main.c`**: Boots the systems, coordinates the file load, calls terminal setup, and executes the central state polling loop.
2. **`config.h`**: Pre-compiler configuration declarations containing general limits, versions, standard keyboard keys, and color constants.
3. **`editor.h` / `editor.c`**: Houses the central `EditorState` model singleton tracking file coordinates, dimensions, modes, paths, and status message lifecycles.
4. **`terminal.h` / `terminal.c`**: Handles raw termios setup, window sizing, fallback geometries, and cleanup on exit via `atexit`.
5. **`buffer.h` / `buffer.c`**: Manages row-by-row lines structure. Allocates, deletes, and modifies character rows, merges strings on backspaces, and formats tabs into spaces.
6. **`fileio.h` / `fileio.c`**: Connects disk files to memory rows. Saves buffers cleanly by collapsing lists to structured flat strings, keeping error paths transparent.
7. **`render.h` / `render.c`**: Renders text efficiently. Implements double buffering using append structures to push updates in single transactions, shielding from page flickers.
8. **`input.h` / `input.c`**: Captures arrow mappings, control characters, mode transformations, and translates decimal SGR coordinates into screen mouse clicks.
9. **`mouse.h` / `mouse.c`**: Manages mouse click coordinate transitions, calculating cursor snaps.
10. **`syntax.h` / `syntax.c`**: Highlights text. Evaluates language arrays (regex-style matches, strings, comments) and stores token ids per character, separated from visual layout logic.
11. **`statusbar.h` / `statusbar.c`**: Handles formatting of the primary metadata ribbon and ephemeral warning notifications.

---

## Central State: `EditorState`

The entire user context is modeled as a unified struct:

- **Cursor Coordinates (`cx`, `cy`)**: Represents indices inside the raw document buffer.
- **Scroll Bounds (`rowoff`, `coloff`)**: Vertical and horizontal offsets of current line ranges within the visual terminal dimensions.
- **Buffer Rows (`row`)**: Array of `erow` models tracking text strings, tab-expanded rendering lines, and syntax formatting byte arrays.
- **Modified Flag (`dirty`)**: Increments with buffer adjustments; clears when saved to disk.
- **Editor Mode (`mode`)**: Strict system state: `MODE_VIEW` or `MODE_EDIT`.

---

## Rendering Pipeline

To ensure maximum redraw performance:
1. Every draw call is written to a dynamic string builder (the `abuf` append stream).
2. The terminal cursor is hidden first to eliminate flickering.
3. Coordinates are drawn starting with line columns, text arrays, status bars, and ephemeral alert diagnostics.
4. The cursor position is moved to its correct coordinate, and the terminal cursor is made visible.
5. The accumulated append stream buffer is flushed to standard output descriptor in a single flat write transaction.
