/* 
  Smart Watch para M5StickCPlus2
  Versão: 1.0 (Ajustes Finais)
  Autor: Bruno Nepomuceno
  Data: 05/2025
*/

#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <time.h>

// =========== CONSTANTES ===========
const char* SSID = "ELDORADO";
const char* SENHA = "G4solinaneles";
const char* SERVIDOR_NTP = "br.pool.ntp.org";
const long FUSO_HORARIO = -3 * 3600;
const int AJUSTE_VERAO = 0;

// Configurações de modo noturno
#define HORA_INICIO_NOTURNO 18
#define HORA_FIM_NOTURNO 6
#define BRILHO_DIURNO 100
#define BRILHO_NOTURNO 30

// =========== VARIÁVEIS GLOBAIS ===========
uint8_t brilho_atual = BRILHO_DIURNO;
bool modo_noturno = false;
unsigned long ultimo_update_bateria = 0;

// =========== CONFIGURAÇÃO INICIAL ===========
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Configurações de energia
  setCpuFrequencyMhz(80);
  WiFi.setSleep(WIFI_PS_MIN_MODEM);

  // Configuração da tela
  M5.Display.setRotation(3);
  M5.Display.setBrightness(brilho_atual);
  M5.Display.fillScreen(TFT_BLACK);

  // Conexão WiFi temporária
  conectar_wifi();
  configurar_ntp();
  WiFi.disconnect(true);
}

void conectar_wifi() {
  WiFi.begin(SSID, SENHA);
  M5.Display.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Display.print(".");
  }
  M5.Display.println("\nConectado!");
}

void configurar_ntp() {
  configTime(FUSO_HORARIO, AJUSTE_VERAO, SERVIDOR_NTP);
  delay(1000);
}

// =========== LOOP PRINCIPAL ===========
void loop() {
  struct tm info_tempo;
  
  if (!getLocalTime(&info_tempo)) {
    tela_erro();
    return;
  }

  verificar_modo_noturno(info_tempo);
  atualizar_tela(info_tempo);
  controle_brilho();
  atualizar_bateria();

  delay(1000);
}

// =========== FUNÇÕES DE ENERGIA ===========
void verificar_modo_noturno(struct tm &tempo) {
  bool novo_modo = (tempo.tm_hour >= HORA_INICIO_NOTURNO || tempo.tm_hour < HORA_FIM_NOTURNO);
  
  if (novo_modo != modo_noturno) {
    modo_noturno = novo_modo;
    brilho_atual = modo_noturno ? BRILHO_NOTURNO : BRILHO_DIURNO;
    M5.Display.setBrightness(brilho_atual);
  }
}

void atualizar_bateria() {
  if (millis() - ultimo_update_bateria > 10000) {
    desenhar_bateria();
    ultimo_update_bateria = millis();
  }
}

// =========== FUNÇÕES DE TELA (COM AJUSTES SOLICITADOS) ===========
void desenhar_bateria() {
  int nivel = M5.Power.getBatteryLevel();
  
  // Posição ajustada conforme solicitado (x=150)
  int x = 150;
  int y = 5;
  uint16_t cor = TFT_GREEN;
  if (nivel <= 20) cor = TFT_RED;
  else if (nivel <= 50) cor = TFT_YELLOW;

  // Ícone de bateria (tamanho otimizado)
  M5.Display.drawRect(x, y, 25, 12, TFT_WHITE);
  M5.Display.fillRect(x+25, y+3, 2, 6, TFT_WHITE);
  M5.Display.fillRect(x+2, y+2, map(nivel, 0, 100, 0, 21), 8, cor);

  // Texto de porcentagem (afastado 30px do ícone)
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(String(nivel) + "%", x+30, y+1);
}

void atualizar_tela(struct tm &tempo) {
  M5.Display.fillScreen(TFT_BLACK);
  
  // Hora (centralizada)
  M5.Display.setTextSize(3);
  M5.Display.setCursor(20, 35);
  M5.Display.setTextColor(modo_noturno ? TFT_GREENYELLOW : TFT_GREEN, TFT_BLACK);
  M5.Display.printf("%02d:%02d:%02d", tempo.tm_hour, tempo.tm_min, tempo.tm_sec);

  // Data (com ajuste solicitado Y=80)
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 80);  // Ajuste específico para Y=80
  M5.Display.setTextColor(modo_noturno ? TFT_CYAN : TFT_BLUE, TFT_BLACK);
  M5.Display.printf("%02d/%02d/%04d", tempo.tm_mday, tempo.tm_mon + 1, tempo.tm_year + 1900);

  desenhar_bateria();
}

// =========== FUNÇÕES AUXILIARES ===========
void tela_erro() {
  M5.Display.fillScreen(TFT_RED);
  M5.Display.drawString("Erro NTP!", 10, 50);
  delay(2000);
  ESP.restart();
}

void controle_brilho() {
  M5.update();
  
  if (M5.BtnA.wasPressed()) {
    brilho_atual = (brilho_atual + 50) % 255;
    M5.Display.setBrightness(brilho_atual);
    
    // Feedback visual
    M5.Display.fillRect(0, 0, 30, 10, TFT_BLACK);
    M5.Display.drawString(String((brilho_atual*100)/255) + "%", 0, 0);
    delay(1000);
  }
}