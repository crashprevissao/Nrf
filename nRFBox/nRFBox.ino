/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include "ícone.h"
#include "setting.h"
#include "config.h"
[span_7](start_span)#include <ELECHOUSE_CC1101_SRC_DRV.h> // Inclui a biblioteca de controle do módulo CC1101[span_7](end_span)

[span_8](start_span)extern uint8_t oledBrightness;[span_8](end_span)

[span_9](start_span)// Aumentado o número de itens de 12 para 13 para acomodar o CC1101[span_9](end_span)
[span_10](start_span)const int NUM_ITEMS = 13;[span_10](end_span)
[span_11](start_span)const int MAX_ITEM_LENGTH = 20;[span_11](end_span)

// Configuração de Pinos para o CC1101
#define CC1101_CSN   15 
#define CC1101_GDO0   4 

// Variável global para armazenar os tempos de pulso capturados do sinal de RF
[span_12](start_span)unsigned long sinalCapturado = 0;[span_12](end_span)

[span_13](start_span)// Associação de ícones (agora incluindo o bitmap_icon_cc1101 na 13ª posição)[span_13](end_span)
const unsigned char* bitmap_icons[NUM_ITEMS] = {
  bitmap_icon_scanner, bitmap_icon_analyzer, bitmap_icon_jammer, bitmap_icon_kill,
  bitmap_icon_ble_jammer, bitmap_icon_spoofer, bitmap_icon_apple, bitmap_icon_ble,
  bitmap_icon_wifi, bitmap_icon_wifi_jammer, bitmap_icon_about, 
  bitmap_icon_setting, bitmap_icon_cc1101 
};

[span_14](start_span)// Nome do novo console exibido no Menu[span_14](end_span)
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {  
  "Scanner", "Analyzer", "WLAN Jammer", "Proto Kill", "BLE Jammer",
  "BLE Spoofer", "Sour Apple", "BLE Scan", "WiFi Scan", 
  "Deauther", "About", "Setting", "Sub-GHz Console"
};

[span_15](start_span)// Declaração antecipada da nossa nova função de controle[span_15](end_span)
[span_16](start_span)void subGhzConsoleSetup();[span_16](end_span)

void (*menu_functions[NUM_ITEMS])() = {
  Scanner::scannerSetup, Analyzer::analyzerSetup, Jammer::jammerSetup,
  ProtoKill::blackoutSetup, BleJammer::blejammerSetup, Spoofer::spooferSetup,
  SourApple::sourappleSetup, BleScan::bleScanSetup, WifiScan::wifiScanSetup,
  Deauther::deautherSetup, About::aboutSetup, Setting::settingSetup,
  [span_17](start_span)subGhzConsoleSetup // Mapeia o clique do item 13 para rodar o nosso console[span_17](end_span)
};

[span_18](start_span)int item_selected = 0;[span_18](end_span)
[span_19](start_span)int current_screen = 0;[span_19](end_span)

// --- LÓGICA DE CONTROLE DO MÓDULO CC1101 ---

void executarSubGhzRX() {
  [span_20](start_span)u8g2.clearBuffer();[span_20](end_span)
  [span_21](start_span)u8g2.setFont(u8g2_font_6x10_tr);[span_21](end_span)
  [span_22](start_span)u8g2.drawStr(10, 25, "A ESCUTAR 433MHZ...");[span_22](end_span)
  [span_23](start_span)u8g2.drawStr(10, 40, "Aperte o controle");[span_23](end_span)
  [span_24](start_span)u8g2.sendBuffer();[span_24](end_span)

  [span_25](start_span)ELECHOUSE_cc1101.SetRx();[span_25](end_span)
  [span_26](start_span)unsigned long tempoLimite = millis();[span_26](end_span)
  [span_27](start_span)bool sucesso = false;[span_27](end_span)

  [span_28](start_span)// Monitora o pino de dados por até 10 segundos[span_28](end_span)
  while (millis() - tempoLimite < 10000) {
    [span_29](start_span)if (digitalRead(CC1101_GDO0) == HIGH) {[span_29](end_span)
      [span_30](start_span)unsigned long duracao = pulseIn(CC1101_GDO0, HIGH, 60000);[span_30](end_span)
      [span_31](start_span)if (duracao > 150) { // Valida se é um pulso quadrado real de RF[span_31](end_span)
        [span_32](start_span)sinalCapturado = duracao;[span_32](end_span)
        [span_33](start_span)sucesso = true;[span_33](end_span)
        [span_34](start_span)break;[span_34](end_span)
      }
    }
    // Permite sair se o botão de seleção for pressionado novamente
    [span_35](start_span)if (readButton(BUTTON_SELECT_PIN)) {[span_35](end_span)
      [span_36](start_span)delay(200);[span_36](end_span)
      [span_37](start_span)break;[span_37](end_span)
    }
  }

  ELECHOUSE_cc1101.setSidle(); [span_38](start_span)// Desativa o rádio para poupar bateria[span_38](end_span)

  [span_39](start_span)u8g2.clearBuffer();[span_39](end_span)
  [span_40](start_span)if (sucesso) {[span_40](end_span)
    [span_41](start_span)u8g2.drawStr(15, 30, "SINAL CAPTURADO!");[span_41](end_span)
    [span_42](start_span)setNeoPixelColour("green");[span_42](end_span)
  [span_43](start_span)} else {[span_43](end_span)
    [span_44](start_span)u8g2.drawStr(20, 30, "SEM SINAL / TIMEOUT");[span_44](end_span)
    [span_45](start_span)setNeoPixelColour("red");[span_45](end_span)
  }
  [span_46](start_span)u8g2.sendBuffer();[span_46](end_span)
  [span_47](start_span)delay(1500);[span_47](end_span)
  [span_48](start_span)setNeoPixelColour("black");[span_48](end_span)
}

