const int buttonPin = 2;
const int RledPin = 3;
const int GledPin = 5;
const int BledPin = 4;

int buttonState = 0;
int lastButtonState = HIGH;
int moodPoint = 10; // 初始中性 (綠)
unsigned long lastChangeTime = 0;

void setup() {
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);

  // 🌿 初始為綠色
  setMoodColor(moodPoint);
  Serial.println("Init: GREEN");
  lastChangeTime = millis();
}

void loop() {
  buttonState = digitalRead(buttonPin);

  // --- 每按一下加一分（往藍方向） ---
  if (lastButtonState == HIGH && buttonState == LOW) {
    if (moodPoint < 20) moodPoint++;
    Serial.print("Pressed +1 → ");
    Serial.println(moodPoint);
    lastChangeTime = millis();
  }
  lastButtonState = buttonState;

  // --- 每 2 秒沒按：扣一分（往紅方向） ---
  if (millis() - lastChangeTime >= 2000) {
    if (moodPoint > 0) {
      moodPoint--;
      Serial.print("Auto -1 → ");
      Serial.println(moodPoint);
      lastChangeTime = millis();
    }
  }

  // --- 根據分數顯示光譜顏色 ---
  setMoodColor(moodPoint);
}

// moodPoint: 0(紅) → 10(綠) → 20(藍)
void setMoodColor(int point) {
  int r, g, b;

  if (point <= 10) {
    // 紅 → 綠
    r = map(point, 0, 10, 255, 0);
    g = map(point, 0, 10, 0, 255);
    b = 0;
  } else {
    // 綠 → 藍
    r = 0;
    g = map(point, 10, 20, 255, 0);
    b = map(point, 10, 20, 0, 255);
  }

  // ✅ 共陰極：PWM 數值越小越亮（反轉）
  analogWrite(RledPin, 255 - r);
  analogWrite(GledPin, 255 - g);
  analogWrite(BledPin, 255 - b);

  Serial.print("Mood: ");
  Serial.print(point);
  Serial.print(" | RGB(");
  Serial.print(r); Serial.print(", ");
  Serial.print(g); Serial.print(", ");
  Serial.print(b); Serial.println(")");
}







