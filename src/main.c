#include <unistd.h>
#include <stdbool.h>
#include <locale.h>

#include "bar.h"
#include "blocks.h"

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");

    bar_t bar;

    bar_init(&bar);

    bar_add(&bar, block_battery);
    bar_add(&bar, block_memory);
    bar_add(&bar, block_cpu);
    bar_add(&bar, block_time);

    while (true) {
        bar_update(&bar);
        bar_wait(&bar);
    }

    return 0;
}
