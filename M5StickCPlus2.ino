/* 
  Smart Watch para M5StickCPlus2
  Versão: 2 (Sem Ícone de Carregamento)
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

// Configurações de exibição
#define BRILHO_DIURNO 100
#define BRILHO_NOTURNO 30
#define TEMPO_TELA_LIGADA 15000 // 15 segundos

// Configurações do modo noturno
#define HORA_INICIO_NOTURNO 18 // 18h (6PM)
#define HORA_FIM_NOTURNO 6     // 6h (6AM)

// =========== VARIÁVEIS GLOBAIS ===========
uint8_t brilho_atual = BRILHO_DIURNO;
bool modo_noturno = false;
bool tela_ligada = false;
unsigned long ultimo_toque = 0;

// =========== CONFIGURAÇÃO INICIAL ===========
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Configurações de energia
  setCpuFrequencyMhz(80);
  WiFi.setSleep(WIFI_PS_MIN_MODEM);

  // Inicialização da tela
  M5.Display.setRotation(3);
  M5.Display.setBrightness(0);
  M5.Display.fillScreen(TFT_BLACK);

  // Conexão WiFi e NTP
  conectar_wifi();
  configurar_ntp();
  WiFi.disconnect(true);
}

void conectar_wifi() {
  WiFi.begin(SSID, SENHA);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void configurar_ntp() {
  configTime(FUSO_HORARIO, AJUSTE_VERAO, SERVIDOR_NTP);
  delay(1000);
}

// =========== LOOP PRINCIPAL ===========
void loop() {
  M5.update();

  if(M5.BtnA.wasPressed()) {
    if(!tela_ligada) {
      ligar_tela();
    }
    ultimo_toque = millis();
  }

  if(tela_ligada) {
    struct tm info_tempo;
    if(getLocalTime(&info_tempo)) {
      verificar_modo_noturno(info_tempo);
      atualizar_tela(info_tempo);
    } else {
      tela_erro();
    }

    if(millis() - ultimo_toque > TEMPO_TELA_LIGADA) {
      desligar_tela();
    }
  }
  
  delay(100);
}

// =========== FUNÇÕES DE CONTROLE ===========
void ligar_tela() {
  M5.Display.setBrightness(brilho_atual);
  tela_ligada = true;
  M5.Display.fillScreen(TFT_BLACK);
}

void desligar_tela() {
  M5.Display.setBrightness(0);
  tela_ligada = false;
}

void verificar_modo_noturno(struct tm &tempo) {
  bool novo_modo = (tempo.tm_hour >= HORA_INICIO_NOTURNO || tempo.tm_hour < HORA_FIM_NOTURNO);
  
  if(novo_modo != modo_noturno) {
    modo_noturno = novo_modo;
    brilho_atual = modo_noturno ? BRILHO_NOTURNO : BRILHO_DIURNO;
    if(tela_ligada) {
      M5.Display.setBrightness(brilho_atual);
    }
  }
}

// =========== FUNÇÕES DE TELA ===========
void desenhar_bateria() {
  int nivel = M5.Power.getBatteryLevel();
  
  int x = 150;
  int y = 5;
  uint16_t cor = TFT_GREEN;
  if(nivel <= 20) cor = TFT_RED;
  else if(nivel <= 50) cor = TFT_YELLOW;

  // Ícone de bateria simplificado
  M5.Display.drawRect(x, y, 25, 12, TFT_WHITE);
  M5.Display.fillRect(x+25, y+3, 2, 6, TFT_WHITE);
  M5.Display.fillRect(x+2, y+2, map(nivel, 0, 100, 0, 21), 8, cor);

  // Apenas porcentagem (sem ícone de carregamento)
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(String(nivel) + "%", x+30, y+1);
}

void atualizar_tela(struct tm &tempo) {
  M5.Display.fillScreen(TFT_BLACK);
  
  // Hora
  M5.Display.setTextSize(3);
  M5.Display.setCursor(20, 35);
  M5.Display.setTextColor(modo_noturno ? TFT_GREENYELLOW : TFT_GREEN, TFT_BLACK);
  M5.Display.printf("%02d:%02d:%02d", tempo.tm_hour, tempo.tm_min, tempo.tm_sec);

  // Data
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 80);
  M5.Display.setTextColor(modo_noturno ? TFT_CYAN : TFT_BLUE, TFT_BLACK);
  M5.Display.printf("%02d/%02d/%04d", tempo.tm_mday, tempo.tm_mon + 1, tempo.tm_year + 1900);

  desenhar_bateria();
}

void tela_erro() {
  M5.Display.fillScreen(TFT_RED);
  M5.Display.setTextColor(TFT_WHITE, TFT_RED);
  M5.Display.drawString("Erro NTP!", 10, 50);
  delay(2000);
  ESP.restart();
}