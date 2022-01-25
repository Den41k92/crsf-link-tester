
#include "ui.h"
#include <M5Core2.h>

static TFT_eSprite rssi_text(&M5.Lcd);
static TFT_eSprite rssi_bar(&M5.Lcd);
static TFT_eSprite lq_text(&M5.Lcd);
static TFT_eSprite lq_bar(&M5.Lcd);
static TFT_eSprite tx_pwr_text(&M5.Lcd);
static TFT_eSprite link_rate_text(&M5.Lcd);
static TFT_eSprite rx_frame_indicator_bar(&M5.Lcd);

static TFT_eSprite ch1_text(&M5.Lcd);
static TFT_eSprite ch2_text(&M5.Lcd);
static TFT_eSprite ch3_text(&M5.Lcd);
static TFT_eSprite ch4_text(&M5.Lcd);
static TFT_eSprite ch5_text(&M5.Lcd);
static TFT_eSprite ch6_text(&M5.Lcd);
static TFT_eSprite ch7_text(&M5.Lcd);
static TFT_eSprite ch8_text(&M5.Lcd);
static TFT_eSprite ch9_text(&M5.Lcd);
static TFT_eSprite ch10_text(&M5.Lcd);

static TFT_eSprite ch1_bar(&M5.Lcd);
static TFT_eSprite ch2_bar(&M5.Lcd);
static TFT_eSprite ch3_bar(&M5.Lcd);
static TFT_eSprite ch4_bar(&M5.Lcd);
static TFT_eSprite ch5_bar(&M5.Lcd);
static TFT_eSprite ch6_bar(&M5.Lcd);
static TFT_eSprite ch7_bar(&M5.Lcd);
static TFT_eSprite ch8_bar(&M5.Lcd);
static TFT_eSprite ch9_bar(&M5.Lcd);
static TFT_eSprite ch10_bar(&M5.Lcd);

static uint32_t packet_counter = 0;

static const TFT_eSprite * channel_texts [] = {
	&ch1_text, &ch2_text, &ch3_text, &ch4_text, &ch5_text, &ch6_text, &ch7_text, &ch8_text, &ch9_text, &ch10_text
};

static const TFT_eSprite * channel_bars [] = {
	&ch1_bar, &ch2_bar, &ch3_bar, &ch4_bar, &ch5_bar, &ch6_bar, &ch7_bar, &ch8_bar, &ch9_bar, &ch10_bar
};

static int rssi_scale_min = 120;
static int rssi_scale_max = 50;

// remember last values to prevent unnecessary sprite updates
static int current_rc_channels_data [10] = {0};
static int current_rssi = -1;
static int current_lq = -1;
static int current_pw = -1;


static void createElement(TFT_eSprite &sprite, uint8_t font_size, uint16_t width, uint16_t height, uint16_t color) {
	sprite.createSprite(width, height);
	sprite.setTextSize(font_size);
	sprite.setTextColor(color);
}

static void clearSprite(TFT_eSprite & s) {
	s.setCursor(0, 0);
	s.fillSprite(0);
}

static void drawProgressBar(TFT_eSprite & s, uint16_t color, int val, int v_min, int v_max) {
	s.fillSprite(0);
	s.drawRect(0, 0, s.width()-1, s.height()-1, color); // draw outline
	s.fillRect(0, 0, map(val, v_min, v_max, 0, s.width()-1), s.height()-1, color);
}

void UI_setup() {
	createElement(rssi_text, 2, 100, 20, TFT_CYAN);
	createElement(rssi_bar, 2, 200, 20, TFT_CYAN);
	createElement(lq_text, 2, 100, 20, TFT_GREENYELLOW);
	createElement(lq_bar, 2, 200, 20, TFT_GREENYELLOW);
	createElement(tx_pwr_text, 2, 100, 20, TFT_DARKGREY);
	createElement(link_rate_text, 2, 100, 20, TFT_DARKGREY);
	createElement(rx_frame_indicator_bar, 4, 200, 50, TFT_ORANGE);
	rx_frame_indicator_bar.setScrollRect(0, 0, rx_frame_indicator_bar.width(), rx_frame_indicator_bar.height(), TFT_BLACK);
	for (uint8_t i=0; i < 10; i++) {
		createElement((TFT_eSprite&) *channel_texts[i], 1, 90, 10, TFT_WHITE);
		createElement((TFT_eSprite&) *channel_bars[i], 1, 210, 9, TFT_LIGHTGREY);
	}
	rx_frame_indicator_bar.print("RX WAIT");
	rx_frame_indicator_bar.setTextColor(TFT_BLACK);
	rx_frame_indicator_bar.pushSprite(110, 70);
	UI_setRssi(0);
	UI_setLq(0);
	UI_setTxPwr(0);
	UI_setLinkRate(0);
}

