// ========================================
// AIRSOFT BOMB - CON DEV MODE
// Ottimizzato per Arduino Uno
// ========================================

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ======= LCD I2C =======
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======= KEYPAD 4x4 =======
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ======= PIN =======
#define LED_RED 10
#define LED_YELLOW 11
#define LED_GREEN 12
#define BUZZER_PIN 13

// ======= BOMBA =======
const char armPassword[] = "000000";
const char devPassword[] = "123456";
char disarmPassword[10] = "123456";
char inputPassword[10] = "";
int passwordIndex = 0;

unsigned long bombTimer = 300;
unsigned long bombStartTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastBeepTime = 0;
unsigned long lastLedBlinkTime = 0;
unsigned long keyAPressStart = 0;
bool keyAPressed = false;

char tempInput[10] = "";
int tempIndex = 0;

// ======= STATI =======
enum BombState {
  S_IDLE, S_ARMED, S_DEFUSING, S_EXPLODED, S_DEFUSED,
  S_DEV_LOGIN, S_DEV_MENU, S_DEV_TIMER, S_DEV_CODE
};
BombState currentState = S_IDLE;

// ======= FUNZIONI HELPER =======
void clearPassword() {
  passwordIndex = 0;
  inputPassword[0] = '\0';
}

void addToPassword(char c) {
  if (passwordIndex < 9) {
    inputPassword[passwordIndex++] = c;
    inputPassword[passwordIndex] = '\0';
  }
}

void clearTemp() {
  tempIndex = 0;
  tempInput[0] = '\0';
}

