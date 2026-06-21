/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include "ícone.h"
#include "setting.h"
#include "config.h"
#include <ELECHOUSE_CC1101_SRC_Arduino.h> // Biblioteca corrigida do CC1101

extern uint8_t oledBrightness;[span_4](start_span)[span_4](end_span)

// Menu expandido para 13 itens para incluir o CC1101[span_5](start_span)[span_5](end_span)
const int NUM_ITEMS = 13;[span_6](start_span)[span_6](end_span)
const int MAX_ITEM_LENGTH = 20;[span_7](start_span)[span_7](end_span)

// Configuração de Pinos para o CC1101[span_8](start_span)[span_8](end_span)
#define CC1101_CSN   15 
#define CC1101_GDO0   4 

// Armazenamento do sinal de RF capturado[span_9](start_span)[span_9](end_span)
unsigned long sinalCapturado = 0;[span_10](start_span)[span_10](end_span)

// Matriz de ícones associando o bitmap do CC1101 na 13ª posição
const unsigned char* bitmap_icons[NUM_ITEMS] = {
  bitmap_icon_scanner, bitmap_icon_analyzer, bitmap_icon_jammer, bitmap_icon_kill,
  bitmap_icon_ble_jammer, bitmap_icon_spoofer, bitmap_icon_apple, bitmap_icon_ble,
  bitmap_icon_wifi, bitmap_icon_wifi_jammer, bitmap_icon_about, 
  bitmap_icon_setting, bitmap_icon_cc1101 
};

// Nomes exibidos no menu da interface oled[span_11](start_span)[span_11](end_span)
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {  
  "Scanner", "Analyzer", "WLAN Jammer", "Proto Kill", "BLE Jammer",
  "BLE Spoofer", "Sour Apple", "BLE Scan", "WiFi Scan", 
  "Deauther", "About", "Setting", "Sub-GHz Console"
};

void subGhzConsoleSetup();[span_12](start_span)[span_12](end_span)

void (*menu_functions[NUM_ITEMS])() = {
  Scanner::scannerSetup, Analyzer::analyzerSetup, Jammer::jammerSetup,
  ProtoKill::blackoutSetup, BleJammer::blejammerSetup, Spoofer::spooferSetup,
  SourApple::sourappleSetup, BleScan::bleScanSetup, WifiScan::wifiScanSetup,
  Deauther::deautherSetup, About::aboutSetup, Setting::settingSetup,
  subGhzConsoleSetup 
};

int item_selected = 0;[span_13](start_span)[span_13](end_span)
int current_screen = 0;[span_14](start_span)[span_14](end_span)

// --- LÓGICA DE CONTROLE DO MÓDULO CC1101 ---

void executarSubGhzRX() {
  u8g2.clearBuffer();[span_15](start_span)[span_15](end_span)
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 25, "A ESCUTAR 433MHZ...");
  u8g2.drawStr(10, 40, "Aperte o controle");
  u8g2.sendBuffer();

  ELECHOUSE_cc1101.SetRx();
  unsigned long tempoLimite = millis();
  bool sucesso = false;

  while (millis() - tempoLimite < 10000) {
    if (digitalRead(CC1101_GDO0) == HIGH) {
      unsigned long duracao = pulseIn(CC1101_GDO0, HIGH, 60000);
      if (duracao > 150) { 
        sinalCapturado = duracao;
        sucesso = true;
        break;
      }
    }
    if (readButton(BUTTON_SELECT_PIN)) { 
      delay(200);
      break; 
    }
  }

  ELECHOUSE_cc1101.setSidle();

  u8g2.clearBuffer();
  if (sucesso) {
    u8g2.drawStr(15, 30, "SINAL CAPTURADO!");
    setNeoPixelColour("green");
  } else {
    u8g2.drawStr(20, 30, "SEM SINAL / TIMEOUT");
    setNeoPixelColour("red");
  }
  u8g2.sendBuffer();
  delay(1500);
  setNeoPixelColour("black");
}

void executarSubGhzTX() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  
  if (sinalCapturado == 0) {
    u8g2.drawStr(10, 30, "Nenhum sinal salvo!");
    u8g2.sendBuffer();
    delay(1500);
    return;
  }

  u8g2.drawStr(20, 30, "A TRANSMITIR...");
  u8g2.sendBuffer();
  setNeoPixelColour("blue");

  ELECHOUSE_cc1101.SetTx();

  for (int f = 0; f < 12; f++) {
    digitalWrite(CC1101_GDO0, HIGH);
    delayMicroseconds(sinalCapturado);
    digitalWrite(CC1101_GDO0, LOW);
    delayMicroseconds(sinalCapturado * 2);
  }

  ELECHOUSE_cc1101.setSidle();
  setNeoPixelColour("black");
}

