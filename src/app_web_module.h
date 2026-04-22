#pragma once
#include <WebServer.h>
#include "esp_camera.h"
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

// --- THÊM HÀM XỬ LÝ API THAY ĐỔI CẤU HÌNH CAMERA ---
void handleCameraControl() {
    if (appServer.hasArg("var") && appServer.hasArg("val")) {
        String variable = appServer.arg("var");
        int val = appServer.arg("val").toInt();
        
        // Lấy con trỏ quản lý cảm biến camera hiện tại
        sensor_t * s = esp_camera_sensor_get();
        if (s == NULL) {
            appServer.send(500, "text/plain", "Loi Cam Sensor");
            return;
        }

        int res = 0;

        // Áp dụng thông số tùy theo tên biến (var) gửi xuống
        if (variable == "framesize") {
            if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
        }
        else if (variable == "quality") res = s->set_quality(s, val);
        else if (variable == "contrast") res = s->set_contrast(s, val);
        else if (variable == "brightness") res = s->set_brightness(s, val);
        else if (variable == "saturation") res = s->set_saturation(s, val);
        else if (variable == "hmirror") res = s->set_hmirror(s, val);
        else if (variable == "vflip") res = s->set_vflip(s, val);
        else if (variable == "special_effect") res = s->set_special_effect(s, val);
        else {
            res = -1; // Biến không hợp lệ
        }

        if (res == 0) {
            appServer.send(200, "text/plain", "OK");
        } else {
            appServer.send(500, "text/plain", "Failed");
        }
    } else {
        appServer.send(400, "text/plain", "Thieu tham so");
    }
}
// ---------------------------------------------------

void startAppWebServer() {
    appServer.on("/", HTTP_GET, handleDashboard);
    appServer.on("/action", HTTP_GET, handleAIAction); 
    
    // Đăng ký API điều khiển camera
    appServer.on("/control", HTTP_GET, handleCameraControl); 
    
    appServer.begin();
    Serial.println("Web AI da san sang tren cong 80!");
}

void appWebLoop() { appServer.handleClient(); }