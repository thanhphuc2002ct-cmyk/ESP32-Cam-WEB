const char* ai_dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>AI Cam Mini - IoT Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js" type="text/javascript"></script>
    <style>
        body { font-family: 'Segoe UI', sans-serif; background: #121212; margin: 0; padding: 0; height: 100vh; overflow: hidden; display: flex; flex-direction: row; color: #eee; }
        .sidebar-left { width: 280px; background: #1e1e1e; padding: 20px; display: flex; flex-direction: column; gap: 15px; border-right: 1px solid #333; box-sizing: border-box; overflow-y: auto; flex-shrink: 0; }
        .main-center { flex: 1; display: flex; flex-direction: column; justify-content: center; align-items: center; background: #151515; overflow: hidden; position: relative; }
        .sidebar-right { width: 340px; background: #1e1e1e; padding: 20px; display: flex; flex-direction: column; gap: 15px; border-left: 1px solid #333; box-sizing: border-box; overflow-y: auto; flex-shrink: 0; }
        
        .camera-section { width: 540px; height: 405px; background: #000; border: 3px solid #444; border-radius: 12px; position: relative; box-shadow: 0 10px 40px rgba(0,0,0,0.8); overflow: hidden; flex-shrink: 0; }
        #stream { width: 100%; height: 100%; object-fit: contain; }
        #canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; }

        .status-box { padding: 12px; border-radius: 8px; background: #252525; text-align: center; font-size: 13px; font-weight: bold; border-left: 4px solid #6200ea; }
        label.section-title { font-size: 11px; color: #888; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 5px; margin-top: 5px; display: block; font-weight: bold; }
        
        .box { background:#252525; border:1px solid #444; border-radius:8px; padding:15px; }
        .mode-box { background: #252525; border: 1px solid #333; border-radius: 10px; padding: 12px; margin-bottom: 8px; cursor: pointer; transition: 0.3s; }
        .mode-box.active { border-color: #00e676; background: #1b2e20; }
        .btn-apply { width: 100%; padding: 14px; background: #00c853; color: white; border: none; border-radius: 8px; font-weight: bold; cursor: pointer; margin-top: auto; transition: 0.2s; }
        .btn-stop { background: #ff9100; display: none; }
        .latest-log { background: #1e1e1e; border: 1px solid #333; border-radius: 8px; padding: 10px; text-align: center; font-size: 11px; word-wrap: break-word; }
        
        .card { background: #252525; padding: 12px; border-radius: 8px; border: 1px solid #333; margin-bottom: 12px;}
        .card h3 { margin: 0 0 10px 0; font-size: 12px; color: #00b0ff; border-bottom: 1px solid #333; padding-bottom: 5px; text-transform: uppercase; }
        .cam-control { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; font-size: 12px; }
        .cam-control select, .cam-control input[type="range"] { width: 55%; background: #2a2a2a; color: white; border: 1px solid #444; border-radius: 4px; padding: 3px; }

        /* Form MQTTX */
        .input-group { margin-bottom: 10px; }
        .input-group label { font-size: 11px; color: #aaa; margin-bottom: 4px; display: block; }
        .box input[type="text"], .box input[type="number"], .box input[type="password"], .box select { width:100%; background:#1e1e1e; color:white; border:1px solid #555; padding: 6px; box-sizing: border-box; border-radius: 4px; font-family: inherit; font-size: 12px; }
        .box input:focus, .box select:focus { border-color: #2196F3; outline: none; }
        .btn-mqtt { width: 100%; padding: 10px; background: #2196F3; color: white; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; margin-top: 5px; transition: 0.2s; }

        ::-webkit-scrollbar { width: 6px; }
        ::-webkit-scrollbar-track { background: #121212; }
        ::-webkit-scrollbar-thumb { background: #444; border-radius: 3px; }
        ::-webkit-scrollbar-thumb:hover { background: #666; }
    </style>
</head>
<body>
    
    <div class="sidebar-left">
        <div class="status-box" id="status">Kết nối Camera...</div>
        <label class="section-title">CHỌN MÔ HÌNH AI</label>
        <label class="mode-box active" id="box-color-cloud"><div class="mode-title"><input type="radio" name="mainTask" value="colorCloud" checked onchange="updateUI()"> Màu Sắc</div></label>
        <label class="mode-box" id="box-shape-cloud"><div class="mode-title"><input type="radio" name="mainTask" value="shapeCloud" onchange="updateUI()"> Hình Dạng</div></label>
        <label class="mode-box" id="box-local"><div class="mode-title"><input type="radio" name="mainTask" value="local" onchange="updateUI()"> Mô hình AI tự train</div></label>
        <div id="upload-section" style="display: none; padding: 10px; background: #252525; border-radius: 8px; margin-top: 5px;">
            <input type="file" id="files" multiple accept=".js,.wasm" style="font-size: 10px;" onchange="loadFiles(event)">
        </div>
        <div class="box" style="margin-top: 10px; padding: 12px;">
            <label style="font-size: 11px; color: #ff9100; font-weight: bold; margin-bottom: 8px; display: block;">ĐỘ TỰ TIN: <span id="thresh-val">50%</span></label>
            <input type="range" id="threshold-slider" style="width:100%" min="10" max="99" value="50" oninput="updateThresh(this.value)">
        </div>
        <button class="btn-apply" id="btn-start" onclick="startAI()">CHẠY AI</button>
        <button class="btn-apply btn-stop" id="btn-stop" onclick="stopAI()">DỪNG AI</button>
        <div class="latest-log" id="log-box">Sẵn sàng!</div>
    </div>

    <div class="main-center">
        <div class="camera-section">
            <img id="stream" src="" crossorigin="anonymous">
            <canvas id="canvas"></canvas>
        </div>
    </div>

    <div class="sidebar-right">
        <label class="section-title">☁️ CÀI ĐẶT IOT (MQTT)</label>
        <div class="box">
            <div class="input-group">
                <label>Broker (Host):</label>
                <div style="display: flex; gap: 5px;">
                    <select id="mqtt-protocol" style="width: 30%;">
                        <option value="wss">wss://</option>
                        <option value="ws">ws://</option>
                    </select>
                    <input type="text" id="mqtt-server" value="broker.emqx.io" style="width: 70%;">
                </div>
            </div>
            <div style="display: flex; gap: 10px;">
                <div class="input-group" style="width: 40%;">
                    <label>Port:</label>
                    <input type="number" id="mqtt-port" value="8084">
                </div>
                <div class="input-group" style="width: 60%;">
                    <label>Client ID:</label>
                    <div style="display: flex; gap: 5px;">
                        <input type="text" id="mqtt-clientid" value="CamMini" style="width: 80%;">
                        <button onclick="document.getElementById('mqtt-clientid').value = 'Cam_' + Math.random().toString(16).substr(2, 6)" style="width: 20%; cursor: pointer; background: #333; color: white; border: 1px solid #555; border-radius: 4px;">🔄</button>
                    </div>
                </div>
            </div>
            <hr style="border: 0; border-top: 1px solid #444; margin: 10px 0;">
            <div class="input-group"><label>Username (Tùy chọn):</label><input type="text" id="mqtt-user" placeholder="Nhập Username nếu có"></div>
            <div class="input-group"><label>Password (Tùy chọn):</label><input type="password" id="mqtt-pass" placeholder="Nhập Password nếu có"></div>
            <hr style="border: 0; border-top: 1px solid #444; margin: 10px 0;">
            <div class="input-group"><label>Publish Topic:</label><input type="text" id="mqtt-topic" value="ai_cam/detect"></div>
            <button class="btn-mqtt" id="btn-mqtt-connect" onclick="toggleMQTT()">KẾT NỐI SERVER</button>
            <div id="mqtt-status" style="color:#ff9100; margin-top:10px; text-align:center; font-size: 12px; font-weight: bold;">Chưa kết nối</div>
        </div>

        <label class="section-title">⚙️ THÔNG SỐ CAMERA</label>
        <div class="card">
            <h3>Cơ bản</h3>
            <div class="cam-control"><span>Độ phân giải</span><select id="framesize" onchange="updateCam('framesize', this.value)"><option value="9">SVGA (800x600)</option><option value="6">VGA (640x480)</option><option value="5" selected>CIF (400x296)</option><option value="4">QVGA (320x240)</option></select></div>
            <div class="cam-control"><span>Chất lượng</span><input type="range" id="quality" min="10" max="63" value="12" onchange="updateCam('quality', this.value)"></div>
        </div>
        <div class="card">
            <h3>Hình ảnh</h3>
            <div class="cam-control"><span>Độ sáng</span><input type="range" min="-2" max="2" value="0" onchange="updateCam('brightness', this.value)"></div>
            <div class="cam-control"><span>Tương phản</span><input type="range" min="-2" max="2" value="0" onchange="updateCam('contrast', this.value)"></div>
        </div>
        <div class="card" style="margin-bottom: 0;">
            <h3>Xoay & Lật</h3>
            <div class="cam-control"><span>Lật dọc</span><input type="checkbox" onchange="updateCam('vflip', this.checked ? 1 : 0)"></div>
            <div class="cam-control"><span>Lật ngang</span><input type="checkbox" onchange="updateCam('hmirror', this.checked ? 1 : 0)"></div>
        </div>
    </div>

    <iframe id="ai-sandbox" style="display:none;"></iframe>

    <script>
        const URLS = {
            color: { wasm: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/edge-impulse-standalone.wasm", core: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/edge-impulse-standalone.js", run: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/run-impulse.js" },
            shape: { wasm: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/edge-impulse-standalone.wasm", core: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/edge-impulse-standalone.js", run: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/run-impulse.js" }
        };
        const streamImg = document.getElementById('stream'); 
        const canvas = document.getElementById('canvas'); 
        const ctx = canvas.getContext('2d'); 
        const statusEl = document.getElementById('status'); 
        const logBox = document.getElementById('log-box');
        
        let jsStandalone = null, jsRun = null, wasmFile = null;
        let classifier = null, isRunning = false, aiInterval;
        let currentThreshold = 0.5;

        let mqttClient = null;
        let isMqttConnected = false;
        let lastPubLabel = "";
        let lastPubTime = 0;

        window.onload = () => { 
            streamImg.src = `http://${location.hostname}:81/stream`; 
            document.getElementById('mqtt-clientid').value = 'Cam_' + Math.random().toString(16).substr(2, 6);
        };
        
        function writeLog(msg) { if(logBox) logBox.innerText = msg; }
        function updateThresh(v) { document.getElementById('thresh-val').innerText = v + "%"; currentThreshold = v / 100; }
        function updateCam(n, v) { fetch(`/control?var=${n}&val=${v}`); }
        
        function updateUI() { 
            const v = document.querySelector('input[name="mainTask"]:checked').value; 
            document.querySelectorAll('.mode-box').forEach(el => el.classList.remove('active'));
            const uploadSec = document.getElementById('upload-section');
            if(uploadSec) uploadSec.style.display = (v === 'local') ? 'block' : 'none';
            if(v === "colorCloud") document.getElementById('box-color-cloud').classList.add('active');
            if(v === "shapeCloud") document.getElementById('box-shape-cloud').classList.add('active');
            if(v === "local") document.getElementById('box-local').classList.add('active');
        }

        function loadFiles(e) { 
            jsStandalone = null; jsRun = null; wasmFile = null;
            Array.from(e.target.files).forEach(f => { 
                if(f.name === 'edge-impulse-standalone.js') jsStandalone = f; 
                else if(f.name === 'run-impulse.js') jsRun = f; 
                else if(f.name.endsWith('.wasm')) wasmFile = f; 
            }); 
            if(jsStandalone && jsRun && wasmFile) writeLog("Đã nhận đủ 3 file mới!");
            else writeLog("Lỗi: Hãy chọn đủ 3 file chuẩn.");
        }

        function toggleMQTT() {
            let btn = document.getElementById("btn-mqtt-connect");
            let statusText = document.getElementById("mqtt-status");

            if (isMqttConnected) {
                if (mqttClient) mqttClient.disconnect();
                isMqttConnected = false;
                statusText.innerText = "Đã ngắt kết nối"; statusText.style.color = "#aaa";
                btn.innerText = "KẾT NỐI SERVER"; btn.style.background = "#2196F3";
                return;
            }

            let protocol = document.getElementById("mqtt-protocol").value;
            let server = document.getElementById("mqtt-server").value;
            let port = parseInt(document.getElementById("mqtt-port").value);
            let clientId = document.getElementById("mqtt-clientid").value || "CamMini";
            let user = document.getElementById("mqtt-user").value;
            let pass = document.getElementById("mqtt-pass").value;

            if(!server || !port) { alert("Vui lòng nhập Broker và Port!"); return; }
            
            mqttClient = new Paho.MQTT.Client(server, port, "/mqtt", clientId);
            
            mqttClient.onConnectionLost = () => { 
                isMqttConnected = false; 
                statusText.innerText = "Mất kết nối"; statusText.style.color = "#ff1744";
                btn.innerText = "KẾT NỐI SERVER"; btn.style.background = "#2196F3";
            };
            
            statusText.innerText = "Đang kết nối..."; statusText.style.color = "#ff9100";
            
            let connectOptions = {
                useSSL: (protocol === "wss"),
                onSuccess: () => { 
                    isMqttConnected = true; 
                    statusText.innerText = "Đã kết nối Broker!"; statusText.style.color = "#00e676"; 
                    btn.innerText = "NGẮT KẾT NỐI"; btn.style.background = "#ff1744";
                    writeLog("MQTT Sẵn sàng");
                },
                onFailure: (err) => { 
                    isMqttConnected = false; 
                    statusText.innerText = "Lỗi kết nối MQTT"; statusText.style.color = "#ff1744";
                }
            };

            if(user !== "") connectOptions.userName = user;
            if(pass !== "") connectOptions.password = pass;

            mqttClient.connect(connectOptions);
        }

        function publishData(label) {
            if (!isMqttConnected) return;
            let now = Date.now();
            if (label === lastPubLabel && (now - lastPubTime) < 3000) return; 
            
            let fullTopic = document.getElementById("mqtt-topic").value; 
            if(fullTopic === "") return;
            
            let message = new Paho.MQTT.Message(label);
            message.destinationName = fullTopic;
            mqttClient.send(message);
            
            lastPubLabel = label;
            lastPubTime = now;
            console.log("MQTT Sent -> " + fullTopic + ": " + label);
        }

        async function startAI() {
            const v = document.querySelector('input[name="mainTask"]:checked').value;
            if (v === "local" && (!jsStandalone || !jsRun || !wasmFile)) { writeLog("Lỗi: Bạn chưa chọn đủ 3 file!"); return; }
            statusEl.innerText = "NẠP MÔ HÌNH...";
            let oldBox = document.getElementById('ai-sandbox'); if(oldBox) oldBox.remove(); 
            let newBox = document.createElement('iframe'); newBox.id = 'ai-sandbox'; newBox.style.display = 'none'; 
            document.body.appendChild(newBox); const win = newBox.contentWindow;

            try {
                if (v === "colorCloud" || v === "shapeCloud") {
                    const s = (v === "colorCloud") ? URLS.color : URLS.shape;
                    const [w, c, r] = await Promise.all([fetch(s.wasm), fetch(s.core), fetch(s.run)]);
                    win.Module = { wasmBinary: await w.arrayBuffer() };
                    const s1 = win.document.createElement('script'); s1.textContent = await c.text(); win.document.head.appendChild(s1);
                    const s2 = win.document.createElement('script'); s2.textContent = await r.text(); win.document.head.appendChild(s2);
                    const s3 = win.document.createElement('script'); s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`; win.document.head.appendChild(s3);
                    classifier = await win.initAI(); activateAI();
                } else {
                    const readerWasm = new FileReader();
                    readerWasm.onload = function() {
                        win.Module = { wasmBinary: readerWasm.result };
                        const readerCore = new FileReader();
                        readerCore.onload = function() {
                            const readerRun = new FileReader();
                            readerRun.onload = async function() {
                                const s1 = win.document.createElement('script'); s1.textContent = readerCore.result; win.document.head.appendChild(s1);
                                const s2 = win.document.createElement('script'); s2.textContent = readerRun.result; win.document.head.appendChild(s2);
                                const s3 = win.document.createElement('script'); s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`; win.document.head.appendChild(s3);
                                classifier = await win.initAI(); activateAI();
                            }; readerRun.readAsText(jsRun);
                        }; readerCore.readAsText(jsStandalone);
                    }; readerWasm.readAsArrayBuffer(wasmFile);
                }
            } catch (e) { statusEl.innerText = "LỖI NẠP AI"; stopAI(); }
        }

        function activateAI() {
            statusEl.innerText = "ĐANG CHẠY AI"; isRunning = true; 
            document.getElementById('btn-start').style.display='none'; document.getElementById('btn-stop').style.display='block'; 
            processFrame();
        }

        function stopAI() { 
            isRunning = false; cancelAnimationFrame(aiInterval); 
            document.getElementById('btn-start').style.display='block'; document.getElementById('btn-stop').style.display='none'; 
            statusEl.innerText = "TẠM DỪNG"; ctx.clearRect(0,0,canvas.width,canvas.height); 
        }

        async function processFrame() {
            if (!isRunning || !classifier || !streamImg.complete || streamImg.naturalWidth === 0) {
                aiInterval = requestAnimationFrame(processFrame); return;
            }
            try {
                const camW = streamImg.width; const camH = streamImg.height;
                const props = classifier.getProperties(); const aiSize = props.input_width || 96;
                const hiddenCanvas = document.createElement('canvas'); hiddenCanvas.width = aiSize; hiddenCanvas.height = aiSize;
                const hiddenCtx = hiddenCanvas.getContext('2d'); hiddenCtx.drawImage(streamImg, 0, 0, aiSize, aiSize);
                const imgData = hiddenCtx.getImageData(0, 0, aiSize, aiSize);
                let features = [];
                for (let i = 0; i < imgData.data.length; i += 4) { let pixel = (imgData.data[i] << 16) | (imgData.data[i + 1] << 8) | imgData.data[i + 2]; features.push(pixel); }
                const res = classifier.classify(features);
                canvas.width = camW; canvas.height = camH; ctx.clearRect(0, 0, camW, camH);
                let bestLabel = ""; let bestValue = 0;

                if (res.results) {
                    const sX = camW / aiSize; const sY = camH / aiSize;
                    res.results.forEach(box => {
                        if (box.value < currentThreshold) return;
                        if (box.value > bestValue) { bestValue = box.value; bestLabel = box.label; }
                        const cx = (box.x + box.width / 2) * sX; const cy = (box.y + box.height / 2) * sY;
                        let label = box.label.toLowerCase(); let color = "#ff9100"; 
                        if (label.includes("do") || label.includes("red")) { color = "#ff1744"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke(); } 
                        else if (label.includes("xanh") || label.includes("green")) { color = "#00e676"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke(); } 
                        else if (label.includes("tron") || label.includes("circle")) { color = "#29b6f6"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke(); } 
                        else if (label.includes("vuong") || label.includes("square")) { color = "#ab47bc"; ctx.fillStyle=color; ctx.fillRect(cx-12, cy-12, 24, 24); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.strokeRect(cx-12, cy-12, 24, 24); } 
                        else { ctx.fillStyle=color; ctx.fillRect(cx-12, cy-12, 24, 24); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.strokeRect(cx-12, cy-12, 24, 24); }
                        let p = Math.round(box.value * 100); ctx.font = "bold 14px Arial"; ctx.lineWidth = 3; ctx.strokeStyle = "black";
                        ctx.strokeText(`${box.label} ${p}%`, cx - 15, cy - 20); ctx.fillStyle = "white"; ctx.fillText(`${box.label} ${p}%`, cx - 15, cy - 20);
                    });
                }
                if (bestLabel !== "") { publishData(bestLabel); }
                aiInterval = requestAnimationFrame(processFrame);
            } catch (e) {
                console.error("AI Error:", e);
                if (e.toString().includes("Classification failed")) { stopAI(); writeLog("Lỗi: Model không khớp dữ liệu đầu vào!"); } 
                else { aiInterval = requestAnimationFrame(processFrame); }
            }
        }
    </script>
</body>
</html>
)rawliteral";