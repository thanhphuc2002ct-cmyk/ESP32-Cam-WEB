#include <Arduino.h>
#include <Preferences.h>
#include <ESPmDNS.h>

#include "camera_module.h"
#include "wifi_module.h"
#include "app_web_module.h" 

// --- CẤU HÌNH CHÂN NÚT NHẤN & LED ---
#define LED_PIN 2    // Chân nối với LED D2
#define BTN_PIN 20    // Chân nối với Nút nhấn (Dùng luôn nút BOOT/IO0)

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

    // 1. Khởi tạo phần cứng Nút nhấn & LED
    pinMode(LED_PIN, OUTPUT);
    // Chân IO0 đã có sẵn trở kéo lên nội bộ trên mạch nên dùng INPUT_PULLUP
    pinMode(BTN_PIN, INPUT_PULLUP); 
    digitalWrite(LED_PIN, LOW); // Tắt LED ban đầu

    // 2. Khởi động Camera
    if (!initCamera()) {
        Serial.println("Loi khoi dong Camera!"); return;
    }

    // 3. Đọc bộ nhớ NVS xem có WiFi chưa
    mainPrefs.begin("wifi_config", true);
    String savedSSID = mainPrefs.getString("ssid", "");
    String savedPass = mainPrefs.getString("pass", "");
    mainPrefs.end();

    if (savedSSID == "") {
        // CHẾ ĐỘ 1: CÀI ĐẶT WIFI (Chưa kết nối)
        isSetupMode = true;
        startWifiSetupAP();
    } 
    else {
        // CHẾ ĐỘ 2: CHẠY AI (Đã có mạng)
        isSetupMode = false;
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());

        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) { delay(500); timeout++; }

        if (WiFi.status() == WL_CONNECTED) {
            String myIP = WiFi.localIP().toString();
            
            // Bật mDNS
            if (MDNS.begin("esp32camAI")) {
                Serial.println("Vao bang mDNS: http://esp32camAI.local");
            }

            // Bật "Biển quảng cáo" WiFi
            String mac = WiFi.macAddress();
            String shortMac = mac.substring(12, 14) + mac.substring(15, 17);
            String apName = myIP + "-cam-" + shortMac; 
            WiFi.softAP(apName.c_str(), ""); 
            
            Serial.println("Vao bang IP: http://" + myIP);
            
            startCameraServer(); 
            startAppWebServer(); 
        } else {
            // Nếu lỗi mạng -> Tự động xóa NVS và Reset
            mainPrefs.begin("wifi_config", false);
            mainPrefs.clear();
            mainPrefs.end();
            delay(1000); ESP.restart(); 
        }
    }
}

void loop() {
    // ---------------------------------------------------------
    // TÁC VỤ 1: DUY TRÌ MÁY CHỦ WEB
    // ---------------------------------------------------------
    if (isSetupMode) {
        wifiSetupLoop();
    } else {
        appWebLoop();
    }

    // ---------------------------------------------------------
    // TÁC VỤ 2: XỬ LÝ LED D2 (Chớp tắt hoặc Sáng đứng)
    // ---------------------------------------------------------
    if (isSetupMode) {
        // Đang ở chế độ AP (chờ kết nối): LED nháy mỗi 0.3s (300ms)
        if (millis() - lastBlinkTime >= 300) {
            lastBlinkTime = millis();
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
        }
    } else {
        // Đã kết nối mạng nhà: LED sáng liên tục
        digitalWrite(LED_PIN, HIGH);
    }

    // ---------------------------------------------------------
    // TÁC VỤ 3: NÚT NHẤN RESET (Đè tay 5 giây)
    // ---------------------------------------------------------
    if (digitalRead(BTN_PIN) == LOW) { // Nếu nút bị đè (Mức thấp)
        if (!isBtnPressed) {
            isBtnPressed = true;
            btnPressTime = millis(); // Ghi lại thời gian bắt đầu nhấn
        } else {
            // Kiểm tra xem đã đè đủ 5000ms chưa
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