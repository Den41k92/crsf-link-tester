#include "crsf_rx.h"

#ifdef BOARD_M5CORE2
	#include <M5Core2.h> 
	#define PIN_RX G13
	#define PIN_TX G14
#else
	#include <Arduino.h>
#endif // BOARD_M5CORE2

static const uint8_t LINKSTAT_FRAME_START_SEQ [] = {CRSF_ADDRESS_FLIGHT_CONTROLLER, 10+2, CRSF_FRAMETYPE_LINK_STATISTICS}; // 0xC8, 0x0C, 0x14
static const uint8_t CHANNEL_DATA_FRAME_START_SEQ [] = {CRSF_ADDRESS_FLIGHT_CONTROLLER, 22+2, CRSF_FRAMETYPE_RC_CHANNELS_PACKED};

static const int tx_power_to_milliwats [] = {0, 10, 25, 100, -1, -1 -1, 250};
static const int rfmd_to_link_rate_elrs [] = {4, 25, 50, 100, 150, 200, 250, 500};
static const int rfmd_to_link_rate_tbs [] = {4, 50, 150};
static const int rfmd_dbm_sensitivity [] = {125, 123, 120, 117, 112, 112, 108, 105}; // expresslrs
static const int * rfmd_to_link_rate;
static int rfmd_to_link_rate_len = 0;
static uint8_t crc8_lut [256];

static uint8_t crsf_frame_buffer [CRSF_FRAME_SIZE_MAX*2];
static int crsf_frame_buffer_pos = 0;


static void CRC8_createLut(uint8_t poly)
{
	for (int i=0; i < 256; i++) {
		uint8_t crc = i;
		for (int j=0; j < 8; j++) {
			crc = (crc << 1) ^ ((crc & 0x80) ? poly : 0);
		}
		crc8_lut[i] = crc;
	}
}

static uint8_t CRC8_calculate(uint8_t * data, int len) {
	uint8_t crc = 0;
	for (int i=0; i < len; i++) {
		crc = crc8_lut[crc ^ *data++];
	}
	return crc;
}

static crsf_payload_link_statistics_t crsf_link_stats;
static crsf_channels_t crsf_channels_data;


static void (*link_statistics_callback)(crsf_payload_link_statistics_t * link_info) = nullptr;
static void (*channel_data_callback)(crsf_channels_t * channels) = nullptr;
static void (*invalid_frame_callback)() = nullptr;


void CRSF_RX_begin(crsf_rx_variant_e rx_variant) {
	if (rx_variant == CRSF_RX_VARIANT_TBS) {
		rfmd_to_link_rate = rfmd_to_link_rate_tbs;
		rfmd_to_link_rate_len = sizeof(rfmd_to_link_rate_tbs);
	}
	else {
		rfmd_to_link_rate = rfmd_to_link_rate_elrs;
		rfmd_to_link_rate_len = sizeof(rfmd_to_link_rate_elrs);
	}
	Serial1.begin(CRSF_BAUDRATE, 134217756U, PIN_RX, PIN_TX);
	Serial1.setTimeout(0);
	CRC8_createLut(0xD5);
}

void CRSF_RX_onReceiveLinkStatistics(void (*callback)(crsf_payload_link_statistics_t * link_info)) {
	link_statistics_callback = callback;
}

void CRSF_RX_onReceiveChannelData(void (*callback)(crsf_channels_t * channels)) {
	channel_data_callback = callback;
}

void CRSF_RX_onReceiveInvalidFrame(void (*callback)()) {
	invalid_frame_callback = callback;
}

bool processFrame() {
	// check for CRC
	int address = crsf_frame_buffer[0];
	int length = crsf_frame_buffer[1];
	int frame_type = crsf_frame_buffer[2];
	int expected_crc = crsf_frame_buffer[length + 1];
	if (CRC8_calculate(&crsf_frame_buffer[2], length - 1) == expected_crc) { // expected crc
		if (address == CRSF_ADDRESS_FLIGHT_CONTROLLER) {
			if (frame_type == CRSF_FRAMETYPE_LINK_STATISTICS && length == CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE + 2) {
				memcpy(&crsf_link_stats, &crsf_frame_buffer[3], sizeof(crsf_payload_link_statistics_t));
				if (link_statistics_callback != nullptr)
					link_statistics_callback(&crsf_link_stats);
			}
			// Serial.printf("len: %02X %02X %02X\n", crsf_frame_buffer[0], crsf_frame_buffer[1], crsf_frame_buffer[2]);
			else if (frame_type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED && length == CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE + 2) {
				memcpy(&crsf_channels_data, &crsf_frame_buffer[3], sizeof(crsf_channels_t));
				if (channel_data_callback != nullptr) 
					channel_data_callback(&crsf_channels_data);
			}
			else {
				Serial.printf("len: %d : %02X %02X %02X\n", length, crsf_frame_buffer[0], crsf_frame_buffer[1], crsf_frame_buffer[2]);
			}
		}
		return true;
	}
	return false;
}

static void gotFrameError() {
	crsf_frame_buffer_pos = 0; // flush buffer
	if (invalid_frame_callback != nullptr) {
		invalid_frame_callback();
	}
}

IRAM_ATTR void CRSF_RX_loop() {
	while (Serial1.available() && crsf_frame_buffer_pos < CRSF_FRAME_SIZE_MAX + 2) {
		crsf_frame_buffer[crsf_frame_buffer_pos++] = Serial1.read();
	}

	if (crsf_frame_buffer_pos > 3) {
		int expected_frame_len = crsf_frame_buffer[CRSF_FRAME_LENGTH_ADDRESS];
		int full_frame_length = expected_frame_len + 2;
		if (full_frame_length > CRSF_FRAME_SIZE_MAX) {
			Serial.printf("Frame error: size if too large = %d\n", full_frame_length);
			gotFrameError();
		}
		else if (crsf_frame_buffer_pos >= full_frame_length) {
			if (processFrame()) {
				crsf_frame_buffer_pos -= full_frame_length; // subtract frame size from write pointer
				memcpy(crsf_frame_buffer, &crsf_frame_buffer[full_frame_length], crsf_frame_buffer_pos); // put the remaining bytes into beginning
			}
			else {
				Serial.printf("Frame error: wrong CRC\n");
				gotFrameError();
			}
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
