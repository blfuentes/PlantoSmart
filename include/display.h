#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <ssd1306.h>

#define DISPLAY_BUFFER_SIZE 16

typedef struct {
    SSD1306_t dev;
    char lock;
    char buffer[DISPLAY_BUFFER_SIZE];
} Display;

void display_init(Display* display);

#endif  // DISPLAY_H__