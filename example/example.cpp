/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    getchar();

    while(1) {
        tight_loop_contents();
    }
}
