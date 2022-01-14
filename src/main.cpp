
#include <Arduino.h>
#include <M5Core2.h> 
#include <WiFi.h>
// #include "mqtt_client.h"

#include "crsf_rx.h"
#include "ui.h"

static crsf_payload_link_statistics_t * link_stat_ptr = nullptr;
static crsf_channels_t * channel_data_ptr = nullptr;
static xTaskHandle task_lcd, task_crsf_polling;

void setup() {
	M5.begin(true, false, false);
	UI_setup();
	Serial.begin(115200);
	CRSF_RX_begin(CRSF_RX_VARIANT_ELRS);
	CRSF_RX_onReceiveLinkStatistics([](crsf_payload_link_statistics_t * stats) { 
		link_stat_ptr = stats; 
	});
	CRSF_RX_onReceiveChannelData([](crsf_channels_t * c) {
		channel_data_ptr = c;
	});

	// Screen rendering on CORE 0
	disableCore0WDT();
	xTaskCreatePinnedToCore([](void * parameters) {
		uint32_t t = millis();
		for(;;) {
			if (millis() - t >= 20) {
				t = millis();
				if (link_stat_ptr != nullptr) { 
					crsf_payload_link_statistics_t * sig = link_stat_ptr;
					Serial.printf("RSSI: %d, LQ: %d, RFMODE: %d, TXPWR: %d\n", 
						sig->uplink_RSSI_1, sig->uplink_Link_quality, sig->rf_Mode, CRSF_txPowerToMilliwatts(sig->uplink_TX_Power));
					UI_setRssi(sig->uplink_RSSI_1);
					UI_setLq(sig->uplink_Link_quality);
					UI_setTxPwr(CRSF_txPowerToMilliwatts(sig->uplink_TX_Power));
					UI_setLinkRate(CRSF_rfmdToLinkRateHz(sig->rf_Mode));
					UI_setRssiScale(CRSF_rfmdToRxSensitivity(sig->rf_Mode), 50);
					
				}
				if (channel_data_ptr != nullptr) {
					crsf_channels_t * c = channel_data_ptr;
					uint32_t channel_data [] = {c->ch0, c->ch1, c->ch2, c->ch3, c->ch4, c->ch5, c->ch6, c->ch7};
					UI_setChannels8(channel_data);
				}
				link_stat_ptr = nullptr;
				channel_data_ptr = nullptr;
			}
			yield();
		}
	}, "lcd", 8*1024, NULL, 1, &task_lcd, 0);

	// CRSF serial polling task on CORE 1
	xTaskCreatePinnedToCore([](void * parameters) {
		for (;;) {
			CRSF_RX_loop();
			yield();
		}
	}, "crsf_polling", 8*1024, NULL, 1, &task_crsf_polling, 1);
}


void loop() {
	
}

