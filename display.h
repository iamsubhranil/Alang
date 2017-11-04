#ifndef DISPLAY_H
#define DISPLAY_H

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define error(x) (ANSI_COLOR_RED "\n[Error] " x ANSI_COLOR_RESET)
#define fatal(x) (ANSI_COLOR_RED "\n[Fatal] " x ANSI_COLOR_RESET)
#define line_error(x) (ANSI_COLOR_RED "\n[Error] [Line:%d] " x ANSI_COLOR_RESET)
#define runtime_error(x) (ANSI_COLOR_RED "\n[Runtime Error] [Line:%d] " x ANSI_COLOR_RESET)
#define warning(x) (ANSI_COLOR_YELLOW "\n[Warning] " x ANSI_COLOR_RESET)
#define info(x) (ANSI_COLOR_BLUE "\n[Info] " x ANSI_COLOR_RESET)
#define debug(x) (ANSI_COLOR_GREEN "\n[Debug] " x ANSI_COLOR_RESET)

#endif
