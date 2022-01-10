#include <M5Core2.h>
#include "ui.h"

static TFT_eSprite sprite_rssi_text (&M5.Lcd); 
static TFT_eSprite sprite_rssi_pg (&M5.Lcd);
static TFT_eSprite sprite_lq_text (&M5.Lcd);
static TFT_eSprite sprite_lq_pg (&M5.Lcd);
static TFT_eSprite sprite_tx_pwr_text (&M5.Lcd);

static void spriteBasicSetup(TFT_eSprite & s) {
    s.setTextSize(2);
}

static void spriteClear(TFT_eSprite & s) {
    s.setCursor(0, 0);
    s.fillSprite(0);
}

static void drawProgressBar(TFT_eSprite & s, uint16_t color, int val, int v_min, int v_max) {
    s.fillSprite(0);
    s.drawRect(0, 0, s.width()-1, s.height()-1, color); // draw outline
    s.fillRect(0, 0, map(val, v_min, v_max, 0, s.width()-1), s.height()-1, color);
}

void UI_setup() {
    sprite_rssi_text.createSprite(100, 20);
    sprite_rssi_text.setTextSize(2);
    sprite_rssi_text.setTextColor(TFT_CYAN);

    sprite_lq_text.createSprite(100, 20);
    sprite_lq_text.setTextSize(2);
    sprite_lq_text.setTextColor(TFT_GREENYELLOW);

    sprite_tx_pwr_text.createSprite(100, 20);
    sprite_tx_pwr_text.setTextSize(2);
    sprite_tx_pwr_text.setTextColor(TFT_DARKGREY);

    sprite_lq_pg.createSprite(200, 20);
    sprite_rssi_pg.createSprite(200, 20);
}

void UI_setLq(int value) {
    spriteClear(sprite_lq_text);
    sprite_lq_text.printf("LQ %d", value);
    sprite_lq_text.pushSprite(10, 40);
    drawProgressBar(sprite_lq_pg, TFT_GREENYELLOW, value, 0, 100);
    sprite_lq_pg.pushSprite(110, 40);
}

void UI_setRssi(int value) {
    spriteClear(sprite_rssi_text);
    sprite_rssi_text.printf("RS %d", value);
    sprite_rssi_text.pushSprite(10, 10);
    drawProgressBar(sprite_rssi_pg, TFT_CYAN, value, 130, 50);
    sprite_rssi_pg.pushSprite(110, 10);
}

void UI_setTxPwr(int value) {
    spriteClear(sprite_tx_pwr_text);
    sprite_tx_pwr_text.printf("PW %d", value);
    sprite_tx_pwr_text.pushSprite(10, 70);
}

