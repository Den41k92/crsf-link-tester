#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "crsf_definitions.h"

void CRSF_RX_begin(crsf_rx_variant_e rx_variant = CRSF_RX_VARIANT_ELRS);
void CRSF_RX_onReceiveLinkStatistics(void (*callback)(crsf_payload_link_statistics_t * link_info));
void CRSF_RX_onReceiveChannelData(void (*callback)(crsf_channels_t * channels));
void CRSF_RX_onReceiveInvalidFrame(void (*callback)());
void CRSF_RX_loop();

int CRSF_txPowerToMilliwatts(uint8_t n);
int CRSF_rfmdToLinkRateHz(uint8_t n);
int CRSF_rfmdToRxSensitivity(uint8_t n);