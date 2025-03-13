#include "pico_logger.h"

Logger::Logger(size_t len) {
    this->len = len;
}

void Logger::write_memory() {
    printf("Definitely wrote some memory! Approx %d bytes of it....\n", this->len);
}
