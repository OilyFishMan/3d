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
    ({ struct timespec ts_now_; clock_gettime(CLOCK_REALTIME, &ts_now_); ts_now_; })

#define fixed_delta (1.0/60.0)

int main(void)
{
    //fprintf(stderr, "%f %f, %f %f\n", width, height, width, height);
    //return 0;
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

    size_t tick = 0;

    while (true) {
        const struct timespec ts_begin = current_time;

        for (int y = 0; y < (int) height; ++y) {
            for (int x = 0; x < (int) width; ++x) {
                const float normal_x = (float) x / width;
                const float normal_y = (float) y / height;

                float diff = 1.0f;

                for (size_t i = 0; i <= 10; ++i) {
                    const float pos_x = 0.5f + cosf((tick + i * i) * 2.0f * M_PI * 1.0f / 1000.0f) * (i % 10 * 0.125f);
                    const float pos_y = 0.5f + sinf((tick + i * i) * 2.0f * M_PI * 1.0f / 1000.0f) * (i % 10 * 0.125f);

                    diff *= fmin( ((normal_y - pos_y) * (normal_y - pos_y) + (normal_x - pos_x) * (normal_x - pos_x))
                                / ((sinf(tick * 2.0f * M_PI * 1.0f / 200.0f) + 1.0f) * 0.03f + 0.05f), 2.0f);
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

        const struct timespec ts_end = current_time;
        const double diff = (double) (ts_end.tv_sec - ts_begin.tv_sec)
                          + 1e-9 * (double) (ts_end.tv_nsec - ts_begin.tv_nsec);
        if (diff < fixed_delta) {
            usleep((fixed_delta - diff) * 1e+6);
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
