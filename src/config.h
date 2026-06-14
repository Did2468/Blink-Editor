#ifndef CONFIG_H
#define CONFIG_H

#define BLINK_VERSION "1.0.0"
#define TAB_STOP 4

// Editor modes
typedef enum {
    MODE_VIEW,
    MODE_EDIT
} Mode;

// ANSI Colors and Styling
#define ANSI_RESET          "\x1b[0m"
#define ANSI_BOLD           "\x1b[1m"
#define ANSI_DIM            "\x1b[2m"
#define ANSI_UNDERLINE      "\x1b[4m"
#define ANSI_REVERSE        "\x1b[7m"

// Highlight color escape codes
#define FG_CYAN             "\x1b[36m"
#define FG_YELLOW           "\x1b[33m"
#define FG_BLUE             "\x1b[34m"
#define FG_GREEN            "\x1b[32m"
#define FG_MAGENTA          "\x1b[35m"
#define FG_RED              "\x1b[31m"
#define FG_WHITE            "\x1b[37m"
#define FG_DARK_GRAY        "\x1b[90m"

#endif // CONFIG_H
