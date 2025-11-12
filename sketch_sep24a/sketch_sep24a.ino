// ==== 腳位設定 ====
const int buttonPin = 2;  // 按鈕 (INPUT_PULLUP：放開=HIGH，按下=LOW)
const int GledPin   = 5;  // 綠燈
const int BledPin   = 4;  // 藍燈

// ==== 模式 ====
int mode = 0;  // 0=恆亮, 1=慢閃, 2=中閃, 3=快閃

// ==== 去彈跳 ====
int buttonState = HIGH;        
int lastReading = HIGH;        
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 去彈跳延遲

// ==== 閃爍控制 ====
unsigned long previousMillis = 0;
bool ledOn = true;  // G+B 是否點亮 (LOW=亮, HIGH=滅)

// 閃爍速度（毫秒）
const unsigned long slowBlink = 1000;
const unsigned long midBlink  = 500;
const unsigned long fastBlink = 200;

// ==== 小工具：控制 G+B ====
void GBwrite(bool on) {
  // ⚙️ LOW=亮、HIGH=滅
  if (on) {
    digitalWrite(GledPin, LOW);
    digitalWrite(BledPin, LOW);
  } else {
    digitalWrite(GledPin, HIGH);
    digitalWrite(BledPin, HIGH);
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);

  // ✅ 初始為 G+B 恆亮（青色）
  GBwrite(true);
  previousMillis = millis();
}

void loop() {
  // ---- 去彈跳讀按鈕 ----
  int reading = digitalRead(buttonPin);
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        mode = (mode + 1) % 4;     // 循環 0→1→2→3→0
        previousMillis = millis(); // 切模式時重設時間
        ledOn = true;              // 新模式從亮開始
        GBwrite(true);
      }
    }
  }
  lastReading = reading;

  // ---- 根據模式控制 G+B ----
  unsigned long now = millis();

  switch (mode) {
    case 0: // 恆亮
      GBwrite(true);
      break;

    case 1: // 慢閃
      if (now - previousMillis >= slowBlink) {
        previousMillis = now;
        ledOn = !ledOn;
        GBwrite(ledOn);
      }
      break;

    case 2: // 中閃
      if (now - previousMillis >= midBlink) {
        previousMillis = now;
        ledOn = !ledOn;
        GBwrite(ledOn);
      }
      break;

    case 3: // 快閃
      if (now - previousMillis >= fastBlink) {
        previousMillis = now;
        ledOn = !ledOn;
        GBwrite(ledOn);
      }
      break;
  }
}