void executarSubGhzTX() {
  [span_49](start_span)u8g2.clearBuffer();[span_49](end_span)
  [span_50](start_span)u8g2.setFont(u8g2_font_6x10_tr);[span_50](end_span)
  
  [span_51](start_span)if (sinalCapturado == 0) {[span_51](end_span)
    [span_52](start_span)u8g2.drawStr(10, 30, "Nenhum sinal salvo!");[span_52](end_span)
    [span_53](start_span)u8g2.sendBuffer();[span_53](end_span)
    [span_54](start_span)delay(1500);[span_54](end_span)
    [span_55](start_span)return;[span_55](end_span)
  }

  [span_56](start_span)u8g2.drawStr(20, 30, "A TRANSMITIR...");[span_56](end_span)
  [span_57](start_span)u8g2.sendBuffer();[span_57](end_span)
  [span_58](start_span)setNeoPixelColour("blue");[span_58](end_span)

  ELECHOUSE_cc1101.SetTx(); [span_59](start_span)// Ativa o transmissor físico[span_59](end_span)

  // Emite o padrão de pulso modulado gravado em loop para replicação estável
  [span_60](start_span)for (int f = 0; f < 12; f++) {[span_60](end_span)
    [span_61](start_span)digitalWrite(CC1101_GDO0, HIGH);[span_61](end_span)
    [span_62](start_span)delayMicroseconds(sinalCapturado);[span_62](end_span)
    [span_63](start_span)digitalWrite(CC1101_GDO0, LOW);[span_63](end_span)
    [span_64](start_span)delayMicroseconds(sinalCapturado * 2);[span_64](end_span)
  }

  ELECHOUSE_cc1101.setSidle(); [span_65](start_span)// Coloca o rádio em modo de espera[span_65](end_span)
  [span_66](start_span)setNeoPixelColour("black");[span_66](end_span)
}

[span_67](start_span)// Menu interno do Sub-GHz para selecionar entre Receber ou Transmitir[span_67](end_span)
void subGhzConsoleSetup() {
  [span_68](start_span)int subOpcao = 0;[span_68](end_span)
  [span_69](start_span)while (true) {[span_69](end_span)
    [span_70](start_span)u8g2.clearBuffer();[span_70](end_span)
    [span_71](start_span)u8g2.setFont(u8g2_font_6x10_tr);[span_71](end_span)
    [span_72](start_span)u8g2.drawStr(15, 12, "--- CONSOLE SUB-GHZ ---");[span_72](end_span)
    [span_73](start_span)if (subOpcao == 0) u8g2.drawStr(10, 32, "> 1. CAPTURAR (RX)");[span_73](end_span)
    [span_74](start_span)else u8g2.drawStr(10, 32, "  1. CAPTURAR (RX)");[span_74](end_span)
    [span_75](start_span)if (subOpcao == 1) u8g2.drawStr(10, 48, "> 2. REPRODUZIR (TX)");[span_75](end_span)
    [span_76](start_span)else u8g2.drawStr(10, 48, "  2. REPRODUZIR (TX)");[span_76](end_span)
    [span_77](start_span)u8g2.drawStr(10, 62, "Pressione L p/ Sair");[span_77](end_span)
    [span_78](start_span)u8g2.sendBuffer();[span_78](end_span)

    [span_79](start_span)if (readButton(BUTTON_DOWN_PIN) || readButton(BTN_PIN_RIGHT)) {[span_79](end_span)
      [span_80](start_span)subOpcao = 1;[span_80](end_span)
    [span_81](start_span)}
    if (readButton(BUTTON_UP_PIN) || readButton(BTN_PIN_LEFT)) {[span_81](end_span)
      [span_82](start_span)subOpcao = 0;[span_82](end_span)
    [span_83](start_span)}
    if (readButton(BUTTON_SELECT_PIN)) {[span_83](end_span)
      [span_84](start_span)delay(200);[span_84](end_span)
      [span_85](start_span)if (subOpcao == 0) executarSubGhzRX();[span_85](end_span)
      [span_86](start_span)else executarSubGhzTX();[span_86](end_span)
    [span_87](start_span)}
    
    if (readButton(BTN_PIN_LEFT) && subOpcao == 0) {[span_87](end_span)
      [span_88](start_span)delay(200);[span_88](end_span)
      [span_89](start_span)break;[span_89](end_span)
    }
  }
}

