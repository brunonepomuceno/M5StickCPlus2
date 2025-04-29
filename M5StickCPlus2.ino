#include <M5StickCPlus2.h>

// Paleta Cyberpunk 2077
#define GLITCH_RED 0xF800
#define MATRIX_GREEN 0x07E0
#define DEEP_BLUE 0x0015
#define NEON_PURPLE 0x780F
#define TERMINAL_CYAN 0x07FF
#define WARNING_YELLOW 0xFFE0

// Variáveis de estado
bool overclockMode = false;
int netwatchCounter = 0;
unsigned long lastGlitch = 0;
bool blinkState = false;
bool lowBatteryWarningShown = false;

void playHackerSound(int freq = 1200, int duration = 80) {
  StickCP2.Speaker.tone(freq, duration);
  delay(duration);
  StickCP2.Speaker.end();
}

void glitchEffect() {
  for (int i = 0; i < 3; i++) {
    M5.Lcd.fillScreen(random(2) ? GLITCH_RED : NEON_PURPLE);
    playHackerSound();
    delay(30 + random(70));
    M5.Lcd.fillScreen(TFT_BLACK);
    delay(20);
  }
}

void typeWriter(String text, uint16_t color, int delayTime = 80) {
  M5.Lcd.setTextColor(color);
  for (int i = 0; i < text.length(); i++) {
    M5.Lcd.print(text.charAt(i));
    if (text.charAt(i) != ' ') {
      StickCP2.Speaker.tone(800 + (i * 30), 20);
      StickCP2.Speaker.end();
    }
    delay(delayTime + random(40));
  }
}

void drawTerminalHeader() {
  M5.Lcd.setTextColor(MATRIX_GREEN);
  M5.Lcd.println("[bl4k.code@root:~] $>_");
  M5.Lcd.drawFastHLine(0, 15, M5.Lcd.width(), TERMINAL_CYAN);
}

void scanlineBootEffect() {
  for (int i = 0; i < M5.Lcd.width(); i += 3) {
    M5.Lcd.drawFastVLine(i, 0, M5.Lcd.height(), MATRIX_GREEN);
    delay(10);
  }
  M5.Lcd.fillScreen(TFT_BLACK);
}

void bootAnimation() {
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1);
  
  typeWriter("> Initializing blackICE protocol...\n", TERMINAL_CYAN);
  typeWriter("> Bypassing Arasaka firewalls...\n", NEON_PURPLE);
  typeWriter("> WARNING: NetWatch detected\n", GLITCH_RED);
  typeWriter("> Access granted: ROOT privileges\n", MATRIX_GREEN);
  
  glitchEffect();
  scanlineBootEffect();
}

void drawCyberCursor() {
  blinkState = !blinkState;
  M5.Lcd.setTextColor(blinkState ? MATRIX_GREEN : TFT_BLACK);
  M5.Lcd.print("_");
}

void checkNetwatchEasterEgg() {
  netwatchCounter++;
  if (netwatchCounter >= 3) {
    M5.Lcd.fillScreen(DEEP_BLUE);
    M5.Lcd.setTextColor(GLITCH_RED);
    M5.Lcd.setCursor(0, 10);
    typeWriter("> NETWATCH TRACE DETECTED!\n", GLITCH_RED);
    typeWriter("> ACTIVATING ICEPHAGE...\n", NEON_PURPLE);
    
    for (int i = 0; i < 5; i++) {
      M5.Lcd.fillRect(0, 40, M5.Lcd.width(), 10, GLITCH_RED);
      StickCP2.Speaker.tone(2000, 100);
      delay(100);
      M5.Lcd.fillRect(0, 40, M5.Lcd.width(), 10, DEEP_BLUE);
      StickCP2.Speaker.end();
      delay(100);
    }
    
    netwatchCounter = 0;
    drawMainUI();
  }
}

