# Smart Bin IoT Monitoring System

## Overview

Smart Bin เป็นระบบตรวจวัดระดับขยะอัจฉริยะโดยใช้ ESP32-C3 และ Ultrasonic Sensor HC-SR04 สำหรับวัดระดับขยะภายในถัง พร้อมส่งข้อมูลแบบ Real-time ไปยัง Discord และ Google Sheets เพื่อใช้ในการติดตามและวิเคราะห์ปริมาณขยะ

---

## Features

* ตรวจวัดระดับขยะด้วย HC-SR04
* คำนวณเปอร์เซ็นต์ความเต็มของถัง
* คำนวณ Time To Full (TTF)
* แจ้งเตือนผ่าน Discord Webhook
* บันทึกข้อมูลลง Google Sheets
* ส่งกราฟ Analytics อัตโนมัติ
* ระบบ Anti-Spam
* Progress Bar แสดงระดับขยะ

---

## Hardware Used

* ESP32-C3 DevKitM-1
* HC-SR04 Ultrasonic Sensor
* Resistor 2kΩ
* Jumper Wire

---

## Wiring Diagram

| HC-SR04 | ESP32-C3                    |
| ------- | --------------------------- |
| VCC     | 5V                          |
| GND     | GND                         |
| TRIG    | GPIO 5                      |
| ECHO    | GPIO 18 (ผ่าน Resistor 2kΩ) |

> หมายเหตุ: ใช้ตัวต้านทานเพื่อลดแรงดันจากขา ECHO ของ HC-SR04 ก่อนเข้าสู่ ESP32-C3

---

## System Workflow

1. HC-SR04 ตรวจวัดระยะห่างระหว่างเซนเซอร์กับขยะ
2. ESP32-C3 คำนวณระดับความเต็มของถัง (%)
3. ระบบคำนวณ Rate และ Time To Full
4. ส่งข้อมูลไปยัง Discord Dashboard
5. บันทึกข้อมูลลง Google Sheets
6. สร้างกราฟ Analytics และส่งเข้า Discord

---

## Installation

### 1. Clone Repository

```bash
git clone https://github.com/Dorm6159/SmartBin.git
```

---

### 2. Open Project in VS Code

ติดตั้ง Extensions:

* PlatformIO IDE
* Wokwi Simulator

---

### 3. Build Project

```bash
pio run
```

---

### 4. Run Simulation

```bash
Wokwi: Start Simulator
```

---

## Google Apps Script Setup

### 1. Create Google Sheets

สร้าง Google Sheets ใหม่ แล้วตั้งชื่อคอลัมน์ดังนี้:

| Time | Level | Distance | Rate | TTF |
| ---- | ----- | -------- | ---- | --- |

---

### 2. Open Apps Script

ไปที่:

```text
Extensions → Apps Script
```

ลบโค้ดเดิมทั้งหมด แล้วใส่โค้ดนี้:

```javascript
function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Sheet1");

  var level = e.parameter.level;
  var distance = e.parameter.distance;
  var rate = e.parameter.rate;
  var ttf = e.parameter.ttf;

  sheet.appendRow([
    new Date(),
    level,
    distance,
    rate,
    ttf
  ]);

  return ContentService.createTextOutput("OK");
}
```

---

### 3. Deploy Web App

1. กด `Deploy`
2. เลือก `New Deployment`
3. เลือก `Web App`
4. ตั้งค่า:

```text
Execute as: Me
Who has access: Anyone
```

5. กด Deploy
6. Copy URL ที่ได้

ตัวอย่าง:

```text
https://script.google.com/macros/s/xxxxxxxxxxxxxxxx/exec
```

นำไปใส่ใน:

```cpp
String sheetURL = "YOUR_SCRIPT_URL";
```

---

## Discord Webhook Setup

### 1. Create Webhook

1. เปิด Discord Server
2. ไปที่:

```text
Server Settings → Integrations → Webhooks
```

3. กด `New Webhook`
4. Copy Webhook URL

ตัวอย่าง:

```text
https://discord.com/api/webhooks/xxxxxxxxx
```

นำไปใส่ใน:

```cpp
String webhook = "YOUR_WEBHOOK_URL";
String webhookgraph = "YOUR_GRAPH_WEBHOOK_URL";
```

---

## Google Sheets Chart Setup

### 1. Create Chart

1. เลือกข้อมูลใน Google Sheets
2. กด:

```text
Insert → Chart
```

3. เลือกประเภทกราฟตามต้องการ

---

### 2. Publish Chart

1. กดจุดสามจุดที่ Chart
2. เลือก `Publish Chart`
3. เลือก `Embed`
4. Copy URL รูปภาพ

ตัวอย่าง:

```text
https://docs.google.com/spreadsheets/d/e/xxxxxxxx/pubchart?oid=xxxx&format=image
```

นำไปใส่ใน:

```cpp
String chartURL = "YOUR_CHART_URL";
```

---

## Configuration

แก้ไข URL ต่าง ๆ ภายใน `main.cpp`

```cpp
String webhook = "";
String webhookgraph = "";
String sheetURL = "";
String chartURL = "";
```

---

## Formula

### Fill Level

```text
Level = ((BinHeight - Distance) / BinHeight) × 100
```

### Time To Full

```text
TTF = (100 - Level) / Rate
```

---

## Example Discord Dashboard

* 📊 Level
* ⏳ Time To Full
* 🧱 Progress Bar
* 📌 Status Alert

---

## Project Structure

```text
SmartBin/
│
├── src/
│   └── main.cpp
│
├── diagram.json
├── platformio.ini
├── wokwi.toml
├── README.md
└── .gitignore
```

