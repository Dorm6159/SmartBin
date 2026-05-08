#include <WiFi.h>
#include <HTTPClient.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

const char* ssid = "Wokwi-GUEST";
const char* password = "";

String webhook = "https://discord.com/api/webhooks/1499154537045426219/A7UNkuux-O5msoUCiP3tgEDeApIryEZtMcFfamJXxEKzRb6VrmvEJDjuYaPcoPx_Kunx";
String webhookgraph = "https://discord.com/api/webhooks/1499154539415081172/sOqBemie4ETkLxcOEK2oTVQ8inCQh48U7WB2W6bfZ4_LnHVIgd4XNPBosWSl2iX_ZNIm";
String sheetURL = "https://script.google.com/macros/s/AKfycbwNz0TUAsBACHxNAu8xbdHr8RDlOHSNq-Tz6FtOP08PbF-BL1wF_Zwrjxqsy9kduXNVAA/exec";
String chartURL = "https://docs.google.com/spreadsheets/d/e/2PACX-1vQU6CjB91o7QmL6aQjAvD74qa7fMSPpeKOGRDZwVsXLSdlubw-4hmzBVMfR7IIWlNP7B1aNxy4l8ccV/pubchart?oid=316735742&format=image";

float binHeight = 50.0;

float prevLevel = 0;
unsigned long prevTime = 0;


float rate = 0;
float timeToFull = 0;

float lastRate = 0;
float lastTTF = 0;
static unsigned long lastGraphSend = 0;
float lastSentDistance = 0;
unsigned long lastSendTime = 0;
bool alertSent = false;

// ================= PROGRESS BAR =================
String getProgressBar(float level) {
  int bars = level / 10;
  String bar = "";

  for (int i = 0; i < 10; i++) {
    if (i < bars) {
      if (i < 6) bar += "🟩";
      else if (i < 8) bar += "🟨";
      else bar += "🟥";
    } else {
      bar += "⬜";
    }
  }

  return bar;
}

// ================= DISCORD =================
void sendDashboard(float level, float timeToFull) {
  HTTPClient http;
  http.begin(webhook);
  http.addHeader("Content-Type", "application/json");

  String status;
  String color;
  String mention = "";

  if (level > 80) {
    status = "🔴 Full";
    color = "16711680";
    mention = "<@&1499159081901101116>";
  } 
  else if (level > 60) {
    status = "🟡 Warning";
    color = "16776960";
  } 
  else {
    status = "🟢 Normal";
    color = "3066993";
  }

  if (timeToFull < 0 || timeToFull > 24) timeToFull = 0;

  String json = "{";
  json += "\"content\":\"" + mention + "\",";
  json += "\"embeds\":[{";
  json += "\"title\":\"🗑️ Smart Bin Dashboard\",";
  json += "\"color\":" + color + ",";
  json += "\"fields\":[";

  json += "{ \"name\":\"📊 Level\", \"value\":\"" + String(level,2) + "%\", \"inline\":true },";
  json += "{ \"name\":\"⏳ TTF\", \"value\":\"" + String(timeToFull,2) + " hr\", \"inline\":true },";
  json += "{ \"name\":\"🧱 Bar\", \"value\":\"" + getProgressBar(level) + "\", \"inline\":false },";
  json += "{ \"name\":\"📌 Status\", \"value\":\"" + status + "\", \"inline\":false }";

  json += "]";
  json += "}]";
  json += "}";

  http.POST(json);
  http.end();
}
void sendToSheets(float level, float distance, float rate, float ttf) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = sheetURL +
      "?level=" + String(level,2) +
      "&distance=" + String(distance,2) +
      "&rate=" + String(rate,2) +
      "&ttf=" + String(ttf,2);

    http.begin(url);
    http.GET();
    http.end();
  }
}
// ================= ULTRASONIC =================
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return binHeight;

  float distance = (duration * 0.034 / 2) * 16;

  if (distance > binHeight) distance = binHeight;

  return distance;
}
void sendGraph() {

  HTTPClient http;

  http.begin(webhookgraph);

  http.addHeader("Content-Type", "application/json");

  String dynamicChart = chartURL + "&t=" + String(micros()) + "&r=" + String(random(1000000));
  String json = "{";
  json += "\"embeds\": [{";
  json += "\"title\": \"📈 Smart Bin Analytics\",";
  json += "\"color\": 3447003,";
  json += "\"image\": {";
  json += "\"url\": \"" + dynamicChart + "\"";
  json += "}";
  json += "}]";
  json += "}";

  int code = http.POST(json);

  Serial.println("Graph Code: " + String(code));

  http.end();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  while (webhook == "") {
    if (Serial.available()) {
      Serial.println("Paste webhook:");
      webhook = Serial.readString();
      webhook.trim();
    }
  }
  while (webhookgraph == "") {
    if (Serial.available()) {
      Serial.println("Paste webhook graph:");
      webhookgraph = Serial.readString();
      webhookgraph.trim();
    }
  }
  while (sheetURL == "") {
    if (Serial.available()) {
      Serial.println("Paste sheetURL:");
      sheetURL = Serial.readString();
      sheetURL.trim();
    }
  }
  while (chartURL == "") {
    if (Serial.available()) {
      Serial.println("Paste chartURL:");
      chartURL = Serial.readString();
      chartURL.trim();
    }
  }
}

// ================= LOOP =================
void loop() {

  float distance = getDistance();
  float level = (binHeight - distance) / binHeight * 100;

  if (level < 0) level = 0;
  if (level > 100) level = 100;
  unsigned long currentTime = millis();
  // ===== SMOOTH =====
  

  unsigned long now = millis();
  float dt = (now - prevTime) / 1000.0;

  // ===== RATE =====
  if (prevTime != 0 && dt > 10) {

    float diff = level - prevLevel;

    if (abs(diff) > 1.0) {

      rate = diff / (dt / 3600.0);

      if (rate < 0) rate = 0;
      if (rate > 200) rate = 200;

      if (rate > 0.1) {
        timeToFull = (100 - level) / rate;
      }

      lastRate = rate;
      lastTTF = timeToFull;
    }else if (diff < -2.0){
      rate = 0;
      timeToFull = 0;

      lastRate = 0;
      lastTTF = 0;
    }
  }
  else {
    rate = lastRate;
    timeToFull = lastTTF;
  }

  prevLevel = level;
  prevTime = now;

  // ===== SEND =====
  bool distanceChanged = abs(distance - lastSentDistance) > 2.0;
  bool timePassed = millis() - lastSendTime > 30000;
  
  if (distanceChanged || timePassed) {
    sendDashboard(level, timeToFull);
    sendToSheets(level, distance, rate, timeToFull);

    lastSentDistance = distance;
    lastSendTime = millis();
  }

  // ===== ALERT =====
  if (level > 80 && !alertSent) {
    sendDashboard(level, timeToFull);
    alertSent = true;
  }

  if (level < 70) alertSent = false;

  

  if (millis() - lastGraphSend > 50000) {
    sendGraph();
    lastGraphSend = millis();
  }
  Serial.println("distance: " + String(distance));
  Serial.println("Level: " + String(level));
  Serial.println("Rate: " + String(rate));
  Serial.println("TTF: " + String(timeToFull));
  delay(5000);
}