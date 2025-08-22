#pragma once
#include <stdio.h>
#include "../todo.h"
#define DCLOC __FILE__":"STRINGIFY1(__LINE__)
#if 1
#   define dcss_log(level, ...) (fprintf(stderr, "CSS:"level" " DCLOC " " __VA_ARGS__), fprintf(stderr, "\n"))
#else
#   define dcss_log(...)
#endif
#define dcss_err(...)  dcss_log("ERROR", __VA_ARGS__)
#define dcss_todo(...) dcss_log("TODO", __VA_ARGS__)
#define dcss_warn(...) dcss_log("WARN", __VA_ARGS__)
