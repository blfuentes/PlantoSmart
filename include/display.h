#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <stdbool.h>
#include <ssd1306.h>

#define DISPLAY_BUFFER_SIZE 16
#define DISPLAY_NUM_LINES   8
#define DISPLAY_MAX_CONSECUTIVE_ERRORS 3

typedef struct {
    SSD1306_t dev;
    char lock;
    char lines[DISPLAY_NUM_LINES][DISPLAY_BUFFER_SIZE];
    int consecutive_errors;
} Display;

void display_init(Display* display);
bool display_update(Display* display);

#endif  // DISPLAY_H__