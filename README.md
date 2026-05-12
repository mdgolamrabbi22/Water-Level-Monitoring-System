# 💧 Water Level Monitoring System
### Arduino-based 3-Stage Ultrasonic Alert System

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=flat-square&logo=arduino)
![Language](https://img.shields.io/badge/Language-Arduino_C-00979D?style=flat-square)
![Sensor](https://img.shields.io/badge/Sensor-HC--SR04-blue?style=flat-square)
![Status](https://img.shields.io/badge/Status-Working-success?style=flat-square)

---

## 📌 প্রজেক্ট সম্পর্কে

Arduino Uno দিয়ে তৈরি একটি **IoT সেন্সর সিস্টেম** যা পানির উচ্চতা (বা যেকোনো বস্তুর দূরত্ব) পরিমাপ করে এবং ৩টি বিপদ-স্তর অনুযায়ী LED ও Buzzer অ্যাকটিভ করে। Ultrasonic sensor থেকে **5টি reading-এর median** নিয়ে accurate ও noise-free measurement নিশ্চিত করা হয়েছে।

---

## ✨ মূল ফিচারসমূহ

### ⚠️ 3-Stage Alert System
| Stage | দূরত্ব | LED | Buzzer | মানে |
|-------|--------|-----|--------|------|
| Stage 1 | ≤ 8 cm | LED1 (Slow blink 300ms) | 15 সেকেন্ড | সতর্কতা |
| Stage 2 | ≤ 4 cm | LED2 (Fast blink 200ms) | 15 সেকেন্ড | বিপদজনক |
| Stage 3 | ≤ 2 cm | LED3 (Very fast 100ms) | 30 সেকেন্ড | জরুরি অবস্থা |
| Safe | > 8 cm | সব OFF | OFF | স্বাভাবিক |

### 🔬 Technical Features
- **Median-of-5 Noise Filtering** — 5টি reading নিয়ে মাঝের মানটা ব্যবহার করা হয়, ফলে noise/interference থেকে accurate reading পাওয়া যায়
- **Non-blocking `millis()` Logic** — `delay()` ব্যবহার না করে `millis()` দিয়ে timer handle করা হয়েছে, ফলে system সবসময় responsive থাকে
- **Stage-wise Buzzer Duration** — Stage 1 ও 2 তে 15 সেকেন্ড, Stage 3 তে 30 সেকেন্ড buzzer active থাকে
- **Independent LED Blink Patterns** — প্রতিটা stage-এ আলাদা speed-এ LED blink করে
- **Serial Monitor Debug** — Real-time distance ও stage change Serial Monitor-এ দেখা যায়

---

## 🔌 Circuit Connection

```
HC-SR04 Ultrasonic Sensor
━━━━━━━━━━━━━━━━━━━━━━━━━
VCC  →  5V  (Arduino)
GND  →  GND
TRIG →  D9
ECHO →  D10

LEDs (প্রতিটায় 220Ω resistor সিরিজে)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
LED1 (হলুদ/সবুজ)  →  D4 → 220Ω → GND
LED2 (কমলা)        →  D5 → 220Ω → GND
LED3 (লাল)         →  D6 → 220Ω → GND

Buzzer (Active Buzzer)
━━━━━━━━━━━━━━━━━━━━━━
Buzzer (+) →  D7
Buzzer (-) →  GND

Power
━━━━━
Arduino powered via USB
```

---

## 📁 প্রজেক্ট স্ট্রাকচার

```
Water Level Monitoring System/
├── Water_Level/
│   └── Water_Level.ino     # মূল Arduino sketch
└── Circuit.txt             # Circuit connection গাইড
```

---

## ⚙️ কীভাবে রান করবে

### Requirements
- Arduino Uno (বা compatible board)
- HC-SR04 Ultrasonic Sensor
- 3টি LED (বিভিন্ন রঙ)
- 3টি 220Ω Resistor
- 1টি Active Buzzer
- Jumper Wires + Breadboard
- Arduino IDE (2.x recommended)

### ধাপ ১ — Circuit সংযুক্ত করো
উপরের Circuit Connection অনুযায়ী সব component সংযুক্ত করো।

### ধাপ ২ — Code আপলোড করো
```
1. Arduino IDE খোলো
2. Water_Level.ino ফাইলটি open করো
3. Board: "Arduino Uno" সিলেক্ট করো
4. Port সঠিক সিলেক্ট করো
5. Upload বাটন চাপো (→)
```

### ধাপ ৩ — Serial Monitor চালু করো
```
Tools → Serial Monitor → Baud Rate: 9600
```
দেখতে পাবে:
```
Distance cm: 12
Distance cm: 5
Stage changed to: 2
Distance cm: 3
Stage changed to: 3
```

---

## 🧠 কোডের গুরুত্বপূর্ণ অংশ

### Median Filter (Noise Reduction)
```cpp
long getMedianDistance() {
    const int N = 5;
    long arr[N];
    for (int i = 0; i < N; ++i) {
        arr[i] = readSingleDistance();
        delay(20);
    }
    // Bubble sort করে মাঝের value নেওয়া হয়
    // ... (sort code)
    return arr[N/2];  // Median return
}
```

### Non-blocking millis() Timer
```cpp
// delay() ব্যবহার না করে:
if (now - lastLedToggle >= ledInterval) {
    ledState = !ledState;
    digitalWrite(activeLedPin, ledState ? HIGH : LOW);
    lastLedToggle = now;
}
```

---

## 🛠️ Technologies & Components

| Item | Spec |
|------|------|
| Microcontroller | Arduino Uno (ATmega328P) |
| Sensor | HC-SR04 Ultrasonic (range: 2cm–400cm) |
| Language | Arduino C/C++ |
| Alert | 3x LED + Active Buzzer |
| Power | USB (5V) |

---

## 🔮 ভবিষ্যৎ উন্নয়নের সুযোগ

- LCD display যোগ করা (I2C 16x2)
- WiFi module (ESP8266) দিয়ে mobile notification
- Data logging (SD card)
- Web dashboard integration

----

## 👨‍💻 Developer

**Name:** Md Golam Rabbi
**Institution:** Barisal Polytechnic Institute  
**Project Type:** IoT / Hardware Project  
**Domain:** Embedded Systems / Arduino Programming
