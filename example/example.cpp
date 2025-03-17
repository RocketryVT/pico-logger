#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "pico_logger.h"

#define PACKET_LEN 32

void print_packet(const uint8_t* packet) {
    static bool first_call = true;

    if (first_call) {
        printf("time (us)\t|\tstate\t|\tdep pcnt\t|\talt (m)\t|\tvel (m/s)\t|\tempty\n");
        first_call = false;
    }

    uint64_t now_us = (((uint64_t)packet[0] << 56) | ((uint64_t)packet[1] << 48) | \
                      ((uint64_t)packet[2] << 40)  | ((uint64_t)packet[3] << 32) | \
                      ((uint64_t)packet[4] << 24)  | ((uint64_t)packet[5] << 16) | \
                      ((uint64_t)packet[6] << 8)   | ((uint64_t)packet[7]));

    uint8_t state = packet[8];
    uint8_t deploy_percent = packet[9];

    uint32_t alt_bits = (packet[10] << 24) | (packet[11] << 16) | (packet[12] << 8) | (packet[13]);
    uint32_t vel_bits = (packet[14] << 24) | (packet[15] << 16) | (packet[16] << 8) | (packet[17]);
    float altitude = *(float *)(&alt_bits);
    float velocity = *(float *)(&vel_bits);
    printf("%" PRIu64 "\t|\t%" PRIu8 "\t|\t%" PRIu8 "\t|\t%4.2f\t|\t%4.2f\t|\t%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", \
            now_us, state, deploy_percent, altitude, velocity, \
            packet[18],packet[19],packet[20],packet[21],packet[22],packet[23],packet[24],packet[25],packet[26],packet[27],packet[28],packet[29],packet[30],packet[31]);
}

Logger logger(PACKET_LEN, LOG_BASE_ADDR, &print_packet);

void core1_entry() {
    printf("Reading memory!\n");
    logger.read_memory();
    printf("Erasing memory!\n");
    logger.erase_memory();
    printf("Reading memory!\n");
    logger.read_memory();
    logger.initialize(false);

    printf("Writing memory!\n");

    uint8_t entry[PACKET_LEN];

    printf("Written Data:\n");
    printf("time (us)\t|\tstate\t|\tdep pcnt\t|\talt (m)\t|\tvel (m/s)\t|\tempty\n");
    for (uint16_t i = 0; i < 500; i++) {
        absolute_time_t now = get_absolute_time();
        uint64_t now_us= to_us_since_boot(now);
        float altitude = 10.0f * i;
        float velocity = 5.0f * i;
        uint8_t deploy_percent = (i*100) / 200;
        printf("%" PRIu64 "\t|\t%" PRIu8 "\t|\t%" PRIu8 "\t|\t%4.2f\t|\t%4.2f\t|\tDAWSYN_SCHRAIB\n", now_us, (uint8_t)(i), deploy_percent, altitude, velocity);
        uint32_t alt_bits = *((uint32_t *)&altitude);
        uint32_t vel_bits = *((uint32_t *)&velocity);
        entry[0] = now_us >> 56;
        entry[1] = now_us >> 48;
        entry[2] = now_us >> 40;
        entry[3] = now_us >> 32;
        entry[4] = now_us >> 24;
        entry[5] = now_us >> 16;
        entry[6] = now_us >> 8;
        entry[7] = now_us;
        entry[8] = i;
        entry[9] = deploy_percent;
        entry[10] = alt_bits >> 24;
        entry[11] = alt_bits >> 16;
        entry[12] = alt_bits >> 8;
        entry[13] = alt_bits;
        entry[14] = vel_bits >> 24;
        entry[15] = vel_bits >> 16;
        entry[16] = vel_bits >> 8;
        entry[17] = vel_bits;
        entry[18] = 'D';
        entry[19] = 'A';
        entry[20] = 'W';
        entry[21] = 'S';
        entry[22] = 'Y';
        entry[23] = 'N';
        entry[24] = '_';
        entry[25] = 'S';
        entry[26] = 'C';
        entry[27] = 'H';
        entry[28] = 'R';
        entry[29] = 'A';
        entry[30] = 'I';
        entry[31] = 'B';
        logger.write_memory(entry, false);
    }
    logger.flush_buffer();
 
    printf("Reading memory!\n");
    logger.read_memory();
}

int main() {
    stdio_init_all();

    getchar();

    logger.initialize(true);

    multicore_launch_core1(core1_entry);

    uint32_t counter = 0;
    while(1) {
        if (counter == get_rand_32()) {
            printf("Hooray!\n");
        } else if (counter == UINT32_MAX) {
            counter = 0;
        }
        counter += get_rand_32();
    }
}
