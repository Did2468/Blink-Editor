# Blink Editor Keybindings

Blink is designed as a distraction-free, keyboard-first, and highly intuitive text editor using **strictly two modes**.

---

## 1. View Mode (Default Startup Mode)

When Blink starts, it opens in **View Mode** (Read-Only). Text letters are locked from inputs; arrow navigation and mouse tracking are fully functional.

| Key | Action |
|---|---|
| **`e`** or **`E`** | Enter **Edit Mode** |
| **`s`** or **`S`** | Save File immediately (if file is named) |
| **`ESC`** (once) | Check quit and save options in notification bar |
| **`ESC` `ESC`** (twice) | Safely Quit the editor |
| **`Arrows`** | Move cursor around the text area |
| **`PageUp` / `PageDown`** | Scroll view up or down by page height |
| **`Home` / `End`** | Snap cursor to start or end of current line |
| **`Mouse Click`** | Click to place cursor on any letter |
| **`Mouse Wheel`** | Scroll lines up or down |

---

## 2. Edit Mode (Interactive Typing Mode)

Entered by pressing `e` from View Mode. Status bar reflects **`EDIT`** mode.

| Key | Action |
|---|---|
| **`ESC`** | Return to **View Mode** (Read-Only) |
| **`Standard Characters`** | Insert text character at cursor position |
| **`Enter`** | Insert newline (split line at cursor) |
| **`Tab`** | Insert spaces corresponding to Tab stops (4 spaces) |
| **`Backspace`** | Delete character to the left of the cursor (merges lines if at col 0) |
| **`Delete`** | Delete character to the right of the cursor |
| **`Arrows` / `Home` / `End`** | Move cursor around the text area |
| **`Mouse Click`** | Coordinate-aware mouse clicking to position cursor |
| **`Mouse Wheel`** | Scroll and move viewer viewport |
