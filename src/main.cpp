
#include <Arduino.h>
#include <M5Core2.h> 
#include <WiFi.h>
// #include "mqtt_client.h"

#include "crsf_rx.h"
#include "ui.h"


void fetchLinkStatistics(crsf_payload_link_statistics_t * sig) {
	Serial.printf("RSSI: %d, LQ: %d, RFMODE: %d, TXPWR: %d\n", 
		sig->uplink_RSSI_1, sig->uplink_Link_quality, sig->rf_Mode, CRSF_txPowerToMilliwatts(sig->uplink_TX_Power));
	
	UI_setRssi(sig->uplink_RSSI_1);
	UI_setLq(sig->uplink_Link_quality);
	UI_setTxPwr(CRSF_txPowerToMilliwatts(sig->uplink_TX_Power));
	UI_setLinkRate(CRSF_rfmdToLinkRateHz(sig->rf_Mode));
	UI_setRssiScale(CRSF_rfmdToRxSensitivity(sig->rf_Mode), 50);
}

void fetchChannelsData(crsf_channels_t * c) {
	uint32_t channel_data [] = {c->ch0, c->ch1, c->ch2, c->ch3, c->ch4, c->ch5, c->ch6, c->ch7};
	UI_setChannels8(channel_data);
}

void setup() {
	M5.begin(true, false, false);
	UI_setup();
	Serial.begin(115200);
	CRSF_RX_begin(CRSF_RX_VARIANT_ELRS);
	CRSF_RX_onReceiveLinkStatistics(fetchLinkStatistics);
	CRSF_RX_onReceiveChannelData(fetchChannelsData);
	
}


void loop() {
	CRSF_RX_loop();
}