void addToTemp(char c) {
  if (tempIndex < 9) {
    tempInput[tempIndex++] = c;
    tempInput[tempIndex] = '\0';
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  Serial.println(F("AIRSOFT BOMB v2"));
  Serial.println(F("ARM:000000 DEV:123456"));
  Serial.println(F("Hold A 10s=DEV mode"));
  
  lcd.init();
  lcd.backlight();
  lcd.print(F("AIRSOFT BOMB"));
  lcd.setCursor(0, 1);
  lcd.print(F("v2.0 DEV MODE"));

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  clearPassword();
  
  delay(2000);
  displayMainMenu();
}

// ======= LOOP =======
void loop() {
  char key = keypad.getKey();
  
  // Long press A check using key event
  if (currentState == S_IDLE) {
    // Se premi A, inizia il conteggio
    if (key == 'A') {
      if (!keyAPressed) {
        keyAPressStart = millis();
        keyAPressed = true;
      }
    }
    
    // Se A è stato premuto, controlla se sono passati 5 secondi
    if (keyAPressed) {
      if (millis() - keyAPressStart >= 5000) {
        enterDevLogin();
        keyAPressed = false;
        return; // Esci dal loop per evitare doppia gestione
      }
    }
  }

  if (key) {
    // Reset conteggio se premi un altro tasto
    if (key != 'A') {
      keyAPressed = false;
    }
    handleKeyPress(key);
  }

  if (currentState == S_ARMED || currentState == S_DEFUSING) {
    updateBombTimer();
  }
}

// ======= DEV LOGIN =======
void enterDevLogin() {
  currentState = S_DEV_LOGIN;
  clearPassword();
  tone(BUZZER_PIN, 1500, 300);
  lcd.clear();
  lcd.print(F("== DEV MODE =="));
  lcd.setCursor(0, 1);
  lcd.print(F("Password:"));
}

// ======= DEV MENU =======
void enterDevMenu() {
  currentState = S_DEV_MENU;
  clearPassword();
  lcd.clear();
  lcd.print(F("B=Timer C=Code"));
  lcd.setCursor(0, 1);
  lcd.print(F("A=Exit"));
}

// ======= SET TIMER =======
void enterSetTimer() {
  currentState = S_DEV_TIMER;
  clearTemp();
  lcd.clear();
  lcd.print(F("Timer (sec):"));
  lcd.setCursor(0, 1);
  lcd.print(F("Now: "));
  lcd.print(bombTimer);
}

// ======= SET CODE =======
void enterSetCode() {
  currentState = S_DEV_CODE;
  clearTemp();
  lcd.clear();
  lcd.print(F("Disarm code:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Now: "));
  lcd.print(disarmPassword);
}

// ======= KEY HANDLER =======
void handleKeyPress(char key) {

  if (currentState == S_IDLE) {
    if (key == '#') {
      clearPassword();
      displayMainMenu();
    } 
    else if (key == '*') {
      if (strcmp(inputPassword, armPassword) == 0) {
        armBomb();
      } else {
        wrongPassword();
      }
    } 
    else if (key >= '0' && key <= '9') {
      addToPassword(key);
      displayPasswordInput();
    }
  }

  else if (currentState == S_DEV_LOGIN) {
    if (key == '#') {
      clearPassword();
      lcd.setCursor(9, 1);
      lcd.print(F("      "));
    }
    else if (key == '*') {
      if (strcmp(inputPassword, devPassword) == 0) {
        enterDevMenu();
      } else {
        tone(BUZZER_PIN, 400, 300);
        clearPassword();
        lcd.setCursor(9, 1);
        lcd.print(F("ERROR!"));
        delay(1000);
        lcd.setCursor(9, 1);
        lcd.print(F("      "));
      }
    }
    else if (key >= '0' && key <= '9') {
      addToPassword(key);
      lcd.setCursor(9, 1);
      for (int i = 0; i < passwordIndex; i++) lcd.print('*');
    }
    else if (key == 'A') {
      currentState = S_IDLE;
      clearPassword();
      displayMainMenu();
    }
  }

  else if (currentState == S_DEV_MENU) {
    if (key == 'A') {
      currentState = S_IDLE;
      displayMainMenu();
    }
    else if (key == 'B') enterSetTimer();
    else if (key == 'C') enterSetCode();
  }

  else if (currentState == S_DEV_TIMER) {
    if (key == 'B') {
      if (tempIndex > 0) {
        bombTimer = atol(tempInput);
        if (bombTimer < 10) bombTimer = 10;
        if (bombTimer > 3600) bombTimer = 3600;
        tone(BUZZER_PIN, 2000, 200);
        lcd.clear();
        lcd.print(F("Saved: "));
        lcd.print(bombTimer);
        lcd.print('s');
        delay(1500);
      }
      enterDevMenu();
    }
    else if (key == '#') {
      clearTemp();
      enterSetTimer();
    }
    else if (key >= '0' && key <= '9' && tempIndex < 5) {
      addToTemp(key);
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      lcd.setCursor(0, 1);
      lcd.print(tempInput);
    }
    else if (key == 'A') enterDevMenu();
  }

  else if (currentState == S_DEV_CODE) {
    if (key == 'C') {
      if (tempIndex >= 4) {
        strcpy(disarmPassword, tempInput);
        tone(BUZZER_PIN, 2000, 200);
        lcd.clear();
        lcd.print(F("Saved: "));
        lcd.print(disarmPassword);
        delay(1500);
      }
      enterDevMenu();
    }
    else if (key == '#') {
      clearTemp();
      enterSetCode();
    }
    else if (key >= '0' && key <= '9' && tempIndex < 6) {
      addToTemp(key);
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      lcd.setCursor(0, 1);
      lcd.print(tempInput);
    }
    else if (key == 'A') enterDevMenu();
  }

  else if (currentState == S_ARMED) {
    if (key == 'D') startDefusing();
  }

  else if (currentState == S_DEFUSING) {
    if (key == '#') {
      clearPassword();
      displayDefusingScreen();
    } 
    else if (key == '*') {
      if (strcmp(inputPassword, disarmPassword) == 0) {
        defuseBomb();
      } else {
        wrongPassword();
        currentState = S_DEFUSING;
        displayDefusingScreen();
      }
    } 
    else if (key >= '0' && key <= '9') {
      addToPassword(key);
      displayDefusingScreen();
    }
  }
}

// ======= ARM BOMB =======
void armBomb() {
  currentState = S_ARMED;
  bombStartTime = millis();
  clearPassword();

  lcd.clear();
  lcd.print(F("BOMBA ARMATA!"));
  
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
  }
  noTone(BUZZER_PIN);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  delay(1500);
}

