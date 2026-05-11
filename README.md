# PG Automation — Smart Room (ESP32 + Blynk)

Home automation firmware for an ESP32 that controls a **fan** and **light** via relays, reads **temperature and humidity** (DHT11), supports **scheduled fan** operation using **NTP** (IST), optional **IR-based people counting** for auto lights/fan, and is controlled through the **Blynk** app.

**Author / reference:** [github.com/smalakargh](https://github.com/smalakargh)

---

## Features

- **Manual control** — Fan and light from Blynk (active-low relay drive).
- **Fan schedule** — ON/OFF times via Blynk; checked every 15 s against local time from NTP (**IST**, UTC+5:30).
- **Fan runtime tracking** — Accumulated run time shown on Blynk (minutes while running; display updates).
- **DHT11** — Temperature and humidity pushed to Blynk every 2 s.
- **Dual IR sensors** — Entry/exit direction logic updates an **inside people count**; optional **auto mode** turns fan + light on when count &gt; 0 and off when empty.
- **Blynk sync** — Virtual pins synced on connect for consistent UI state.

---

## Hardware overview

| Component        | Notes |
|-----------------|--------|
| ESP32           | Wi‑Fi, Blynk client, GPIO |
| 2× relay module | Fan (GPIO 26), Light (GPIO 27), **active LOW** when ON |
| DHT11           | Data pin **GPIO 14** |
| 2× IR sensors   | **GPIO 5** (outside), **GPIO 18** (inside); LOW = triggered |

---

## Project gallery

<div style="display:flex; overflow-x:auto; gap:10px;">

  <img src="https://raw.githubusercontent.com/arijitbaurigh/PGAutomation/refs/heads/main/src/constants/FullProject.jpeg" alt="Full project" width="250"/>

  <img src="https://raw.githubusercontent.com/arijitbaurigh/PGAutomation/refs/heads/main/src/constants/ESP32andReley.jpeg" alt="ESP32 and relay" width="250"/>

  <img src="https://raw.githubusercontent.com/arijitbaurigh/PGAutomation/refs/heads/main/src/constants/DHT11.jpeg" alt="DHT11" width="250"/>

  <img src="https://raw.githubusercontent.com/arijitbaurigh/PGAutomation/refs/heads/main/src/constants/IRsensor.jpeg" alt="IR sensors" width="250"/>

  <img src="https://raw.githubusercontent.com/arijitbaurigh/PGAutomation/refs/heads/main/src/constants/MatrixDisplay.jpeg" alt="Matrix display" width="250"/>

</div>


---

## Blynk virtual pins (as in firmware)

| Pin | Purpose |
|-----|--------|
| V1  | Fan (manual / state sync) |
| V2  | Light |
| V3  | Fan timer enable |
| V4  | Fan ON time (seconds since midnight) |
| V5  | Fan OFF time |
| V6  | Timer status text |
| V7  | Fan total runtime display |
| V8  | Temperature (°C) |
| V9  | Humidity (%) |
| V10 | People count (inside) |
| V11 | Auto mode (IR-driven fan + light) |

Create a Blynk template matching **`Advance room 112`** (or update `BLYNK_TEMPLATE_*` in code) and wire these widgets to the same virtual pins.

---

## Software requirements

- [Arduino IDE](https://www.arduino.cc/en/software) or PlatformIO
- Libraries:
  - **WiFi** (ESP32 core)
  - **Blynk** (`BlynkSimpleEsp32`)
  - **DHT sensor library** (`DHT.h` by Adafruit-style DHT lib)
  - ESP32 board support package

Install the **ESP32** board package and add the **Blynk** library from the Library Manager (version compatible with your Blynk cloud plan).

---

## Configuration

1. Open `main.ino`.
2. Set **`BLYNK_TEMPLATE_ID`**, **`BLYNK_TEMPLATE_NAME`**, and **`BLYNK_AUTH_TOKEN`** from your Blynk project.
3. Set **`ssid`** and **`password`** for your Wi‑Fi network.
4. Upload to your ESP32 board (select the correct board and COM port).

**Security:** Do not commit real tokens or Wi‑Fi passwords to a public repository. Use placeholders locally or environment-specific config if you extend the project.

---

## Build and upload

1. Connect the ESP32 via USB.
2. In Arduino IDE: **Tools → Board →** your ESP32 variant; **Tools → Port →** the correct serial port.
3. **Verify** then **Upload**.

Open the Serial Monitor at **115200** baud to see connection, IR events, DHT reads, and timer logs.

---
## 🛠 Installation

1. Clone the repository
```sh
git clone https://github.com/arijitbaurigh/PGAutomation.git
```

2. Move into the project folder
```sh
cd PGAutomation
```