
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
}

void setup() {

	M5.begin(true, false, false);
	Serial.begin(115200);
	CRSF_RX_begin();
	CRSF_RX_onReceiveLinkStatistics(fetchLinkStatistics);
	UI_setup();
}


void loop() {
	CRSF_RX_loop();
}