void UI_setLq(int percent) {
	if (percent != current_lq) {
		current_lq = percent;
		clearSprite(lq_text);
		lq_text.printf("LQ %d", percent);
		lq_text.pushSprite(10, 40);
		drawProgressBar(lq_bar, TFT_GREENYELLOW, percent, 0, 100);
		lq_bar.pushSprite(110, 40);
	}
}

void UI_setRssi(int dbm) {
	if (dbm != current_rssi) {
		current_rssi = dbm;
		clearSprite(rssi_text);
		if (dbm > 0) {
			rssi_text.printf("RS -%d", dbm);
			drawProgressBar(rssi_bar, TFT_CYAN, dbm, rssi_scale_min, rssi_scale_max);
		}
		else {
			rssi_text.printf("RX WAIT");
			drawProgressBar(rssi_bar, TFT_RED, 0, 0, 1);
		}
		rssi_text.pushSprite(10, 10);
		rssi_bar.pushSprite(110, 10);
	}
}

void UI_setTxPwr(int value) {
	if (value != current_pw) {
		current_pw = value;
		clearSprite(tx_pwr_text);
		tx_pwr_text.printf("PW %d", value);
		tx_pwr_text.pushSprite(10, 70);
	}
}

void UI_setLinkRate(int hz) {
	clearSprite(link_rate_text);
	link_rate_text.printf("PR %d", hz);
	link_rate_text.pushSprite(10, 100);
}

void UI_setRssiScale(int dbm_min, int dbm_max) {
	rssi_scale_min = dbm_min;
	rssi_scale_max = dbm_max;
}

void UI_setChannels10(uint32_t * channel_data_10) {
	for (uint8_t i=0; i < 10; i++) {
		int value =  channel_data_10[i];
		// update graphics only if the current channel value is different from previous
		if (value != current_rc_channels_data[i]) {
			current_rc_channels_data[i] = value;
			TFT_eSprite& text = (TFT_eSprite&) *channel_texts[i];
			TFT_eSprite& bar = (TFT_eSprite&) *channel_bars[i];
			int v_offset = 140 + 10 * i;

			clearSprite(text);
			text.printf("CH%2d: %4d", i+1, value);
			text.pushSprite(10, v_offset);

			clearSprite(bar);
			drawProgressBar(bar, TFT_LIGHTGREY, value, 172, 1810);
			bar.pushSprite(110, v_offset);
		}
	}
}

void UI_pushDataFrameIndication(uint8_t * results, int count) {
	for (int i=0; i < count; i++) {
		rx_frame_indicator_bar.scroll(1, 0);
		uint16_t color = TFT_DARKGREY;
		switch ((crsf_packet_result_e) results[i]) {
			case CRSF_RESULT_PACKET_OK:
				color = TFT_GREEN;
				break;
			case CRSF_RESULT_PACKET_TIMEOUT:
				color = TFT_RED;
				break;
			default:
				color = TFT_DARKGREY;
		}
		// draw vertical line
		rx_frame_indicator_bar.drawFastVLine(1, 0, rx_frame_indicator_bar.height(), color);
		// draw small vertical bars every 25 pixels
		if ((packet_counter + i) % 25 == 0) {
			rx_frame_indicator_bar.drawFastVLine(1, rx_frame_indicator_bar.height() - 5, rx_frame_indicator_bar.height(), TFT_WHITE);
		}
	}
	rx_frame_indicator_bar.drawRect(0, 0, rx_frame_indicator_bar.width(), rx_frame_indicator_bar.height(), TFT_DARKCYAN);
	rx_frame_indicator_bar.pushSprite(110, 70);
	packet_counter += count;
}

