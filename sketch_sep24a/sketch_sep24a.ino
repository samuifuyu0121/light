// ====== 腳位設定（LOW=亮, HIGH=滅）======
const int pushButton = 2;
const int RledPin   = 3;  // PWM
const int GledPin   = 5;  // PWM
const int BledPin   = 4;  // 非PWM，用軟體PWM實現呼吸

// ====== 模式設定 ======
enum Mode { MODE_CYCLE = 0, MODE_BLINK_YELLOW = 1, MODE_BLINK_GREEN = 2 }; // ✅ 第二個閃爍改成綠
Mode mode = MODE_CYCLE;

// ====== 時序參數 ======
const unsigned long cycleInterval    = 2000;
const unsigned long blinkInterval    = 500;
const unsigned long debounceMs       = 30;
const unsigned long longPressMs      = 600;
const unsigned long ultraLongPressMs = 1500;
const unsigned long breathStepMs     = 1;

// ====== 軟體PWM（給 pin 4 用）=====
const unsigned long SWPWM_PERIOD_US = 1020;
unsigned long swpwmStartUs = 0;

// ====== 狀態變數 ======
int colorState = 0;  // 模式1用：0=綠,1=青,2=黃
unsigned long lastChangeTime = 0;
unsigned long lastButtonChange = 0;
bool buttonStableState = HIGH;
unsigned long pressStartTime = 0;

// ====== 呼吸狀態 ======
bool breathingActive   = false;
bool breatheGreenPhase = true; // true=綠呼吸；false=藍呼吸
int  breathValue       = 0;
int  breathDir         = +1;
unsigned long lastBreathStep = 0;

// ====== 小工具（LOW=亮 / HIGH=滅）======
void allOff() {
  digitalWrite(RledPin, HIGH);
  digitalWrite(GledPin, HIGH);
  digitalWrite(BledPin, HIGH);
}

void showGreen() {
  digitalWrite(RledPin, HIGH);
  digitalWrite(GledPin, LOW);
  digitalWrite(BledPin, HIGH);
}

void showCyan() { // 綠+藍
  digitalWrite(RledPin, HIGH);
  digitalWrite(GledPin, LOW);
  digitalWrite(BledPin, LOW);
}

void showYellow() { // 紅+綠
  digitalWrite(RledPin, LOW);
  digitalWrite(GledPin, LOW);
  digitalWrite(BledPin, HIGH);
}

// ====== PWM 控制（藍色軟PWM）======
inline void pwmGreen(uint8_t level) {
  analogWrite(GledPin, 255 - level);
}

inline void swpwmBlue(uint8_t level) {
  unsigned long phase = (micros() - swpwmStartUs) % SWPWM_PERIOD_US;
  unsigned long onUs  = (unsigned long)level * SWPWM_PERIOD_US / 255;
  if (phase < onUs)  digitalWrite(BledPin, LOW);
  else               digitalWrite(BledPin, HIGH);
}

// ====== 呼吸控制（綠/藍交替、暗→亮→暗）======
void updateBreathing(unsigned long now) {
  if (now - lastBreathStep >= breathStepMs) {
    lastBreathStep = now;
    breathValue += breathDir;
    if (breathValue >= 255) { breathValue = 255; breathDir = -1; }
    if (breathValue <= 0)   { breathValue = 0;   breathDir = +1; breatheGreenPhase = !breatheGreenPhase; }
  }

  digitalWrite(RledPin, HIGH); // R關閉
  if (breatheGreenPhase) {
    pwmGreen((uint8_t)breathValue);
    digitalWrite(BledPin, HIGH);
  } else {
    digitalWrite(GledPin, HIGH);
    swpwmBlue((uint8_t)breathValue);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  swpwmStartUs = micros();

  colorState = 0;
  lastChangeTime = millis();
  showGreen(); // 初始綠
}

void loop() {
  unsigned long now = millis();

  // ===== 按鈕去彈跳 =====
  int raw = digitalRead(pushButton);
  if (raw != buttonStableState && (now - lastButtonChange) >= debounceMs) {
    buttonStableState = raw;
    lastButtonChange  = now;

    if (buttonStableState == LOW) {
      pressStartTime   = now;
      breathingActive  = false;
      breatheGreenPhase = true;
      breathValue      = 0;
      breathDir        = +1;
      lastBreathStep   = now;
      allOff();
    } else {
      unsigned long pressDuration = now - pressStartTime;

      if (pressDuration >= ultraLongPressMs) {
        breathingActive = false;
        if (mode == MODE_CYCLE) { colorState = 0; lastChangeTime = now; showGreen(); }
      } else if (pressDuration >= longPressMs) {
        if (mode == MODE_CYCLE) { colorState = 0; lastChangeTime = now; showGreen(); }
      } else if (pressDuration >= debounceMs) {
        // ✅ 短按切換三模式
        mode = static_cast<Mode>((static_cast<int>(mode) + 1) % 3);
        lastChangeTime = now;
        switch (mode) {
          case MODE_CYCLE:        colorState = 0; showGreen(); break;
          case MODE_BLINK_YELLOW: showYellow();  break;  // 黃色閃爍
          case MODE_BLINK_GREEN:  showGreen();   break;  // 綠色閃爍
        }
      }
    }
  }

  // ===== 長按期間：>1.5s → 呼吸模式 =====
  if (buttonStableState == LOW) {
    unsigned long held = now - pressStartTime;
    if (held >= ultraLongPressMs) {
      breathingActive = true;
      updateBreathing(now);
    } else {
      breathingActive = false;
      allOff();
    }
    return;
  }

  // ===== 放開後維持模式 =====
  breathingActive = false;
  switch (mode) {
    case MODE_CYCLE:
      if (now - lastChangeTime >= cycleInterval) {
        colorState = (colorState + 1) % 3; // 綠→青→黃→綠
        lastChangeTime = now;
      }
      if (colorState == 0)      showGreen();
      else if (colorState == 1) showCyan();
      else                      showYellow();
      break;

    case MODE_BLINK_YELLOW: { // 第一個閃爍：黃色
      static bool onY = true;
      if (now - lastChangeTime >= blinkInterval) {
        lastChangeTime = now;
        onY = !onY;
        if (onY) showYellow(); else allOff();
      }
      break;
    }

    case MODE_BLINK_GREEN: { // 第二個閃爍：綠色
      static bool onG = true;
      if (now - lastChangeTime >= blinkInterval) {
        lastChangeTime = now;
        onG = !onG;
        if (onG) showGreen(); else allOff();
      }
      break;
    }
  }
}