// --- FIM DA LÓGICA DO CC1101 ---

void setup() {
  [span_90](start_span)Serial.begin(115200);[span_90](end_span)
  [span_91](start_span)initNeoPixel();[span_91](end_span)
  [span_92](start_span)initStorage();[span_92](end_span)
  [span_93](start_span)loadSettings();[span_93](end_span)
  [span_94](start_span)initDisplay();[span_94](end_span)
  [span_95](start_span)pinMode(BTN_PIN_LEFT, INPUT_PULLUP);[span_95](end_span)
  [span_96](start_span)pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);[span_96](end_span)
  [span_97](start_span)pinMode(BUTTON_UP_PIN, INPUT_PULLUP);[span_97](end_span)
  [span_98](start_span)pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);[span_98](end_span)
  [span_99](start_span)pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);[span_99](end_span)

  [span_100](start_span)// Inicialização do hardware CC1101 acoplado no SPI nativo (Pinos 18, 19, 23)[span_100](end_span)
  [span_101](start_span)ELECHOUSE_cc1101.Init();[span_101](end_span)
  [span_102](start_span)ELECHOUSE_cc1101.setCCMode(1);[span_102](end_span)
  [span_103](start_span)// Configura modulação genérica ASK/OOK (usada em controles de TV/Portões)[span_103](end_span)
  [span_104](start_span)ELECHOUSE_cc1101.setFrequency(433.92);[span_104](end_span)
  [span_105](start_span)// Ajusta para a frequência central regulamentada no Brasil[span_105](end_span)
  [span_106](start_span)ELECHOUSE_cc1101.setSpiPin(18, 19, 23, CC1101_CSN);[span_106](end_span)
  [span_107](start_span)pinMode(CC1101_GDO0, INPUT);[span_107](end_span)

  [span_108](start_span)drawMenu();[span_108](end_span)
}

