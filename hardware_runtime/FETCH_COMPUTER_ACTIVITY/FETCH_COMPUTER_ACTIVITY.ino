#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const int writeOutPin = D1;

// Wi-Fi configuration
const char* WIFI_SSID = "YOUR_WIFI_SSD";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Replace with your computer's LAN IP
const char* TELEMETRY_API_ENDPOINT =
    "";

// Intensity thresholds
constexpr float EXTREME_THRESHOLD_KEYBOARD_PRESSES = 60.0f;
constexpr float EXTREME_THRESHOLD_MOUSE_CLICKS = 30.0f;
constexpr float EXTREME_CPU_PERCENT = 100.0f;
constexpr float EXTREME_MEMORY_PERCENT = 100.0f;
constexpr float EXTREME_RX_BYTES_PER_SECOND = 10000000.0f;
constexpr float EXTREME_TX_BYTES_PER_SECOND = 5000000.0f;

// ESP8266 PWM range
constexpr int MAX_PWM = 1023;

// Polling interval
constexpr unsigned long POLL_INTERVAL_MS = 2000;

struct Telemetry {
    float cpu_percent;
    float memory_percent;
    float network_rx_bytes_per_sec;
    float network_tx_bytes_per_sec;
    int mouse_clicks_last_2s;
    int keyboard_presses_last_2s;
};

float getAnalogWriteoutValue(
    const Telemetry& telemetry,
    int maxWriteout
) {
    float cpuWeight = 0.2f * constrain(
        telemetry.cpu_percent / EXTREME_CPU_PERCENT,
        0.0f, 1.0f
    );

    float rxWeight = 0.1f * constrain(
        telemetry.network_rx_bytes_per_sec / EXTREME_RX_BYTES_PER_SECOND,
        0.0f, 1.0f
    );

    float txWeight = 0.1f * constrain(
        telemetry.network_tx_bytes_per_sec / EXTREME_TX_BYTES_PER_SECOND,
        0.0f, 1.0f
    );

    float mouseWeight = 0.3f * constrain(
        static_cast<float>(telemetry.mouse_clicks_last_2s) /
            EXTREME_THRESHOLD_MOUSE_CLICKS,
        0.0f, 1.0f
    );

    float keyboardWeight = 0.3f * constrain(
        static_cast<float>(telemetry.keyboard_presses_last_2s) /
            EXTREME_THRESHOLD_KEYBOARD_PRESSES,
        0.0f, 1.0f
    );

    float totalWeight =
        cpuWeight +
        rxWeight +
        txWeight +
        mouseWeight +
        keyboardWeight;

    return maxWriteout * totalWeight;
}

bool fetchTelemetry(Telemetry& telemetry) {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    http.setTimeout(1500);

    if (!http.begin(client, TELEMETRY_API_ENDPOINT)) {
        return false;
    }

    int statusCode = http.GET();

    if (statusCode != HTTP_CODE_OK) {
        Serial.printf("HTTP error: %d\n", statusCode);
        http.end();
        return false;
    }

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, http.getStream());

    if (error) {
        Serial.println("JSON parsing failed");
        http.end();
        return false;
    }

    telemetry.cpu_percent =
        doc["cpu_percent"] | 0.0f;

    telemetry.memory_percent =
        doc["memory_percent"] | 0.0f;

    telemetry.network_rx_bytes_per_sec =
        doc["network_rx_bytes_per_sec"] | 0.0f;

    telemetry.network_tx_bytes_per_sec =
        doc["network_tx_bytes_per_sec"] | 0.0f;

    telemetry.mouse_clicks_last_2s =
        doc["mouse_clicks_last_2s"] | 0;

    telemetry.keyboard_presses_last_2s =
        doc["keyboard_presses_last_2s"] | 0;

    http.end();
    return true;
}

void setup() {
    Serial.begin(115200);

    pinMode(writeOutPin, OUTPUT);
    analogWriteRange(MAX_PWM);
    analogWrite(writeOutPin, 0);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    static unsigned long lastPoll = 0;

    if (millis() - lastPoll < POLL_INTERVAL_MS) {
        return;
    }

    lastPoll = millis();

    Telemetry telemetry{};

    if (!fetchTelemetry(telemetry)) {
        Serial.println("Failed to fetch telemetry");
        return;
    }

    int pwmValue = static_cast<int>(
        getAnalogWriteoutValue(telemetry, MAX_PWM)
    );

    analogWrite(writeOutPin, pwmValue);

    Serial.printf(
        "Intensity: %.1f%% | PWM: %d\n",
        100.0f * pwmValue / MAX_PWM,
        pwmValue
    );
}