void subGhzConsoleSetup() {
  int subOpcao = 0;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(15, 12, "--- CONSOLE SUB-GHZ ---");
    
    if (subOpcao == 0) u8g2.drawStr(10, 32, "> 1. CAPTURAR (RX)");
    else u8g2.drawStr(10, 32, "  1. CAPTURAR (RX)");

    if (subOpcao == 1) u8g2.drawStr(10, 48, "> 2. REPRODUZIR (TX)");
    else u8g2.drawStr(10, 48, "  2. REPRODUZIR (TX)");

    u8g2.drawStr(10, 62, "Pressione L p/ Sair");
    u8g2.sendBuffer();

    if (readButton(BUTTON_DOWN_PIN) || readButton(BTN_PIN_RIGHT)) {
      subOpcao = 1;
    }
    if (readButton(BUTTON_UP_PIN) || readButton(BTN_PIN_LEFT)) {
      subOpcao = 0;
    }
    if (readButton(BUTTON_SELECT_PIN)) {
      delay(200);
      if (subOpcao == 0) executarSubGhzRX();
      else executarSubGhzTX();
    }
    
    if (readButton(BTN_PIN_LEFT) && subOpcao == 0) {
      delay(200);
      break;
    }
  }
}

// --- FIM DA LÓGICA DO CC1101 ---

void setup() {
  Serial.begin(115200);
  initNeoPixel();
  initStorage();
  loadSettings();
  initDisplay();
  
  pinMode(BTN_PIN_LEFT, INPUT_PULLUP);
  pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setCCMode(1);
  ELECHOUSE_cc1101.setFrequency(433.92);
  ELECHOUSE_cc1101.setSpiPin(18, 19, 23, CC1101_CSN);
  pinMode(CC1101_GDO0, INPUT);

  drawMenu();
}

void loop() {
  if (current_screen == 0) {
    const int icons_per_row = 3;

    if (readButton(BTN_PIN_LEFT)) {
      item_selected = max(0, item_selected - 1);
      drawMenu();
    }
    if (readButton(BTN_PIN_RIGHT)) {
      item_selected = min(NUM_ITEMS - 1, item_selected + 1);
      drawMenu();
    }
    if (readButton(BUTTON_UP_PIN)) {
      item_selected = max(0, item_selected - icons_per_row);
      drawMenu();
    }
    if (readButton(BUTTON_DOWN_PIN)) {
      item_selected = min(NUM_ITEMS - 1, item_selected + icons_per_row);
      drawMenu();
    }
    if (readButton(BUTTON_SELECT_PIN)) {
      current_screen = 1;
      for (int cycle = 0; cycle < 2; cycle++) {
        for (int i = 0; i < 3; i++) {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_6x10_tr);
          u8g2.drawStr(30, 32, "Loading");
    
          String dots = ""; 
          for (int j = 0; j <= i; j++) { 
            dots += "."; 
            setNeoPixelColour("white"); 
          }
          u8g2.drawStr(75, 32, dots.c_str());
          u8g2.sendBuffer();
          delay(150);
          setNeoPixelColour("black");
        }
      }
      
      menu_functions[item_selected]();
      
      current_screen = 0;
      drawMenu();
    }
  }
}

void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setContrast(oledBrightness);

  int icon_width = 16;
  int icon_height = 16;
  int padding_x = 18;
  int padding_y = 6;
  int start_x = 14;
  int start_y = 4;

  int icons_per_row = 3;

  int current_row = item_selected / icons_per_row;
  int start_item = max(0, (current_row - 1) * icons_per_row);
  if (start_item + 6 < NUM_ITEMS && current_row > 0) {
    // Mantém o scroll dinâmico
  } else {
    start_item = max(0, NUM_ITEMS - 6);
    if(item_selected < 6) start_item = 0;
  }

  for (int i = 0; i < 6; i++) {
    int item_idx = start_item + i;
    if (item_idx >= NUM_ITEMS) break;

    int row = i / icons_per_row;
    int col = i % icons_per_row;

    int x = start_x + col * (icon_width + padding_x);
    int y = start_y + row * (icon_height + padding_y);

    u8g2.drawXBMP(x, y, icon_width, icon_height, bitmap_icons[item_idx]);

    if (item_idx == item_selected) {
      u8g2.drawFrame(x - 2, y - 2, icon_width + 4, icon_height + 4);
      
      u8g2.setFont(u8g2_font_6x10_tr);
      int text_width = u8g2.getStrWidth(menu_items[item_idx]);
      int text_x = (128 - text_width) / 2;
      u8g2.drawStr(text_x, 58, menu_items[item_idx]);
    }
  }
  u8g2.sendBuffer();
}
