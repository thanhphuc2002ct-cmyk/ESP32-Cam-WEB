#include <Arduino.h>
#include <Preferences.h>
#include <ESPmDNS.h>

#include "camera_module.h"
#include "wifi_module.h"
#include "app_web_module.h" 

// --- CẤU HÌNH CHÂN NÚT NHẤN & LED ---
#define LED_PIN 2    // Chân nối với LED D2
#define BTN_PIN 0    // Chân nối với Nút nhấn (Dùng luôn nút BOOT/IO0)

Preferences mainPrefs;
bool isSetupMode = false; 

// Các biến đếm thời gian (Thay thế cho hàm delay)
unsigned long btnPressTime = 0;
bool isBtnPressed = false;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP); 
    digitalWrite(LED_PIN, LOW); // Tắt LED ban đầu

    if (!initCamera()) {
        Serial.println("Loi khoi dong Camera!"); return;
    }
    mainPrefs.begin("wifi_config", true);
    String savedSSID = mainPrefs.getString("ssid", "");
    String savedPass = mainPrefs.getString("pass", "");
    mainPrefs.end();
    sensor_t * s = esp_camera_sensor_get();

    if (savedSSID == "") {
        isSetupMode = true;
        startWifiSetupAP();
    } 
    else {
        isSetupMode = false;
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());

        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) { delay(500); timeout++; }

        if (WiFi.status() == WL_CONNECTED) {
            String myIP = WiFi.localIP().toString();
            
            if (MDNS.begin("esp32camAI")) {
                Serial.println("Vao bang mDNS: http://esp32camAI.local");
            }

            String mac = WiFi.macAddress();
            String shortMac = mac.substring(12, 14) + mac.substring(15, 17);
            String apName = myIP + "-cam-" + shortMac; 
            WiFi.softAP(apName.c_str(), ""); 
            
            Serial.println("Vao bang IP: http://" + myIP);
            
            startCameraServer(); 
            startAppWebServer(); 
        } else {
            mainPrefs.begin("wifi_config", false);
            mainPrefs.clear();
            mainPrefs.end();
            delay(1000); ESP.restart(); 
        }
    }
}

void loop() {

    if (isSetupMode) {
        wifiSetupLoop();
    } else {
        appWebLoop();
    }

    if (isSetupMode) {
        if (millis() - lastBlinkTime >= 300) {
            lastBlinkTime = millis();
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
        }
    } else {
        digitalWrite(LED_PIN, HIGH);
    }


    if (digitalRead(BTN_PIN) == LOW) { // Nếu nút bị đè (Mức thấp)
        if (!isBtnPressed) {
            isBtnPressed = true;
            btnPressTime = millis(); // Ghi lại thời gian bắt đầu nhấn
        } else {
            if (millis() - btnPressTime >= 5000) {
                Serial.println("\n[CANH BAO] Da xoa cau hinh WiFi! Dang Reset...");
                
                // Nháy LED D2 thật nhanh 5 lần để báo cho người dùng biết là đã xóa thành công
                for(int i=0; i<5; i++){
                    digitalWrite(LED_PIN, LOW); delay(100);
                    digitalWrite(LED_PIN, HIGH); delay(100);
                }

                // Xóa bộ nhớ
                mainPrefs.begin("wifi_config", false);
                mainPrefs.clear();
                mainPrefs.end();
                ESP.restart(); // Khởi động lại
            }
        }
    } else {
        // Nếu nhả tay ra thì reset lại bộ đếm thời gian
        isBtnPressed = false;
    }

    delay(10); // Nghỉ 10ms cho mạch mát
}