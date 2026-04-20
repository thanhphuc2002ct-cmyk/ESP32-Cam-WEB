#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "index.h" 

WebServer setupServer(80);
Preferences wifiPrefs;

void handleRoot() { setupServer.send(200, "text/html", PAGE_HTML); }

void handleScan() {
    int n = WiFi.scanNetworks();
    String json = "{\"ssids\":[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "\"" + WiFi.SSID(i) + "\"";
    }
    json += "]}";
    setupServer.send(200, "application/json", json);
}

void handleConnect() {
    if (setupServer.hasArg("ssid") && setupServer.hasArg("password")) {
        String newSSID = setupServer.arg("ssid");
        String newPass = setupServer.arg("password");

        wifiPrefs.begin("wifi_config", false);
        wifiPrefs.putString("ssid", newSSID);
        wifiPrefs.putString("pass", newPass);
        wifiPrefs.end();

        setupServer.send(200, "text/plain", "OK");
        delay(1000); ESP.restart(); 
    } else {
        setupServer.send(400, "text/plain", "Thieu tham so");
    }
}

void startWifiSetupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_STEM_Setup"); 
    setupServer.on("/", HTTP_GET, handleRoot);
    setupServer.on("/scan", HTTP_GET, handleScan);
    setupServer.on("/connect", HTTP_POST, handleConnect);
    setupServer.begin();
    Serial.println("Diem phat WiFi da mo. Vao 192.168.4.1 de cai dat");
}

void wifiSetupLoop() { setupServer.handleClient(); }