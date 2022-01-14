#include "crsf_rx.h"
#include <M5Core2.h> 

static const uint8_t LINKSTAT_FRAME_START_SEQ [] = {CRSF_ADDRESS_FLIGHT_CONTROLLER, 10+2, CRSF_FRAMETYPE_LINK_STATISTICS}; // 0xC8, 0x0C, 0x14
static const uint8_t CHANNEL_DATA_FRAME_START_SEQ [] = {CRSF_ADDRESS_FLIGHT_CONTROLLER, 22+2, CRSF_FRAMETYPE_RC_CHANNELS_PACKED};

static const int tx_power_to_milliwats [] = {0, 10, 25, 100, -1, -1 -1, 250};
static const int rfmd_to_link_rate_elrs [] = {4, 25, 50, 100, 150, 200, 250, 500};
static const int rfmd_to_link_rate_tbs [] = {4, 50, 150};
static const int rfmd_dbm_sensitivity [] = {125, 123, 120, 117, 112, 112, 108, 105}; // expresslrs
static const int * rfmd_to_link_rate;
static int rfmd_to_link_rate_len = 0;

static uint8_t crsf_frame_buffer [CRSF_FRAME_SIZE_MAX+2];
static crsf_payload_link_statistics_t crsf_link_stats;
static crsf_channels_t crsf_channels_data;

static void (*link_statistics_callback)(crsf_payload_link_statistics_t * link_info) = nullptr;
static void (*channel_data_callback)(crsf_channels_t * channels) = nullptr;


void CRSF_RX_begin(crsf_rx_variant_e rx_variant) {
	if (rx_variant == CRSF_RX_VARIANT_TBS) {
		rfmd_to_link_rate = rfmd_to_link_rate_tbs;
		rfmd_to_link_rate_len = sizeof(rfmd_to_link_rate_tbs);
	}
	else {
		rfmd_to_link_rate = rfmd_to_link_rate_elrs;
		rfmd_to_link_rate_len = sizeof(rfmd_to_link_rate_elrs);
	}
	Serial1.begin(CRSF_BAUDRATE, 134217756U, G13, G14);
	Serial1.setTimeout(1);
}

void CRSF_RX_onReceiveLinkStatistics(void (*callback)(crsf_payload_link_statistics_t * link_info)) {
	link_statistics_callback = callback;
}

void CRSF_RX_onReceiveChannelData(void (*callback)(crsf_channels_t * channels)) {
	channel_data_callback = callback;
}

IRAM_ATTR void CRSF_RX_loop() {
	int nbytes = Serial1.readBytes(crsf_frame_buffer, CRSF_FRAME_SIZE_MAX+2);
	if (nbytes == 0)
		return;

	// Serial.printf("rx frame len = %d\n", nbytes);
	if (nbytes == 14) {
		if (memcmp(crsf_frame_buffer, LINKSTAT_FRAME_START_SEQ, 3) == 0) {
			memcpy(&crsf_link_stats, &crsf_frame_buffer[3], sizeof(crsf_payload_link_statistics_t));
			if (link_statistics_callback != nullptr)
				link_statistics_callback(&crsf_link_stats);
		}
		// Serial.printf("len: %02X %02X %02X\n", crsf_frame_buffer[0], crsf_frame_buffer[1], crsf_frame_buffer[2]);
	}
	else if (nbytes == 26) {
		// Serial.printf("len: %d : %02X %02X %02X\n", nbytes, crsf_frame_buffer[0], crsf_frame_buffer[1], crsf_frame_buffer[2]);
		if (memcmp(crsf_frame_buffer, CHANNEL_DATA_FRAME_START_SEQ, 3) == 0) {
			//memcpy(crsf_channels_buffer, &crsf_frame_buffer[3], sizeof(crsf_channels_buffer));
			memcpy(&crsf_channels_data, &crsf_frame_buffer[3], sizeof(crsf_channels_t));
			if (channel_data_callback != nullptr) 
				channel_data_callback(&crsf_channels_data);
		}
	}
}

int CRSF_txPowerToMilliwatts(uint8_t n) {
	if (n < sizeof(tx_power_to_milliwats))
		return tx_power_to_milliwats[n];
	return -1 * n;
}

int CRSF_rfmdToLinkRateHz(uint8_t n) {
	if (n < rfmd_to_link_rate_len) 
		return rfmd_to_link_rate[n];
	return -1 * n;
}

int CRSF_rfmdToRxSensitivity(uint8_t n) {
	if (n < sizeof(rfmd_dbm_sensitivity)) 
		return rfmd_dbm_sensitivity[n];
	return 120;
}
