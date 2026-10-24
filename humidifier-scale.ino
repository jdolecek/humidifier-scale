#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <HX711.h>  // Rob Tillaart, version 0.6.5
#include "dashboard.h"

constexpr uint8_t DOUT_PIN = 14;  // D5
constexpr uint8_t SCK_PIN = 12;   // D6
constexpr size_t HISTORY_SIZE = 80;
constexpr uint32_t STALE_MS = 2000;

HX711 scale;
ESP8266WebServer server(80);
struct Sample { uint32_t seq; uint32_t ms; int32_t raw; };
Sample history[HISTORY_SIZE];
size_t head = 0, count = 0;
uint32_t sequence = 0, lastSampleMs = 0;

void sendReadings() {
  const uint32_t now = millis();
  // Always return the bounded buffer. Clients deduplicate by sequence number.
  // This avoids missing samples between browser polls, including at 80 SPS.
  String json;
  json.reserve(4000);
  json += F("{\"uptime\":"); json += now;
  json += F(",\"ok\":"); json += (count && now - lastSampleMs < STALE_MS) ? "true" : "false";
  json += F(",\"samples\":[");
  for (size_t i = 0; i < count; ++i) {
    const Sample &s = history[(head + HISTORY_SIZE - count + i) % HISTORY_SIZE];
    if (i) json += ',';
    json += '['; json += s.seq; json += ','; json += s.ms;
    json += ','; json += s.raw; json += ']';
  }
  json += F("]}");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  // Fast-processor timing is required on the ESP8266. Skip the library's
  // blocking reset. A short power cycle sets channel A / gain 128 without
  // waiting for a conversion, even when only the ESP8266 was reset.
  scale.begin(DOUT_PIN, SCK_PIN, true, false);
  scale.power_down();
  scale.power_up();
  pinMode(DOUT_PIN, INPUT_PULLUP);  // Unplugged DOUT should report not-ready.
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP("HX711-Scale", "hx711test")) {
    Serial.println("ERROR: Wi-Fi access point failed to start");
  }
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML);
  });
  server.on("/api/readings", HTTP_GET, sendReadings);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  Serial.println("\nHX711-Scale / password: hx711test");
  Serial.print("Graph: http://"); Serial.println(WiFi.softAPIP());
  Serial.println("Raw ADC counts; not calibrated weight.");
}

void loop() {
  // Never wait for the HX711: the dashboard stays available if it is unplugged.
  if (scale.is_ready()) {
    const int32_t raw = static_cast<int32_t>(scale.read());
    lastSampleMs = millis();
    history[head] = {++sequence, lastSampleMs, raw};
    head = (head + 1) % HISTORY_SIZE;
    if (count < HISTORY_SIZE) ++count;
    Serial.println(raw);  // Also works with Arduino Serial Plotter at 115200.
  }
  server.handleClient();
  delay(0);
}
