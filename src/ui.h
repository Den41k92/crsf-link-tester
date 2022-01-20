#pragma once
#include <stdint.h>


void UI_setup();
void UI_setLq(int value);
void UI_setRssi(int value);
void UI_setTxPwr(int value);
void UI_setLinkRate(int value);
void UI_setRssiScale(int dbm_min, int dbm_max);
void UI_setChannels10(uint32_t * channel_data_8);
void UI_pushDataFrameIndication(bool received_within_timeout);