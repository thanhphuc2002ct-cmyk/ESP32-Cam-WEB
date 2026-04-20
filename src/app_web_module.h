#pragma once
#include <WebServer.h>
#include "ai_dashboard.h"

WebServer appServer(80);

void handleDashboard() { appServer.send(200, "text/html", ai_dashboard_html); }

void handleAIAction() {
    if (appServer.hasArg("cmd")) {
        String action = appServer.arg("cmd");
        Serial.print(">>> AI RA LENH: ");
        Serial.println(action);

        appServer.send(200, "text/plain", "OK");
    } else {
        appServer.send(400, "text/plain", "Thieu tham so");
    }
}

void startAppWebServer() {
    appServer.on("/", HTTP_GET, handleDashboard);
    appServer.on("/action", HTTP_GET, handleAIAction); 
    appServer.begin();
    Serial.println("Web AI da san sang tren cong 80!");
}

void appWebLoop() { appServer.handleClient(); }