// ======= TIMER UPDATE =======
void updateBombTimer() {
  unsigned long now = millis();
  unsigned long elapsed = (now - bombStartTime) / 1000;
  unsigned long remaining = (elapsed < bombTimer) ? (bombTimer - elapsed) : 0;

  // LED blink
  if (remaining > 0 && (currentState == S_ARMED || currentState == S_DEFUSING)) {
    if (remaining < 30) {
      if (now - lastLedBlinkTime >= 250) {
        digitalWrite(LED_RED, !digitalRead(LED_RED));
        lastLedBlinkTime = now;
      }
    } else if (remaining <= 90) {
      if (now - lastLedBlinkTime >= 500) {
        digitalWrite(LED_RED, !digitalRead(LED_RED));
        lastLedBlinkTime = now;
      }
    } else {
      digitalWrite(LED_RED, HIGH);
    }
  }

  // Display + beep
  if (now - lastDisplayUpdate >= 1000) {
    lastDisplayUpdate = now;
    
    if (currentState == S_ARMED) displayArmedScreen();
    
    if (remaining > 0) {
      if (remaining < 30) {
        if (now - lastBeepTime >= 500) {
          tone(BUZZER_PIN, 1200, 100);
          lastBeepTime = now;
        }
        digitalWrite(LED_YELLOW, HIGH);
      } else if (remaining <= 90) {
        if (now - lastBeepTime >= 1000) {
          tone(BUZZER_PIN, 1000, 100);
          lastBeepTime = now;
        }
      } else {
        if (now - lastBeepTime >= 2000) {
          tone(BUZZER_PIN, 800, 100);
          lastBeepTime = now;
        }
      }
    }
  }

  if (remaining == 0 && currentState != S_EXPLODED) {
    explodeBomb();
  }
}

// ======= EXPLODE =======
void explodeBomb() {
  currentState = S_EXPLODED;
  
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  lcd.clear();
  lcd.print(F("** EXPLOSION **"));
  lcd.setCursor(0, 1);
  lcd.print(F("GAME OVER!"));

  // 10 sec alarm
  unsigned long start = millis();
  while (millis() - start < 10000) {
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 2000, 100);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(100);
  }
  noTone(BUZZER_PIN);
  digitalWrite(LED_RED, HIGH);
}

// ======= START DEFUSING =======
void startDefusing() {
  currentState = S_DEFUSING;
  clearPassword();
  tone(BUZZER_PIN, 1500, 300);
  displayDefusingScreen();
}

// ======= DEFUSE =======
void defuseBomb() {
  currentState = S_DEFUSED;

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);

  tone(BUZZER_PIN, 2000, 150);
  delay(200);
  tone(BUZZER_PIN, 2400, 150);
  delay(200);
  tone(BUZZER_PIN, 2800, 300);
  delay(400);
  noTone(BUZZER_PIN);

  lcd.clear();
  lcd.print(F("DEFUSED!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Mission OK!"));

  delay(3000);
  resetSystem();
}

// ======= WRONG PASSWORD =======
void wrongPassword() {
  lcd.clear();
  lcd.print(F("WRONG CODE!"));
  tone(BUZZER_PIN, 400, 500);
  delay(2000);
  clearPassword();
  if (currentState == S_IDLE) displayMainMenu();
}

// ======= RESET =======
void resetSystem() {
  currentState = S_IDLE;
  clearPassword();
  bombStartTime = 0;
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  displayMainMenu();
}

// ======= DISPLAYS =======
void displayMainMenu() {
  lcd.clear();
  lcd.print(F("Arm: 000000"));
  lcd.setCursor(0, 1);
  lcd.print(F("_ _ _ _ _ _"));
}

void displayPasswordInput() {
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  for (int i = 0; i < passwordIndex; i++) lcd.print(F("* "));
}

void displayArmedScreen() {
  unsigned long elapsed = (millis() - bombStartTime) / 1000;
  unsigned long remaining = (elapsed < bombTimer) ? (bombTimer - elapsed) : 0;

  lcd.clear();
  lcd.print(F("!! ARMED !!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Time: "));
  if (remaining/60 < 10) lcd.print('0');
  lcd.print(remaining/60);
  lcd.print(':');
  if (remaining%60 < 10) lcd.print('0');
  lcd.print(remaining%60);
}

void displayDefusingScreen() {
  lcd.clear();
  lcd.print(F("DEFUSING..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Code: "));
  for (int i = 0; i < passwordIndex; i++) lcd.print('*');
}
