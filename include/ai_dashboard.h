const char* ai_dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>AI Cam Mini - Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', sans-serif; background: #121212; margin: 0; padding: 0; height: 100vh; overflow: hidden; display: flex; color: #eee; }
        
        /* Sidebar bên trái: AI */
        .sidebar { width: 300px; background: #1e1e1e; padding: 20px; display: flex; flex-direction: column; gap: 15px; border-right: 1px solid #333; box-sizing: border-box; }
        
        /* Khu vực chính: Stream (Trên) & Cấu hình (Dưới) */
        .main-content { flex: 1; display: flex; flex-direction: column; align-items: center; padding: 20px; overflow-y: auto; background: #151515; }
        
        /* Tăng kích thước khung Stream */
        .camera-section { 
            width: 540px; 
            height: 405px; 
            background: #000; 
            border: 3px solid #444; 
            border-radius: 12px; 
            position: relative; 
            margin-bottom: 30px; 
            box-shadow: 0 10px 40px rgba(0,0,0,0.8);
            overflow: hidden;
            flex-shrink: 0;
        }
        #stream { width: 100%; height: 100%; object-fit: contain; }
        #canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; }

        /* Bảng cấu hình đẩy xuống dưới */
        .settings-grid { width: 100%; max-width: 900px; display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; }
        .card { background: #1e1e1e; padding: 18px; border-radius: 12px; border: 1px solid #333; }
        .card h3 { margin: 0 0 15px 0; font-size: 14px; color: #00b0ff; border-bottom: 1px solid #333; padding-bottom: 10px; text-transform: uppercase; }
        
        .cam-control { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; font-size: 13px; }
        .cam-control select, .cam-control input[type="range"] { width: 50%; background: #2a2a2a; color: white; border: 1px solid #444; border-radius: 4px; padding: 2px; }

        /* UI Sidebar */
        .status-box { padding: 12px; border-radius: 8px; background: #252525; text-align: center; font-size: 13px; font-weight: bold; border-left: 4px solid #6200ea; margin-bottom: 10px; }
        label.section-title { font-size: 11px; color: #888; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 10px; display: block; font-weight: bold;}
        .mode-box { background: #252525; border: 1px solid #333; border-radius: 10px; padding: 12px; margin-bottom: 8px; cursor: pointer; transition: 0.3s; }
        .mode-box.active { border-color: #00e676; background: #1b2e20; }
        .btn-apply { width: 100%; padding: 14px; background: #00c853; color: white; border: none; border-radius: 8px; font-weight: bold; cursor: pointer; margin-top: auto; }
        .btn-stop { background: #ff9100; display: none; }
        .latest-log { background: #1e1e1e; border: 1px solid #333; border-radius: 8px; padding: 10px; text-align: center; font-size: 11px; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="sidebar">
        <div class="status-box" id="status">Kết nối Camera...</div>
        <label class="section-title">CHỌN MÔ HÌNH AI</label>
        <label class="mode-box active" id="box-color-cloud"><div class="mode-title"><input type="radio" name="mainTask" value="colorCloud" checked onchange="updateUI()"> Màu Sắc</div></label>
        <label class="mode-box" id="box-shape-cloud"><div class="mode-title"><input type="radio" name="mainTask" value="shapeCloud" onchange="updateUI()"> Hình Dạng</div></label>
        <label class="mode-box" id="box-local"><div class="mode-title"><input type="radio" name="mainTask" value="local" onchange="updateUI()"> Mô hình AI tự train</div></label>
        
        <div id="upload-section" style="display: none; padding: 10px; background: #252525; border-radius: 8px; margin-top: 5px;">
            <input type="file" id="files" multiple accept=".js,.wasm" style="font-size: 10px;" onchange="loadFiles(event)">
        </div>

        <div style="background:#252525; padding: 12px; border-radius: 8px; margin-top: 10px;">
            <label style="font-size: 11px; color: #ff9100;">ĐỘ TỰ TIN: <span id="thresh-val">50%</span></label>
            <input type="range" id="threshold-slider" style="width:100%" min="10" max="99" value="50" oninput="updateThresh(this.value)">
        </div>

        <button class="btn-apply" id="btn-start" onclick="startAI()">CHẠY AI</button>
        <button class="btn-apply btn-stop" id="btn-stop" onclick="stopAI()">DỪNG AI</button>
        <div class="latest-log" id="log-box">Sẵn sàng!</div>
    </div>

    <div class="main-content">
        <div class="camera-section">
            <img id="stream" src="" crossorigin="anonymous">
            <canvas id="canvas"></canvas>
        </div>

        <label class="section-title">⚙️ CẤU HÌNH THÔNG SỐ CAMERA</label>
        <div class="settings-grid">
            <div class="card">
                <h3>Cơ bản</h3>
                <div class="cam-control">
                    <span>Độ phân giải</span>
                    <select id="framesize" onchange="updateCam('framesize', this.value)">
                        <option value="9">SVGA (800x600)</option>
                        <option value="6">VGA (640x480)</option>
                        <option value="5" selected>CIF (400x296)</option>
                        <option value="4">QVGA (320x240)</option>
                    </select>
                </div>
                <div class="cam-control">
                    <span>Chất lượng</span>
                    <input type="range" id="quality" min="10" max="63" value="12" onchange="updateCam('quality', this.value)">
                </div>
            </div>
            <div class="card">
                <h3>Hình ảnh</h3>
                <div class="cam-control"><span>Độ sáng</span><input type="range" min="-2" max="2" value="0" onchange="updateCam('brightness', this.value)"></div>
                <div class="cam-control"><span>Tương phản</span><input type="range" min="-2" max="2" value="0" onchange="updateCam('contrast', this.value)"></div>
            </div>
            <div class="card">
                <h3>Xoay & Lật</h3>
                <div class="cam-control"><span>Lật dọc</span><input type="checkbox" onchange="updateCam('vflip', this.checked ? 1 : 0)"></div>
                <div class="cam-control"><span>Lật ngang</span><input type="checkbox" onchange="updateCam('hmirror', this.checked ? 1 : 0)"></div>
            </div>
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

        window.onload = () => { streamImg.src = `http://${location.hostname}:81/stream`; };
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

        // ĐÃ SỬA: Xóa sạch bộ nhớ biến khi người dùng tải file mới lên
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

        async function startAI() {
            const v = document.querySelector('input[name="mainTask"]:checked').value;
            
            if (v === "local" && (!jsStandalone || !jsRun || !wasmFile)) {
                writeLog("Lỗi: Bạn chưa chọn đủ 3 file!"); return;
            }

            statusEl.innerText = "NẠP MÔ HÌNH...";
            
            // ĐÃ SỬA: Phá hủy iframe cũ để ép trình duyệt xóa sạch não AI cũ
            let oldBox = document.getElementById('ai-sandbox');
            if(oldBox) oldBox.remove(); 
            let newBox = document.createElement('iframe'); 
            newBox.id = 'ai-sandbox'; newBox.style.display = 'none'; 
            document.body.appendChild(newBox); 
            const win = newBox.contentWindow;

            try {
                if (v === "colorCloud" || v === "shapeCloud") {
                    const s = (v === "colorCloud") ? URLS.color : URLS.shape;
                    const [w, c, r] = await Promise.all([fetch(s.wasm), fetch(s.core), fetch(s.run)]);
                    win.Module = { wasmBinary: await w.arrayBuffer() };
                    const s1 = win.document.createElement('script'); s1.textContent = await c.text(); win.document.head.appendChild(s1);
                    const s2 = win.document.createElement('script'); s2.textContent = await r.text(); win.document.head.appendChild(s2);
                    const s3 = win.document.createElement('script'); s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`; win.document.head.appendChild(s3);
                    classifier = await win.initAI();
                    activateAI();
                } else {
                    // ĐÃ SỬA: Phục hồi logic đọc và chạy Model File Nội bộ
                    const readerWasm = new FileReader();
                    readerWasm.onload = function() {
                        win.Module = { wasmBinary: readerWasm.result };
                        const readerCore = new FileReader();
                        readerCore.onload = function() {
                            const readerRun = new FileReader();
                            readerRun.onload = async function() {
                                const s1 = win.document.createElement('script'); s1.textContent = readerCore.result; win.document.head.appendChild(s1);
                                const s2 = win.document.createElement('script'); s2.textContent = readerRun.result; win.document.head.appendChild(s2);
                                const s3 = win.document.createElement('script'); 
                                s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`; 
                                win.document.head.appendChild(s3);
                                classifier = await win.initAI();
                                activateAI();
                            };
                            readerRun.readAsText(jsRun);
                        };
                        readerCore.readAsText(jsStandalone);
                    };
                    readerWasm.readAsArrayBuffer(wasmFile);
                }
            } catch (e) { statusEl.innerText = "LỖI NẠP AI"; stopAI(); }
        }

        function activateAI() {
            statusEl.innerText = "ĐANG CHẠY AI"; isRunning = true; 
            document.getElementById('btn-start').style.display='none'; 
            document.getElementById('btn-stop').style.display='block'; 
            processFrame();
        }

        function stopAI() { 
            isRunning = false; cancelAnimationFrame(aiInterval); 
            document.getElementById('btn-start').style.display='block'; 
            document.getElementById('btn-stop').style.display='none'; 
            statusEl.innerText = "TẠM DỪNG"; ctx.clearRect(0,0,canvas.width,canvas.height); 
        }

        async function processFrame() {
            if (!isRunning || !classifier) return;
            const size = 96; const hC = document.createElement('canvas'); hC.width = size; hC.height = size;
            hC.getContext('2d').drawImage(streamImg, 0, 0, size, size);
            const data = hC.getContext('2d').getImageData(0,0,size,size);
            let feat = []; for(let i=0; i<data.data.length; i+=4) feat.push((data.data[i]<<16)|(data.data[i+1]<<8)|data.data[i+2]);
            const res = classifier.classify(feat);
            canvas.width = streamImg.width; canvas.height = streamImg.height; ctx.clearRect(0,0,canvas.width,canvas.height);
            
            if(res.results) {
                res.results.forEach(b => {
                    if(b.value < currentThreshold) return;
                    const cx = (b.x + b.width/2) * (canvas.width/size); 
                    const cy = (b.y + b.height/2) * (canvas.height/size);
                    let label = b.label.toLowerCase();
                    let color = "#ff9100"; 
                    
                    if (label.includes("do") || label.includes("red")) {
                        color = "#ff1744"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke();
                    } else if (label.includes("xanh") || label.includes("green")) {
                        color = "#00e676"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke();
                    } else if (label.includes("tron") || label.includes("circle")) {
                        color = "#29b6f6"; ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2*Math.PI); ctx.fillStyle=color; ctx.fill(); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.stroke();
                    } else if (label.includes("vuong") || label.includes("square")) {
                        color = "#ab47bc"; ctx.fillStyle=color; ctx.fillRect(cx-12, cy-12, 24, 24); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.strokeRect(cx-12, cy-12, 24, 24);
                    } else {
                        ctx.fillStyle=color; ctx.fillRect(cx-12, cy-12, 24, 24); ctx.lineWidth=2; ctx.strokeStyle="white"; ctx.strokeRect(cx-12, cy-12, 24, 24);
                    }
                    
                    let p = Math.round(b.value * 100);
                    ctx.font = "bold 14px Arial"; ctx.lineWidth = 3; ctx.strokeStyle = "black";
                    ctx.strokeText(`${b.label} ${p}%`, cx - 15, cy - 20); 
                    ctx.fillStyle = "white"; ctx.fillText(`${b.label} ${p}%`, cx - 15, cy - 20);
                });
            }
            aiInterval = requestAnimationFrame(processFrame);
        }
    </script>
</body>
</html>
)rawliteral";