void loop() {
  [span_109](start_span)if (current_screen == 0) {[span_109](end_span)
    [span_110](start_span)const int icons_per_row = 3;[span_110](end_span)
    [span_111](start_span)if (readButton(BTN_PIN_LEFT)) {[span_111](end_span)
      [span_112](start_span)item_selected = max(0, item_selected - 1);[span_112](end_span)
      [span_113](start_span)drawMenu();[span_113](end_span)
    [span_114](start_span)}
    if (readButton(BTN_PIN_RIGHT)) {[span_114](end_span)
      [span_115](start_span)item_selected = min(NUM_ITEMS - 1, item_selected + 1);[span_115](end_span)
      [span_116](start_span)drawMenu();[span_116](end_span)
    }
    [span_117](start_span)if (readButton(BUTTON_UP_PIN)) {[span_117](end_span)
      [span_118](start_span)item_selected = max(0, item_selected - icons_per_row);[span_118](end_span)
      [span_119](start_span)drawMenu();[span_119](end_span)
    [span_120](start_span)}
    if (readButton(BUTTON_DOWN_PIN)) {[span_120](end_span)
      [span_121](start_span)item_selected = min(NUM_ITEMS - 1, item_selected + icons_per_row);[span_121](end_span)
      [span_122](start_span)drawMenu();[span_122](end_span)
    }
    [span_123](start_span)if (readButton(BUTTON_SELECT_PIN)) {[span_123](end_span)
      [span_124](start_span)current_screen = 1;[span_124](end_span)
      [span_125](start_span)for (int cycle = 0; cycle < 2; cycle++) {[span_125](end_span)
        [span_126](start_span)for (int i = 0; i < 3; i++) {[span_126](end_span)
          [span_127](start_span)u8g2.clearBuffer();[span_127](end_span)
          [span_128](start_span)u8g2.setFont(u8g2_font_6x10_tr);[span_128](end_span)
          [span_129](start_span)u8g2.drawStr(30, 32, "Loading");[span_129](end_span)
    
          [span_130](start_span)String dots = "";[span_130](end_span)
          [span_131](start_span)for (int j = 0; j <= i; j++) {[span_131](end_span)
            [span_132](start_span)dots += ".";[span_132](end_span)
            [span_133](start_span)setNeoPixelColour("white");[span_133](end_span)
          }
          [span_134](start_span)u8g2.drawStr(75, 32, dots.c_str());[span_134](end_span)
          [span_135](start_span)u8g2.sendBuffer();[span_135](end_span)
          [span_136](start_span)delay(150);[span_136](end_span)
          [span_137](start_span)setNeoPixelColour("black");[span_137](end_span)
        [span_138](start_span)}
      }
      
      // Executa a função associada ao item do menu selecionado[span_138](end_span)
      [span_139](start_span)menu_functions[item_selected]();[span_139](end_span)
      [span_140](start_span)current_screen = 0;[span_140](end_span)
      [span_141](start_span)drawMenu();[span_141](end_span)
    }
  }
}

void drawMenu() {
  [span_142](start_span)u8g2.clearBuffer();[span_142](end_span)
  [span_143](start_span)u8g2.setContrast(oledBrightness);[span_143](end_span)

  [span_144](start_span)int icon_width = 16;[span_144](end_span)
  [span_145](start_span)int icon_height = 16;[span_145](end_span)
  [span_146](start_span)int padding_x = 18;[span_146](end_span)
  [span_147](start_span)int padding_y = 6;[span_147](end_span)
  [span_148](start_span)int start_x = 14;[span_148](end_span)
  [span_149](start_span)int start_y = 4;[span_149](end_span)

  [span_150](start_span)int icons_per_row = 3;[span_150](end_span)
  [span_151](start_span)int current_row = item_selected / icons_per_row;[span_151](end_span)
  [span_152](start_span)int start_item = max(0, (current_row - 1) * icons_per_row);[span_152](end_span)
  [span_153](start_span)if (start_item + 6 < NUM_ITEMS && current_row > 0) {[span_153](end_span)
    // Mantém o scroll dinâmico
  [span_154](start_span)} else {[span_154](end_span)
    [span_155](start_span)start_item = max(0, NUM_ITEMS - 6);[span_155](end_span)
    [span_156](start_span)if(item_selected < 6) start_item = 0;[span_156](end_span)
  }

  [span_157](start_span)for (int i = 0; i < 6; i++) {[span_157](end_span)
    [span_158](start_span)int item_idx = start_item + i;[span_158](end_span)
    [span_159](start_span)if (item_idx >= NUM_ITEMS) break;[span_159](end_span)

    [span_160](start_span)int row = i / icons_per_row;[span_160](end_span)
    [span_161](start_span)int col = i % icons_per_row;[span_161](end_span)
    [span_162](start_span)int x = start_x + col * (icon_width + padding_x);[span_162](end_span)
    [span_163](start_span)int y = start_y + row * (icon_height + padding_y);[span_163](end_span)
    [span_164](start_span)u8g2.drawXBMP(x, y, icon_width, icon_height, bitmap_icons[item_idx]);[span_164](end_span)

    [span_165](start_span)if (item_idx == item_selected) {[span_165](end_span)
      [span_166](start_span)u8g2.drawFrame(x - 2, y - 2, icon_width + 4, icon_height + 4);[span_166](end_span)
      [span_167](start_span)u8g2.setFont(u8g2_font_6x10_tr);[span_167](end_span)
      [span_168](start_span)int text_width = u8g2.getStrWidth(menu_items[item_idx]);[span_168](end_span)
      [span_169](start_span)int text_x = (128 - text_width) / 2;[span_169](end_span)
      [span_170](start_span)u8g2.drawStr(text_x, 58, menu_items[item_idx]);[span_170](end_span)
    }
  }
  [span_171](start_span)u8g2.sendBuffer();[span_171](end_span)
}
