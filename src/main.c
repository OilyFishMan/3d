#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <ncurses.h>
#include <math.h>
#include <time.h>

#define key_ratio (1.0f/3.0f)

#define width  (300)
#define height (100)

#define array_len(array_) (sizeof(array_) / sizeof((array_)[0]))
#define string_array_len(string_) (array_len(string_) - 1)

#define current_time \
    ({ struct timespec ts_now_; \
       clock_gettime(CLOCK_REALTIME, &ts_now_); \
       (double) ts_now_.tv_sec + (double) ts_now_.tv_nsec * 1e-9; })

#define fixed_delta (1.0/60.0)

int main(void)
{
    bool success = true;

    const char progression[] = " .+13#";
    const size_t progression_len = string_array_len(progression);

    float* screen = malloc(sizeof(*screen) * (size_t) (width * height));

    if (screen == NULL) {
        perror("OS Error");
        success = false;
        goto end;
    }

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    nodelay(stdscr, true);
    set_escdelay(50);
    curs_set(0);

    const double time_start = current_time;
    size_t tick = 0;

    while (true) {
        const double frame_begin = current_time;

        for (int y = 0; y < (int) height; ++y) {
            for (int x = 0; x < (int) width; ++x) {
                const float normal_x = (float) x / width;
                const float normal_y = (float) y / height;

                float diff = 1.0f;

                size_t count = (size_t) ((frame_begin - time_start) / (1.0f / 50.0f));
                if (count > 20) {
                    count = 20;
                }

                for (size_t i = 0; i < count; ++i) {
                    const float pos_x = 0.5f + cosf(tick * 2.0f * M_PI * 1e-3f + i * 2.0f * M_PI / count) * 2e-2f * (i % 20) * (frame_begin - time_start) / 2.0f;
                    const float pos_y = 0.5f + sinf(tick * 2.0f * M_PI * 1e-3f + i * 2.0f * M_PI / count) * 2e-2f * (i % 20) * (frame_begin - time_start) / 2.0f;

                    const float radius = ((sinf(tick * 2.0f * M_PI * 2e-2) + 1.0f) * 1e-3f + 3e-2f);
                    const float individual_diff = ((normal_y - pos_y) * (normal_y - pos_y) + (normal_x - pos_x) * (normal_x - pos_x));

                    if (individual_diff < radius) {
                        diff *= individual_diff / radius;
                    }
                }

                screen[y * width + x] = 1.0f - fmin(1.0f, diff);

                if (x == 0 || x == (int) (width - 1) || y == 0 || y == (int) (height - 1)) {
                    screen[y * width + x] = 0.75f;
                }
            }
        }

        int ch = getch();
        switch (ch) {
            case 27: {
                goto loop_break;
            }
        }

        erase();
        for (size_t y = 0; y < height; ++y) {
            move(y, 0);
            for (size_t x = 0; x < width; ++x) {
                size_t text_index = round(fmin(screen[y * width + x], 1.0f) * (progression_len - 1));

                if (text_index > progression_len - 1) {
                    text_index = progression_len - 1;
                }

                printw("%c", progression[text_index]);
            }
        }
        refresh();

        const double frame_end = current_time;
        const double delta = frame_end - frame_begin;
        if (delta < fixed_delta) {
            usleep((fixed_delta - delta) * 1e+6);
        }
        ++tick;
    }

loop_break:
    endwin();

    free(screen);

end:
    if (success) {
        fprintf(stderr, "Something bad didn't happen :)\n");
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Something bad happened :(\n");
        return EXIT_FAILURE;
    }
}
