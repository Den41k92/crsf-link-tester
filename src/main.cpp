
#include <Arduino.h>
#include <M5Core2.h> 
#include <WiFi.h>
// #include "mqtt_client.h"

#include "crsf_rx.h"
#include "ui.h"

static crsf_payload_link_statistics_t * link_stat_ptr = nullptr;
static crsf_channels_t * channel_data_ptr = nullptr;
static xTaskHandle task_lcd, task_crsf_polling;

static uint32_t last_packet_time_us = 0;
static uint32_t packet_timeout_us = 0; // (1e6 * 1/packet_rate) * 1.2
static int current_rf_mode = 0;
static StaticQueue_t data_packet_results_queue;
static QueueHandle_t data_packet_results_queue_handle;
static uint8_t data_packet_results_queue_storage [20 * 1];

void reportPacketResult(uint8_t result) {
	xQueueSendToBack(data_packet_results_queue_handle, &result, 1);
}

void setup() {
	M5.begin(true, false, false);
	UI_setup();
	Serial.begin(115200);
	#if defined(RX_VARIANT_TBS)
		CRSF_RX_begin(CRSF_RX_VARIANT_TBS);
	#else
		CRSF_RX_begin(CRSF_RX_VARIANT_ELRS);
	#endif 
	CRSF_RX_onReceiveLinkStatistics([](crsf_payload_link_statistics_t * stats) { 
		link_stat_ptr = stats;
		Serial.printf("RSSI: %d, LQ: %d, RFMODE: %d, TXPWR: %d\n", 
			stats->uplink_RSSI_1, stats->uplink_Link_quality, stats->rf_Mode, CRSF_txPowerToMilliwatts(stats->uplink_TX_Power));
		if (current_rf_mode != stats->rf_Mode) {
			current_rf_mode = stats->rf_Mode;
			packet_timeout_us = (uint32_t) 1000000.0 / CRSF_rfmdToLinkRateHz(current_rf_mode) * 1.2; // add 20% timeout margin
		}
	});
	CRSF_RX_onReceiveChannelData([](crsf_channels_t * c) {
		last_packet_time_us = micros();
		reportPacketResult(CRSF_RESULT_PACKET_OK);
		channel_data_ptr = c;
	});
	CRSF_RX_onReceiveInvalidFrame([]() {
		last_packet_time_us = micros();
		if (packet_timeout_us > 0) {
			reportPacketResult(CRSF_RESULT_PACKET_INVALID);
		}
	});

	data_packet_results_queue_handle = xQueueCreateStatic(20, 1, data_packet_results_queue_storage, &data_packet_results_queue);

	// Screen rendering on CORE 0
	disableCore0WDT();
	xTaskCreatePinnedToCore([](void * parameters) {
		uint8_t packet_results [20];
		for(;;) {
			int queued_packet_results = uxQueueMessagesWaiting(data_packet_results_queue_handle);
			if (link_stat_ptr != nullptr) { 
				crsf_payload_link_statistics_t * sig = link_stat_ptr;
				UI_setRssi(sig->uplink_RSSI_1);
				UI_setLq(sig->uplink_Link_quality);
				UI_setTxPwr(CRSF_txPowerToMilliwatts(sig->uplink_TX_Power));
				UI_setLinkRate(CRSF_rfmdToLinkRateHz(sig->rf_Mode));
				UI_setRssiScale(CRSF_rfmdToRxSensitivity(sig->rf_Mode), 50);
				link_stat_ptr = nullptr;
			}
			if (channel_data_ptr != nullptr) {
				crsf_channels_t * c = channel_data_ptr;
				uint32_t channel_data [] = {c->ch0, c->ch1, c->ch2, c->ch3, c->ch4, c->ch5, c->ch6, c->ch7, c->ch8, c->ch9};
				UI_setChannels10(channel_data);
				channel_data_ptr = nullptr;
			}
			if (queued_packet_results > 0) {
				for (int i=0; i < queued_packet_results; i++) {
					xQueueReceive(data_packet_results_queue_handle, &packet_results[i], 1);
				}
				UI_pushDataFrameIndication(packet_results, queued_packet_results);
			}
			yield();
		}
	}, "lcd", 8*1024, NULL, 1, &task_lcd, 0);

	// CRSF serial polling task on CORE 1
	xTaskCreatePinnedToCore([](void * parameters) {
		for (;;) {
			CRSF_RX_loop();
			if (packet_timeout_us > 0) {
				uint32_t t = micros();
				if (t - last_packet_time_us > packet_timeout_us) {
					// data packet timeout
					last_packet_time_us = micros();
					reportPacketResult(CRSF_RESULT_PACKET_TIMEOUT);
				}
			}
			yield();
		}
	}, "crsf_polling", 8*1024, NULL, 1, &task_crsf_polling, 1);
}


void loop() {
	
}

