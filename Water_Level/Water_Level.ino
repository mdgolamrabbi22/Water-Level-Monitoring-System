// Ultrasonic 3-Stage Alert (custom blink & buzzer durations)
// Pins:
// TRIG -> 9, ECHO -> 10
// LED1 -> 4 (<=8cm), LED2 -> 5 (<=4cm), LED3 -> 6 (<=2cm)
// BUZZER -> 7

const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int LED1_PIN = 4;
const int LED2_PIN = 5;
const int LED3_PIN = 6;
const int BUZZER_PIN = 7;

// Thresholds (cm)
const float TH_STAGE1 = 8.0;
const float TH_STAGE2 = 4.0;
const float TH_STAGE3 = 2.0;

// TOTAL buzzer active durations (ms) as requested
const unsigned long BUZZ_TOTAL_STAGE1 = 15000UL; // 15 s
const unsigned long BUZZ_TOTAL_STAGE2 = 15000UL; // 15 s
const unsigned long BUZZ_TOTAL_STAGE3 = 30000UL; // 30 s

// LED blink intervals (ms)
const unsigned long LED_INT_STAGE1 = 300UL; // 1.0 s
const unsigned long LED_INT_STAGE2 = 200UL;  // 0.5 s
const unsigned long LED_INT_STAGE3 = 100UL;  // 0.3 s

// Buzzer beep-on time within each small period (ms)
const unsigned long BUZZ_ON_MS = 200UL; // buzzer ON time per beep (keeps short)

// Buzzer periods (how often a beep happens) per stage
const unsigned long BUZZ_PERIOD_STAGE1 = 500UL; // 1.0s period -> 1 beep/sec
const unsigned long BUZZ_PERIOD_STAGE2 = 400UL;  // 0.5s period -> 2 beep/sec
const unsigned long BUZZ_PERIOD_STAGE3 = 300UL;  // 0.3s period

// ---- internal state ----
int currentStage = 0; // 0 = none, 1,2,3
unsigned long stageStart = 0;
unsigned long buzzerEndTime = 0;

bool ledState = false;
unsigned long lastLedToggle = 0;

bool buzzState = false;
unsigned long lastBuzzToggle = 0;

// ---------- setup ----------
void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // ensure off
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

// ---------- ultrasonic reading (median of 5) ----------
long readSingleDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL); // timeout 30ms
  if (dur == 0UL) return 9999;
  long dist = (long)(dur * 0.034 / 2.0);
  return dist;
}

long getMedianDistance() {
  const int N = 5;
  long arr[N];
  for (int i = 0; i < N; ++i) {
    arr[i] = readSingleDistance();
    delay(20);
  }
  // sort small array
  for (int i = 0; i < N-1; ++i) {
    for (int j = i+1; j < N; ++j) {
      if (arr[j] < arr[i]) {
        long t = arr[i]; arr[i] = arr[j]; arr[j] = t;
      }
    }
  }
  return arr[N/2];
}

void allOff() {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  ledState = false;
  buzzState = false;
}

void loop() {
  long distance = getMedianDistance();
  Serial.print("Distance cm: ");
  Serial.println(distance);

  int newStage = 0;
  if (distance <= (long)TH_STAGE3) newStage = 3;
  else if (distance <= (long)TH_STAGE2) newStage = 2;
  else if (distance <= (long)TH_STAGE1) newStage = 1;
  else newStage = 0;

  unsigned long now = millis();

  if (newStage != currentStage) {
    currentStage = newStage;
    stageStart = now;
    lastLedToggle = now;
    lastBuzzToggle = now;
    ledState = false;
    buzzState = false;

    if (currentStage == 1) buzzerEndTime = now + BUZZ_TOTAL_STAGE1;
    else if (currentStage == 2) buzzerEndTime = now + BUZZ_TOTAL_STAGE2;
    else if (currentStage == 3) buzzerEndTime = now + BUZZ_TOTAL_STAGE3;
    else buzzerEndTime = 0;

    allOff();
    Serial.print("Stage changed to: ");
    Serial.println(currentStage);
  }

  // if nothing detected, switch everything off
  if (currentStage == 0) {
    allOff();
    delay(60);
    return;
  }

  // choose active LED & blink interval & buzzer period
  int activeLedPin = LED1_PIN;
  unsigned long ledInterval = LED_INT_STAGE1;
  unsigned long buzzPeriod = BUZZ_PERIOD_STAGE1;

  if (currentStage == 1) {
    activeLedPin = LED1_PIN;
    ledInterval = LED_INT_STAGE1;
    buzzPeriod = BUZZ_PERIOD_STAGE1;
  } else if (currentStage == 2) {
    activeLedPin = LED2_PIN;
    ledInterval = LED_INT_STAGE2;
    buzzPeriod = BUZZ_PERIOD_STAGE2;
  } else { // stage 3
    activeLedPin = LED3_PIN;
    ledInterval = LED_INT_STAGE3;
    buzzPeriod = BUZZ_PERIOD_STAGE3;
  }

  // if buzzer still in its active time window -> perform patterned buzzer + LED blinking
  if (now <= buzzerEndTime) {
    // LED blinking (toggle every ledInterval)
    if (now - lastLedToggle >= ledInterval) {
      ledState = !ledState;
      digitalWrite(activeLedPin, ledState ? HIGH : LOW);
      lastLedToggle = now;
    }

    // buzzer patterned beeps (for stage1 & stage2) or near-continuous pulses (stage3)
    if (currentStage == 1 || currentStage == 2) {
      if (now - lastBuzzToggle >= buzzPeriod) {
        // start beep
        digitalWrite(BUZZER_PIN, HIGH);
        buzzState = true;
        lastBuzzToggle = now; // mark start
      } else if (buzzState && (now - lastBuzzToggle >= BUZZ_ON_MS)) {
        digitalWrite(BUZZER_PIN, LOW);
        buzzState = false;
        // leave lastBuzzToggle as beep start time so next period checks correctly
      }
    } else { // stage 3
      // for stage3 we also do quick on/off per buzzPeriod
      if (now - lastBuzzToggle >= buzzPeriod) {
        digitalWrite(BUZZER_PIN, HIGH);
        buzzState = true;
        lastBuzzToggle = now;
      } else if (buzzState && (now - lastBuzzToggle >= BUZZ_ON_MS)) {
        digitalWrite(BUZZER_PIN, LOW);
        buzzState = false;
      }
    }
  }
  else {
    // buzzer window ended -> ensure buzzer OFF; LED stays blinking while object present
    digitalWrite(BUZZER_PIN, LOW);
    buzzState = false;
    // LED: keep toggling (we let the LED continue blinking - because you asked LED to keep blinking)
    if (now - lastLedToggle >= ledInterval) {
      ledState = !ledState;
      digitalWrite(activeLedPin, ledState ? HIGH : LOW);
      lastLedToggle = now;
    }
  }

  // ensure other LEDs are off
  if (activeLedPin != LED1_PIN) digitalWrite(LED1_PIN, LOW);
  if (activeLedPin != LED2_PIN) digitalWrite(LED2_PIN, LOW);
  if (activeLedPin != LED3_PIN) digitalWrite(LED3_PIN, LOW);

  delay(10); // small delay to keep loop stable
}