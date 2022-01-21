#pragma once
#include <stdint.h>


void UI_setup();
void UI_setLq(int value);
void UI_setRssi(int value);
void UI_setTxPwr(int value);
void UI_setLinkRate(int value);
void UI_setRssiScale(int dbm_min, int dbm_max);
void UI_setChannels10(uint32_t * channel_data_8);
void UI_pushDataFrameIndication(uint8_t * results, int count);

typedef enum {
	CRSF_RESULT_PACKET_OK = 0,
	CRSF_RESULT_PACKET_TIMEOUT = 1,
	CRSF_RESULT_PACKET_INVALID = 2
} crsf_packet_result_e;