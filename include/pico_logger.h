#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/flash.h"
#include "pico/assert.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "boards/pico.h"

// 256k from start of flash
#define LOG_BASE_ADDR (0x00040000)

class Logger {
    public:
        /* Logger
         *
         * @param packet_len: Length of packet in bytes; used as read/write
         *                    bounds for provided packet buffers
         *
         *  @param log_base_addr: where pico_logger should start its entries;
         *                        will only be defined if >= LOG_BASE_ADDR
         *                        (256k from start of flash)
         * 
         * @param print_func: Function pointer that takes const buffer ptr as
         *                    parameter and should print out data of packet_len
         *                    in user's desired format; called on every
         *                    non-empty flash address at packet_len intervals
         */
        Logger(size_t packet_len, uint32_t log_base_addr,
                void (*print_func)(const uint8_t*) );

        /* initialize
         *
         * Called to initialize pico_logger by finding the first available
         * (empty) page in flash to begin its log entries. If pico_logger is
         * operating in multicore environment that is executing from flash
         * (i.e. not a COPY_TO_RAM executable), it will call
         * flash_safe_execute_core_init() so that the core not operating on
         * flash is able to be interrupted so the other core can perform safe
         * flash transactions.
         *
         * @param multicore: True if pico_logger is operating in a multicore
         *                   environment
         */
        void initialize(bool multicore);

        /* read_memory
         *
         * Reads memory from first memory address able to be utilized by
         * pico_logger (LOG_BASE_ADDR) until the first empty page appears.
         * Calls print_func (user provided packet print function) if defined on
         * every non-empty flash address at packet_len intervals. Otherwise,
         * read_memory prints all data found in byte format.
         */
        void read_memory();

        /* write_memory
         *
         * Copies data found in packet to internal buffer until bytes copied is
         * equal to the user defined packet length. When the buffer is found to
         * be full (i.e. buffer_len == FLASH_PAGE_SIZE), write_memory calls
         * flush_buffer to commit the data to flash.
         *
         * @param packet: ptr to buffer containing packet data to write to
         *                memory
         * @param flush: True to force buffer flush regardless of current
         *               capacity
         */
        void write_memory(const uint8_t* packet, bool flush);

        /* erase_memory
         *
         * DANGER: DO NOT CALL THIS FUNCTION UNLESS YOU TRULY WANT YOUR FLASH
         *         WIPED. THIS WILL ERASE ALL LOG ENTRIES.
         * Iterate through all sectors in flash starting from LOG_BASE_ADDR and
         * erase them.
         */
        void erase_memory();
        
        /* flush_buffer
         *
         * Commit contents of buffer to flash, increment log_curr_addr by
         * buffer_len, and reset buffer_len to 0.
         */
        void flush_buffer();

        /* initialize_circular_buffer
         *
         * Allocate memory for circular logging buffer given a size_t len.
         *
         * @param len: number of bytes for circular logging buffer to be
         *             allocated
         */
        void initialize_circular_buffer(size_t len);


        /* write_circular_buffer
         *
         * Copies data found in packet to the circular logging buffer until
         * bytes copied is equal to the user defined packet length. When the
         * buffer is found to be full
         * (i.e. circular_buffer_offset == circular_buffer_len),
         * write_circular_buffer will overwrite the oldest data in the buffer.
         * This circumstance is often used as a way to anticipate an event and
         * log the time just before an event occurs (e.g. a rocket launch).
         *
         * @param packet: ptr to buffer containing packet data to write to
         *                memory
         * @param flush: True to force buffer flush regardless of current
         *               capacity
         */
        void write_circular_buffer(const uint8_t* packet);

        /* read_circular_buffer
         *
         * Calls print_func (user provided packet print function) if defined on
         * every non-empty flash address at packet_len intervals. Otherwise,
         * read_circular_buffer prints all data found in byte format.
         */
        void read_circular_buffer();

        /* flush_circular_buffer
         *
         * Commit contents of circular buffer to flash by calling write_memory
         * for every stored packet and eventually flush_buffer to ensure
         * everything is fully committed to flash.
         *
         * @param free_buffer: whether or not to free the memory allocated to
         *                     the circular buffer; usually true as committing
         *                     a circular_buffer's data to flash typically
         *                     implies it is no longer needed.
         */
        void flush_circular_buffer(bool free_buffer);

    private:
        uint32_t log_base_addr;
        uint32_t log_curr_addr;
        size_t packet_len;
        bool space_available;
        uint8_t buffer[FLASH_PAGE_SIZE];
        uint32_t buffer_len;
        void (*print_func)(const uint8_t*);
        uint8_t* circular_buffer;
        size_t circular_buffer_len;
        size_t circular_buffer_offset;
};
