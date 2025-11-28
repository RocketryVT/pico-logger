#include "pico_logger.hpp"

static void call_flash_range_program(void *param) {
    uint32_t offset = ((uintptr_t*)param)[0];
    uint32_t buffer_len = ((uintptr_t*)param)[1];
    const uint8_t *data = (const uint8_t *)((uintptr_t*)param)[2];
    flash_range_program(offset, data, FLASH_PAGE_SIZE);
}

static void call_flash_range_erase(void *param) {
    uint32_t log_base_addr = *((uint32_t *) param);
    flash_range_erase(log_base_addr, (PICO_FLASH_SIZE_BYTES - log_base_addr));
}

Logger::Logger(size_t packet_len, uint32_t log_base_addr, void (*print_func)(const uint8_t*)) {
    this->packet_len = packet_len;
    this->space_available = true;

    this->log_base_addr = (log_base_addr >= LOG_BASE_ADDR) ? log_base_addr :
                                                             LOG_BASE_ADDR;
    this->log_curr_addr = log_base_addr;
    this->buffer_len = 0;
    this->print_func = print_func;
}

void Logger::initialize(bool multicore) {
    #if !PICO_COPY_TO_RAM
    if (multicore) {
        flash_safe_execute_core_init();
    }
    #endif

    // XIP_BASE accesses the flash for reads using the builtin "DMA" of sorts
    const uint8_t* flash_contents = (const uint8_t *) (XIP_BASE + log_base_addr);

    // Search for the first empty page in flash
    uint32_t offset = 0;
    uint32_t counter = 0;
    while (counter < FLASH_PAGE_SIZE && offset < PICO_FLASH_SIZE_BYTES) {
        if (*(flash_contents + offset) == 0xFF) {
            counter++;
        } else {
            counter = 0;
        }
        offset++;
    }

    // All pages were found to contain non-zero (or 0xFF because NOR) data
    if (offset >= (PICO_FLASH_SIZE_BYTES - FLASH_PAGE_SIZE)) {
        printf("PICO_LOGGER: No space available!\n");
        space_available = false;
    } else {
        log_curr_addr = log_base_addr + (offset - FLASH_PAGE_SIZE);
        space_available = true;
        printf("PICO_LOGGER: Open space found at addr 0x%08" PRIx32 "\n", log_curr_addr);
    }

    // Set buffer to all 0xFF to prevent writing unnecessary zeros when flushing
    // an incomplete buffer
    memset(buffer, 0xFF, FLASH_PAGE_SIZE);
}

void Logger::read_memory() {
    const uint8_t* flash_contents = (const uint8_t *) (XIP_BASE + log_base_addr);
    uint32_t offset = 0;

    if (print_func != nullptr) {
        while (offset < (log_curr_addr - log_base_addr)) {
            (*print_func)((flash_contents + offset));
            offset += packet_len;
        }
    } else {
        printf("PICO_LOGGER: Reading memory back in byte format!\n");
        for (; offset < (log_curr_addr - log_base_addr); offset++) {
            printf("%02x", *(flash_contents + offset));
            if ((offset % packet_len) == (packet_len - 1))
                printf("\n");
            else
                printf(" ");
        }
    }

    if (offset == 0) {
        printf("PICO_LOGGER: No log entries found!\n");
    } else {
        printf("PICO_LOGGER: Finished reading memory!\n");
    }
}

void Logger::write_memory(const uint8_t* packet, bool flush) {
    if (packet != NULL) {
        if (FLASH_PAGE_SIZE - buffer_len < packet_len) {
            flush_buffer();
        }
        memcpy((buffer + buffer_len), packet, packet_len);
        buffer_len += packet_len;

        if (flush) { flush_buffer(); }
    }
}

void Logger::flush_buffer() {
    uint32_t ints = save_and_disable_interrupts();
#if PICO_COPY_TO_RAM
    uintptr_t params[] = { log_curr_addr, buffer_len, (uintptr_t)buffer};
    call_flash_range_program(params);
#else
    uintptr_t params[] = { log_curr_addr, buffer_len, (uintptr_t)buffer};
    int rc = flash_safe_execute(call_flash_range_program, params, UINT32_MAX);
    hard_assert(rc == PICO_OK);
#endif
    log_curr_addr = log_curr_addr + buffer_len;
    memset(buffer, 0xFF, FLASH_PAGE_SIZE);
    buffer_len = 0;
    restore_interrupts(ints);
}

void Logger::erase_memory() {
    printf("PICO_LOGGER: Erasing all memory from address 0x%08" PRIx32 " on!\n", log_base_addr);
    uint32_t ints = save_and_disable_interrupts();
    #if PICO_COPY_TO_RAM
        call_flash_range_erase((void*)&log_base_addr);
    #else
        int rc = flash_safe_execute(call_flash_range_erase, (void*)&log_base_addr, UINT32_MAX);
        hard_assert(rc == PICO_OK);
    #endif
    log_curr_addr = log_base_addr;
    restore_interrupts(ints);
    printf("PICO_LOGGER: Memory erasure complete!\n");
}


void Logger::initialize_circular_buffer(size_t len) {
    if (circular_buffer == NULL) {
        circular_buffer_capacity = len;
        circular_buffer = (uint8_t*) calloc(circular_buffer_capacity, 1);
    }
}

void Logger::write_circular_buffer(const uint8_t* packet) {
    if (packet != NULL) {
        for (uint32_t idx = 0; idx < packet_len; idx++) {
            circular_buffer[idx + circular_buffer_offset] = packet[idx];
        }
        circular_buffer_offset += packet_len;
        circular_buffer_offset %= circular_buffer_capacity;
        if (circular_buffer_len != circular_buffer_capacity) {
            circular_buffer_len += packet_len;
        }
    }
}

void Logger::read_circular_buffer() {
    uint32_t idx = 0;
    if (circular_buffer_len == circular_buffer_capacity) {
        uint32_t idx = ((circular_buffer_offset + packet_len) % circular_buffer_capacity);
    }
    if (print_func != nullptr) {
        do {
            print_func(circular_buffer + idx);
            idx += packet_len;
            idx %= circular_buffer_capacity;
        } while (idx != circular_buffer_offset);
    } else {
        printf("PICO_LOGGER: Reading memory back in byte format!\n");
        do {
            printf("%02x", *(circular_buffer + idx));
            if ((idx % packet_len) == (packet_len - 1)) {
                printf("\n");
            } else {
                printf(" ");
            }
            idx += packet_len;
            idx %= circular_buffer_capacity;
        } while ( idx != circular_buffer_offset );
    }
}

void Logger::flush_circular_buffer(bool free_buffer) {
    uint32_t idx = 0;
    if (circular_buffer_len == circular_buffer_capacity) {
        idx = ((circular_buffer_offset + packet_len) % circular_buffer_capacity);
    }
    do {
        write_memory(circular_buffer + idx, true);
        idx += packet_len;
        idx %= circular_buffer_capacity;
    } while (idx != circular_buffer_offset);
    flush_buffer();
    if (free_buffer) {
        free(circular_buffer);
        circular_buffer = NULL;
    }
}
