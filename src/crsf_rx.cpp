#include "crsf_rx.h"
#include <M5Core2.h> 

static void (*link_statistics_callback)(crsf_payload_link_statistics_t * link_info) = nullptr;
static uint8_t crsf_frame_buffer [CRSF_FRAME_SIZE_MAX];
static crsf_payload_link_statistics_t crsf_link_stats;

static const uint8_t LINKSTAT_FRAME_START_SEQ [] = {0xC8, 0x0C, 0x14};

void CRSF_RX_begin() { 
    Serial1.begin(CRSF_BAUDRATE, 134217756U, G13, G14);
    Serial1.setTimeout(2);
}

void CRSF_RX_onReceiveLinkStatistics(void (*callback)(crsf_payload_link_statistics_t * link_info)) {
    link_statistics_callback = callback;
}


void CRSF_RX_loop() {
    int nbytes = Serial1.readBytes(crsf_frame_buffer, 14);
    if (nbytes == 0)
        return;

    // Serial.printf("rx frame len = %d\n", nbytes);
    if (nbytes == 14) {

        if (memcmp(crsf_frame_buffer, LINKSTAT_FRAME_START_SEQ, sizeof(LINKSTAT_FRAME_START_SEQ)) == 0) {
            memcpy(&crsf_link_stats, &crsf_frame_buffer[3], sizeof(crsf_payload_link_statistics_t));
            if (link_statistics_callback != nullptr)
                link_statistics_callback(&crsf_link_stats);
        }
        // Serial.printf("len: %02X %02X %02X\n", crsf_frame_buffer[0], crsf_frame_buffer[1], crsf_frame_buffer[2]);
    }
}

int CRSF_txPowerToMilliwatts(uint8_t n) {
    switch(n) {
        case 1:
            return 10;
        case 2:
            return 25;
        case 8:
            return 50;
        case 3:
            return 100;
        case 7:
            return 250;
        default:
            return -1 * n;
    }
}