void drawBatteryStatus() {
  int batteryLevel = M5.Power.getBatteryLevel();
  
  // Garante que o nível está entre 0-100%
  batteryLevel = constrain(batteryLevel, 0, 100);
  
  // Configura cores baseadas no nível
  uint16_t textColor, barColor;
    if (batteryLevel <= 20 && !lowBatteryWarningShown) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(GLITCH_RED);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 20);
    typeWriter("> CRITICAL POWER LEVEL!\n", GLITCH_RED);
    typeWriter("> SYSTEM SHUTDOWN IMMINENT\n", GLITCH_RED);
    
    for (int i = 0; i < 3; i++) {
      playHackerSound(300, 200);
      delay(300);
    }
    lowBatteryWarningShown = true;
    delay(1000);
    drawMainUI();
    return;

  } else if (batteryLevel <= 40) {
    textColor = WARNING_YELLOW;
    barColor = WARNING_YELLOW;
  } else {
    textColor = MATRIX_GREEN;
    barColor = MATRIX_GREEN;
  }
  
  // Exibe porcentagem
  M5.Lcd.setTextColor(textColor);
  M5.Lcd.setCursor(5, 60);
  M5.Lcd.printf("BAT: %d%%", batteryLevel);
  
  // Barra de bateria estilizada
  int barWidth = map(batteryLevel, 0, 100, 0, 50);
  M5.Lcd.drawRect(60, 60, 52, 8, TERMINAL_CYAN);
  M5.Lcd.fillRect(62, 62, barWidth, 4, barColor);
  
  // Ícone de bateria
  M5.Lcd.fillRect(115, 62, 3, 4, TERMINAL_CYAN);
}

void drawMainUI() {
  M5.Lcd.fillScreen(TFT_BLACK);
  drawTerminalHeader();
  
  // Nome principal
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(NEON_PURPLE);
  M5.Lcd.setCursor(5, 25);
  M5.Lcd.println("bl4k.code");
  
  // Status do sistema
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(MATRIX_GREEN);
  M5.Lcd.setCursor(5, 45);
  M5.Lcd.printf("System: %s", overclockMode ? "OVERCLOCKED" : "STEALTH");
  
  // Informações de hardware
  drawBatteryStatus();
  
  // Barra de status
  M5.Lcd.fillRect(0, M5.Lcd.height() - 20, M5.Lcd.width(), 2, TERMINAL_CYAN);
  
  // Relógio e assinatura
  M5.Lcd.setTextColor(TERMINAL_CYAN);
  M5.Lcd.setCursor(5, M5.Lcd.height() - 15);
  M5.Lcd.printf("%02d:%02d", 
    M5.Rtc.getTime().hours, 
    M5.Rtc.getTime().minutes);
  
  M5.Lcd.setTextColor(NEON_PURPLE);
  M5.Lcd.setCursor(M5.Lcd.width() - 60, M5.Lcd.height() - 15);
  M5.Lcd.print("0xFA7E3C");
}

void setup() {
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1);
  
  playHackerSound();
  bootAnimation();
  drawMainUI();
}

void loop() {
  M5.update();
  
  // Efeitos aleatórios
  if (millis() - lastGlitch > 20000 && random(100) > 90) {
    glitchEffect();
    drawMainUI();
    lastGlitch = millis();
  }
  
  // Controles
  if (M5.BtnA.wasPressed()) {
    glitchEffect();
    drawMainUI();
  }
  
  if (M5.BtnB.wasPressed()) {
    checkNetwatchEasterEgg();
  }
  
  if (M5.BtnB.pressedFor(800)) {
    overclockMode = !overclockMode;
    M5.Lcd.setBrightness(overclockMode ? 100 : 50);
    drawMainUI();
  }
  
  // Atualização dinâmica
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate > 500) {
    // Cursor piscante
    M5.Lcd.setCursor(M5.Lcd.width() - 10, 0);
    drawCyberCursor();
    
    // Atualiza a cada segundo
    if (millis() - lastUpdate > 1000) {
      drawMainUI();
    }
    
    lastUpdate = millis();
  }
}