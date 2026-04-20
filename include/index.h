#pragma once

const char PAGE_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cài đặt WiFi ESP32</title>
    <style>
        * {
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        .container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 40px;
            width: 100%;
            max-width: 420px;
            animation: slideUp 0.5s ease-out;
        }
        @keyframes slideUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
            padding-bottom: 20px;
            border-bottom: 2px solid #f0f0f0;
        }
        .header h1 {
            margin: 0 0 8px 0;
            color: #333;
            font-size: 2em;
            font-weight: 700;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        .header span {
            font-size: 0.9em;
            color: #666;
            display: inline-flex;
            align-items: center;
            gap: 6px;
        }
        .header span::before {
            content: "IP: ";
            font-size: 1.2em;
        }
        .form-group {
            margin-bottom: 24px;
        }
        label {
            display: block;
            margin-bottom: 10px;
            font-weight: 600;
            color: #333;
            font-size: 0.95em;
        }
        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 14px 16px;
            border: 2px solid #e0e0e0;
            border-radius: 12px;
            font-size: 1em;
            transition: all 0.3s ease;
            background: #fff;
        }
        input:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        .scan-btn-wrapper {
            display: flex;
            align-items: flex-end;
            gap: 12px;
        }
        .scan-btn-wrapper label {
            flex: 1;
            margin-bottom: 10px;
        }
        button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 14px 24px;
            border: none;
            border-radius: 12px;
            cursor: pointer;
            font-size: 1em;
            font-weight: 600;
            width: 100%;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.5);
        }
        button:active {
            transform: translateY(0);
        }
        button:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            transform: none;
        }
        #scanBtn {
            width: auto;
            padding: 14px 28px;
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            box-shadow: 0 4px 15px rgba(79, 172, 254, 0.4);
            white-space: nowrap;
        }
        #scanBtn:hover {
            box-shadow: 0 6px 20px rgba(79, 172, 254, 0.5);
        }
        #scanBtn.scanning {
            animation: pulse 1.5s ease-in-out infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
        #status {
            margin-top: 20px;
            text-align: center;
            font-weight: 600;
            font-size: 0.95em;
            min-height: 24px;
            padding: 12px;
            border-radius: 8px;
            transition: all 0.3s ease;
        }
        #status.success {
            background: #d4edda;
            color: #155724;
        }
        #status.error {
            background: #f8d7da;
            color: #721c24;
        }
        #status.info {
            background: #d1ecf1;
            color: #0c5460;
        }
        .show-pass {
            margin-top: 12px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .show-pass input[type="checkbox"] {
            width: auto;
            margin: 0;
            cursor: pointer;
        }
        .show-pass label {
            margin: 0;
            font-weight: 500;
            font-size: 0.9em;
            color: #666;
            cursor: pointer;
        }
        select {
            cursor: pointer;
        }
        select option {
            padding: 10px;
        }
    </style>
</head>
<body>

    <div class="container">
        <div class="header">
            <h1> ESP32</h1>
            <span>192.168.4.1</span>
        </div>

        <div class="form-group scan-btn-wrapper">
            <label for="ssid">Mạng WiFi</label>
            <button id="scanBtn" type="button"> Quét</button>
        </div>

        <div class="form-group">
            <select id="ssid" name="ssid">
                <option value="">Chọn mạng WiFi...</option>
            </select>
        </div>

        <div class="form-group">
            <label for="password">Mật khẩu</label>
            <input type="password" id="password" name="password" placeholder="Nhập mật khẩu WiFi">
            
            <div class="show-pass">
                <input type="checkbox" id="showPass">
                <label for="showPass">Hiển thị mật khẩu</label>
            </div>
        </div>

        <button id="connectBtn" type="button">Lưu & Kết Nối</button>
        <p id="status"></p>
    </div>

    <script>
        const scanBtn = document.getElementById('scanBtn');
        const connectBtn = document.getElementById('connectBtn');
        const ssidSelect = document.getElementById('ssid');
        const passwordInput = document.getElementById('password');
        const statusEl = document.getElementById('status');
        const showPassCheckbox = document.getElementById('showPass');

        function showStatus(message, type) {
            statusEl.textContent = message;
            statusEl.className = type || '';
        }

        showPassCheckbox.addEventListener('change', () => {
            passwordInput.type = showPassCheckbox.checked ? 'text' : 'password';
        });

        scanBtn.addEventListener('click', () => {
            scanBtn.disabled = true;
            scanBtn.classList.add('scanning');
            showStatus('Đang quét mạng WiFi...', 'info');
            ssidSelect.innerHTML = '<option value="">Đang tìm mạng...</option>';

            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    scanBtn.disabled = false;
                    scanBtn.classList.remove('scanning');
                    
                    if (data && data.ssids && data.ssids.length > 0) {
                        ssidSelect.innerHTML = '';
                        data.ssids.forEach((ssid) => {
                            const option = document.createElement('option');
                            option.value = ssid;
                            option.textContent = ssid;
                            ssidSelect.appendChild(option);
                        });
                        showStatus(` Tìm thấy ${data.ssids.length} mạng WiFi`, 'success');
                    } else {
                        ssidSelect.innerHTML = '<option value="">Không tìm thấy mạng</option>';
                        showStatus(' Không tìm thấy mạng WiFi nào', 'error');
                    }
                })
                .catch(error => {
                    scanBtn.disabled = false;
                    scanBtn.classList.remove('scanning');
                    ssidSelect.innerHTML = '<option value="">Lỗi khi quét</option>';
                    showStatus(' Lỗi khi quét mạng', 'error');
                });
        });

        connectBtn.addEventListener('click', () => {
            const ssid = ssidSelect.value;
            const password = passwordInput.value;

            if (!ssid) {
                showStatus(' Vui lòng chọn mạng WiFi', 'error');
                return;
            }

            connectBtn.disabled = true;
            showStatus('Đang lưu cấu hình...', 'info');

            const bodyParams = new URLSearchParams();
            bodyParams.append('ssid', ssid);
            bodyParams.append('password', password);

            fetch('/connect', {
                method: 'POST',
                body: bodyParams 
            })
            .then(response => response.text())
            .then(text => {
                showStatus('Đã lưu. Đang khởi động lại...' , 'success');
            })
            .catch(error => {
                connectBtn.disabled = false;
                showStatus(' Lỗi khi kết nối', 'error');
            });
        });
    </script>
</body>
</html>
)=====";
