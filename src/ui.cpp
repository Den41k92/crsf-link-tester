
#include "ui.h"
#include <M5Core2.h>

static TFT_eSprite rssi_text(&M5.Lcd);
static TFT_eSprite rssi_bar(&M5.Lcd);
static TFT_eSprite lq_text(&M5.Lcd);
static TFT_eSprite lq_bar(&M5.Lcd);
static TFT_eSprite tx_pwr_text(&M5.Lcd);
static TFT_eSprite link_rate_text(&M5.Lcd);

static TFT_eSprite ch1_text(&M5.Lcd);
static TFT_eSprite ch2_text(&M5.Lcd);
static TFT_eSprite ch3_text(&M5.Lcd);
static TFT_eSprite ch4_text(&M5.Lcd);
static TFT_eSprite ch5_text(&M5.Lcd);
static TFT_eSprite ch6_text(&M5.Lcd);
static TFT_eSprite ch7_text(&M5.Lcd);
static TFT_eSprite ch8_text(&M5.Lcd);

static TFT_eSprite ch1_bar(&M5.Lcd);
static TFT_eSprite ch2_bar(&M5.Lcd);
static TFT_eSprite ch3_bar(&M5.Lcd);
static TFT_eSprite ch4_bar(&M5.Lcd);
static TFT_eSprite ch5_bar(&M5.Lcd);
static TFT_eSprite ch6_bar(&M5.Lcd);
static TFT_eSprite ch7_bar(&M5.Lcd);
static TFT_eSprite ch8_bar(&M5.Lcd);

static const TFT_eSprite * channel_texts [] = {
	&ch1_text, &ch2_text, &ch3_text, &ch4_text, &ch5_text, &ch6_text, &ch7_text, &ch8_text
};

static const TFT_eSprite * channel_bars [] = {
	&ch1_bar, &ch2_bar, &ch3_bar, &ch4_bar, &ch5_bar, &ch6_bar, &ch7_bar, &ch8_bar
};

static int rssi_scale_min = 120;
static int rssi_scale_max = 50;


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
	
	for (uint8_t i=0; i < 8; i++) {
		createElement((TFT_eSprite&) *channel_texts[i], 1, 90, 10, TFT_WHITE);
		createElement((TFT_eSprite&) *channel_bars[i], 1, 210, 10, TFT_LIGHTGREY);
	}

}

void UI_setLq(int percent) {
	clearSprite(lq_text);
	lq_text.printf("LQ %d", percent);
	lq_text.pushSprite(10, 40);
	drawProgressBar(lq_bar, TFT_GREENYELLOW, percent, 0, 100);
	lq_bar.pushSprite(110, 40);
}

void UI_setRssi(int dbm) {
	clearSprite(rssi_text);
	rssi_text.printf("RS %d", dbm);
	rssi_text.pushSprite(10, 10);
	drawProgressBar(rssi_bar, TFT_CYAN, dbm, rssi_scale_min, rssi_scale_max);
	rssi_bar.pushSprite(110, 10);
}

void UI_setTxPwr(int value) {
	clearSprite(tx_pwr_text);
	tx_pwr_text.printf("PW %d", value);
	tx_pwr_text.pushSprite(10, 70);
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

void UI_setChannels8(uint32_t * channel_data_8) {
	for (uint8_t i=0; i < 8; i++) {
		TFT_eSprite& text = (TFT_eSprite&) *channel_texts[i];
		TFT_eSprite& bar = (TFT_eSprite&) *channel_bars[i];
		int v_offset = 140 + 12 * i;

		clearSprite(text);
		text.printf("CH%d: %d", i+1, channel_data_8[i]);
		text.pushSprite(10, v_offset);

		clearSprite(bar);
		drawProgressBar(bar, TFT_LIGHTGREY, channel_data_8[i], 172, 1810);
		bar.pushSprite(110, v_offset);
	}

}