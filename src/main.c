#include <unistd.h>
#include <stdbool.h>
#include <locale.h>
#include <signal.h>

#include "bar.h"
#include "blocks.h"

void trap(int unused) {}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");

    const struct sigaction act = {
        .sa_handler = trap
    };

    sigaction(SIGUSR1, &act, NULL);

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
