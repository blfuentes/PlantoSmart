#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <ssd1306.h>

#define DISPLAY_BUFFER_SIZE 16
#define DISPLAY_NUM_LINES   8

typedef struct {
    SSD1306_t dev;
    char lock;
    char lines[DISPLAY_NUM_LINES][DISPLAY_BUFFER_SIZE];
} Display;

void display_init(Display* display);
void display_update(Display* display);

#endif  // DISPLAY_H__