#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <ssd1306.h>
#include <stdbool.h>

#define DISPLAY_BUFFER_SIZE            17
#define DISPLAY_NUM_LINES              8
#define DISPLAY_MAX_CONSECUTIVE_ERRORS 3

static const int DISPLAY_TITLE_LINE    = 0;
static const int DISPLAY_DEBUG_PAGE    = 1;
static const int DISPLAY_LIGHT_LINE    = 2;
static const int DISPLAY_HUMIDITY_LINE = 3;

typedef struct {
    SSD1306_t dev;
    char lock;
    char lines[DISPLAY_NUM_LINES][DISPLAY_BUFFER_SIZE];
    int consecutive_errors;
    bool debug_mode;
} Display;

void display_init(Display* display);
bool display_update(Display* display);

#endif  // DISPLAY_H__