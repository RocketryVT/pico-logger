#pragma once

#include <stddef.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"

class Logger {
    public:
        Logger(size_t len);

        void write_memory();
    private:
        size_t len;
};
