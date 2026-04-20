const char* ai_dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>AI Cam Mini - FOMO Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; margin: 0; padding: 0; height: 100vh; overflow: hidden; display: flex; color: #eee; }
        .sidebar { width: 340px; background: #252525; padding: 15px; display: flex; flex-direction: column; gap: 12px; border-right: 1px solid #333; box-sizing: border-box; overflow-y: auto;}
        .status-box { padding: 10px; border-radius: 6px; background: #333; text-align: center; font-size: 13px; font-weight: bold; border-left: 4px solid #6200ea; }
        label { font-size: 11px; color: #888; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 3px; display: block;}
        
        .mode-box { background: #1e1e1e; border: 1px solid #444; border-radius: 8px; padding: 12px; margin-bottom: 8px; cursor: pointer; transition: 0.3s; }
        .mode-box:hover { border-color: #ff9100; }
        .mode-box.active { border-color: #00e676; background: #1b2e20; }
        .mode-title { font-size: 14px; font-weight: bold; color: white; display: flex; align-items: center; gap: 10px;}
        .mode-desc { font-size: 11px; color: #aaa; margin-top: 4px; margin-left: 26px;}
        input[type="radio"] { accent-color: #00e676; transform: scale(1.2); cursor: pointer;}

        .slider-container { background: #333; padding: 10px; border-radius: 6px;}
        .slider-container input[type="range"] { width: 100%; margin-top: 8px; cursor: pointer; accent-color: #ff9100;}
        .btn-apply { width: 100%; padding: 12px; background: #00c853; color: white; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; margin-top: 5px; transition: 0.3s; }
        .btn-apply:hover { background: #009624; }
        .btn-stop { background: #ff9100; display: none; }
        .btn-stop:hover { background: #e65100; }
        
        .camera-section { flex: 1; position: relative; background: #000; display: flex; justify-content: center; align-items: center; }
        #stream { height: 100%; width: 100%; object-fit: contain; }
        #canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; }
        .latest-log { background: #1e1e1e; border: 1px solid #444; border-radius: 6px; padding: 12px; text-align: center; font-size: 12px; margin-top: auto; }
    </style>
</head>
<body>
    <div class="sidebar">
        <div class="status-box" id="status" style="border-color: #ff9100;">Kết nối Camera...</div>
        <div>
            <label>CHỌN MÔ HÌNH VÀ NGUỒN</label>
            
            <label class="mode-box active" id="box-color-cloud">
                <div class="mode-title">
                    <input type="radio" name="mainTask" id="taskColorCloud" value="colorCloud" checked onchange="updateUI()"> 
                    Nhận diện Màu (Đỏ / Xanh)
                </div>
                <div class="mode-desc">Kéo AI Màu sắc xuyên tường lửa.</div>
            </label>

            <label class="mode-box" id="box-shape-cloud">
                <div class="mode-title">
                    <input type="radio" name="mainTask" id="taskShapeCloud" value="shapeCloud" onchange="updateUI()"> 
                    Nhận diện Hình (Tròn / Vuông)
                </div>
                <div class="mode-desc">Kéo AI Hình dạng xuyên tường lửa.</div>
            </label>

            <label class="mode-box" id="box-local">
                <div class="mode-title">
                    <input type="radio" name="mainTask" id="taskLocal" value="local" onchange="updateUI()"> 
                    Model AI tự tạo 
                </div>
                <div class="mode-desc">Dùng 3 file từ máy tính của bạn.</div>
            </label>

            <div id="upload-section" style="display: none; margin-bottom: 10px;">
                <div style="border: 2px dashed #ff9100; padding: 12px; text-align: center; cursor: pointer; font-size: 13px;" onclick="document.getElementById('files').click()">
                    Bấm để chọn 3 file AI của bạn
                    <input type="file" id="files" multiple accept=".js,.wasm" style="display:none" onchange="loadFiles(event)">
                </div>
                <div id="file-list" style="font-size: 11px; color: #00e676; margin-top: 5px; text-align: center;">Chưa có file.</div>
            </div>

            <div class="slider-container">
                <label style="margin: 0; color: #ff9100; font-weight: bold;">ĐỘ TỰ TIN: <span id="thresh-val" style="color: white; font-size: 14px;">50%</span></label>
                <input type="range" id="threshold-slider" min="10" max="99" value="50" oninput="updateThresh(this.value)">
            </div>
            
            <button class="btn-apply" id="btn-start" onclick="startAI()">🚀 KHỞI ĐỘNG AI</button>
            <button class="btn-apply btn-stop" id="btn-stop" onclick="stopAI()">🛑 TẠM DỪNG AI</button>
        </div>
        
        <div class="latest-log" id="log-box">
            <span style="color:#00b0ff; font-weight:bold;">Hệ thống sẵn sàng!</span>
        </div>
    </div>

    <div class="camera-section">
        <img id="stream" src="" crossorigin="anonymous" alt="Camera Stream">
        <canvas id="canvas"></canvas>
    </div>

    <iframe id="ai-sandbox" style="display:none;"></iframe>

    <script>
        const URLS = {
            color: {
                wasm: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/edge-impulse-standalone.wasm",
                core: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/edge-impulse-standalone.js",
                run:  "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Color_Green_Red/main/run-impulse.js"
            },
            shape: {
                wasm: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/edge-impulse-standalone.wasm",
                core: "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/edge-impulse-standalone.js",
                run:  "https://raw.githubusercontent.com/thanhphuc2002ct-cmyk/AI_Phanbiet_Tron_Vuong/refs/heads/main/run-impulse.js"
            }
        };

        const streamImg = document.getElementById('stream');
        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        const statusEl = document.getElementById('status');
        const logBox = document.getElementById('log-box');
        
        let jsStandalone, jsRun, wasmFile;
        let classifier = null, isRunning = false;
        let aiInterval, scriptsLoaded = false;
        let lastTask = "";

        window.onload = () => {
            const host = document.location.hostname;
            streamImg.src = `http://${host}:81/stream`;
            streamImg.onload = () => { if(!isRunning) { writeLog("Camera OK.", "#00e676"); statusEl.innerText = "SẴN SÀNG"; }};
        };

        function writeLog(msg, color = "#00b0ff") { logBox.innerHTML = `<span style="color:${color}; font-weight:bold;">${msg}</span>`; }
        function updateThresh(val) { document.getElementById('thresh-val').innerText = val + "%"; currentThreshold = parseInt(val) / 100.0; }
        let currentThreshold = 0.5;

        function updateUI() {
            const val = document.querySelector('input[name="mainTask"]:checked').value;
            document.querySelectorAll('.mode-box').forEach(el => el.classList.remove('active'));
            document.getElementById('upload-section').style.display = 'none';

            if (val === "colorCloud") document.getElementById('box-color-cloud').classList.add('active');
            else if (val === "shapeCloud") document.getElementById('box-shape-cloud').classList.add('active');
            else if (val === "local") {
                document.getElementById('box-local').classList.add('active');
                document.getElementById('upload-section').style.display = 'block';
            }
        }

        function loadFiles(e) {
            jsStandalone = null; jsRun = null; wasmFile = null;
            Array.from(e.target.files).forEach(f => {
                if(f.name === 'edge-impulse-standalone.js') jsStandalone = f;
                else if(f.name === 'run-impulse.js') jsRun = f;
                else if(f.name.endsWith('.wasm')) wasmFile = f;
            });
            if(jsStandalone && jsRun && wasmFile) {
                document.getElementById('file-list').innerHTML = `✅ Đã chọn 3 file (${Math.round(wasmFile.size/1024)}KB)`;
                writeLog("Sẵn sàng nạp file Local.", "#ff9100");
            }
        }

        function createSandbox() {
            if (classifier) classifier = null;
            let oldBox = document.getElementById('ai-sandbox');
            if (oldBox) { oldBox.src = "about:blank"; oldBox.remove(); }
            let newBox = document.createElement('iframe');
            newBox.id = 'ai-sandbox'; newBox.style.display = 'none';
            document.body.appendChild(newBox);
            return newBox.contentWindow;
        }

        async function startAI() {
            const task = document.querySelector('input[name="mainTask"]:checked').value;

            if (task === "local" && (!jsStandalone || !jsRun || !wasmFile)) {
                writeLog("Lỗi: Bạn chưa chọn đủ 3 file!", "#ff1744");
                return;
            }

            if (scriptsLoaded && task === lastTask && isRunning === false) {
                writeLog("▶ Tiếp tục chạy...", "#00e676");
                activateUI();
                isRunning = true;
                processFrame();
                return;
            }

            document.getElementById('btn-start').style.display = 'none'; 
            document.getElementById('btn-stop').style.display = 'block';
            statusEl.innerText = "ĐANG NẠP MÔ HÌNH...";
            statusEl.style.borderColor = "#00b0ff";
            
            scriptsLoaded = false; 
            lastTask = task;

            const win = createSandbox();
            const doc = win.document;
            const cb = "?v=" + new Date().getTime(); 

            try {
                if (task === "colorCloud" || task === "shapeCloud") {
                    writeLog("Kéo dữ liệu từ GitHub...", "#ffeb3b");
                    const set = (task === "colorCloud") ? URLS.color : URLS.shape;
                    const [w, c, r] = await Promise.all([fetch(set.wasm + cb), fetch(set.core + cb), fetch(set.run + cb)]);
                    
                    win.Module = { wasmBinary: await w.arrayBuffer() };
                    const s1 = doc.createElement('script'); s1.textContent = await c.text(); doc.head.appendChild(s1);
                    const s2 = doc.createElement('script'); s2.textContent = await r.text(); doc.head.appendChild(s2);
                    
                    const s3 = doc.createElement('script');
                    s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`;
                    doc.head.appendChild(s3);
                    
                    writeLog("Đánh thức bộ não AI...", "#ff9100");
                    classifier = await win.initAI();
                    activateAI();

                } else {
                    writeLog("Đang nạp file Local...", "#ffeb3b");
                    const readerWasm = new FileReader();
                    readerWasm.onload = function() {
                        win.Module = { wasmBinary: readerWasm.result };
                        
                        const readerCore = new FileReader();
                        readerCore.onload = function() {
                            const readerRun = new FileReader();
                            readerRun.onload = async function() {
                                try {
                                    const s1 = doc.createElement('script'); s1.textContent = readerCore.result; doc.head.appendChild(s1);
                                    const s2 = doc.createElement('script'); s2.textContent = readerRun.result; doc.head.appendChild(s2);

                                    const s3 = doc.createElement('script');
                                    s3.textContent = `window.initAI = async function() { window.classifier = new EdgeImpulseClassifier(); await window.classifier.init(); return window.classifier; }`;
                                    doc.head.appendChild(s3);
                                    
                                    writeLog("Đánh thức bộ não AI...", "#ff9100");
                                    classifier = await win.initAI();
                                    activateAI();
                                } catch(err) {
                                    writeLog(`Lỗi khởi tạo: ${err.message}`, "#ff1744"); stopAI();
                                }
                            };
                            readerRun.readAsText(jsRun);
                        };
                        readerCore.readAsText(jsStandalone);
                    };
                    readerWasm.readAsArrayBuffer(wasmFile);
                }
            } catch (err) { writeLog(`Lỗi nạp file: ${err.message}`, "#ff1744"); stopAI(); }
        }

        function activateAI() {
            scriptsLoaded = true;
            statusEl.innerText = "ĐANG CHẠY AI";
            statusEl.style.borderColor = "#00c853";
            writeLog("✅ AI Sẵn sàng!", "#00c853");
            isRunning = true;
            processFrame();
        }

        function activateUI() {
            document.getElementById('btn-start').style.display = 'none'; 
            document.getElementById('btn-stop').style.display = 'block';
            statusEl.style.borderColor = "#00c853";
        }

        function stopAI() {
            isRunning = false;
            if(aiInterval) cancelAnimationFrame(aiInterval);
            document.getElementById('btn-start').style.display = 'block';
            document.getElementById('btn-stop').style.display = 'none';
            statusEl.innerText = "ĐÃ TẠM DỪNG";
            statusEl.style.borderColor = "#ff9100";
            
            setTimeout(() => { ctx.clearRect(0, 0, canvas.width, canvas.height); }, 50);
            writeLog("⏸ Đã tạm dừng và dọn sạch màn hình.", "#ff9100");
        }

        async function processFrame() {
            if (!isRunning || !classifier || !streamImg.complete || streamImg.naturalWidth === 0) return;
            try {
                const camW = streamImg.width; const camH = streamImg.height; const aiSize = 96; 
                const hiddenCanvas = document.createElement('canvas');
                hiddenCanvas.width = aiSize; hiddenCanvas.height = aiSize;
                const hiddenCtx = hiddenCanvas.getContext('2d');
                hiddenCtx.drawImage(streamImg, 0, 0, aiSize, aiSize);
                const imgData = hiddenCtx.getImageData(0, 0, aiSize, aiSize);

                let features = [];
                for (let i = 0; i < imgData.data.length; i += 4) {
                    // CỐ ĐỊNH CHUẨN ĐÓNG GÓI MÀU CỦA EDGE IMPULSE (Cho tất cả các loại Model)
                    features.push((imgData.data[i] << 16) | (imgData.data[i + 1] << 8) | imgData.data[i + 2]);
                }

                const res = classifier.classify(features);
                canvas.width = camW; canvas.height = camH; ctx.clearRect(0, 0, camW, camH);

                if(res.results) {
                    const sX = camW / aiSize; const sY = camH / aiSize;

                    res.results.forEach(box => {
                        if(box.value < currentThreshold) return; 
                        const cx = (box.x + box.width / 2) * sX; 
                        const cy = (box.y + box.height / 2) * sY;
                        let label = box.label.toLowerCase();
                        let color = "#ff9100"; // Màu cam mặc định

                        // XỬ LÝ VẼ ĐA NĂNG: Không thèm quan tâm bạn bấm nút nào, quét trúng chữ gì thì vẽ màu đó!
                        if (label.includes("do") || label.includes("red")) {
                            color = "#ff1744"; // Đỏ
                            ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2 * Math.PI); ctx.fillStyle = color; ctx.fill();
                            ctx.lineWidth = 3; ctx.strokeStyle = "white"; ctx.stroke();
                        } else if (label.includes("xanh") || label.includes("green")) {
                            color = "#00e676"; // Xanh lá
                            ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2 * Math.PI); ctx.fillStyle = color; ctx.fill();
                            ctx.lineWidth = 3; ctx.strokeStyle = "white"; ctx.stroke();
                        } else if (label.includes("tron") || label.includes("circle")) {
                            color = "#29b6f6"; // Xanh dương cho Tròn
                            ctx.beginPath(); ctx.arc(cx, cy, 12, 0, 2 * Math.PI); ctx.fillStyle = color; ctx.fill();
                            ctx.lineWidth = 3; ctx.strokeStyle = "white"; ctx.stroke();
                        } else if (label.includes("vuong") || label.includes("square")) {
                            color = "#ab47bc"; // Tím cho Vuông
                            ctx.fillStyle = color; ctx.fillRect(cx-12, cy-12, 24, 24);
                            ctx.lineWidth = 3; ctx.strokeStyle = "white"; ctx.strokeRect(cx-12, cy-12, 24, 24);
                        } else {
                            // FALLBACK: Lỡ bạn up một model nhận diện con chó con mèo thì nó vẫn vẽ ô vuông màu cam
                            ctx.fillStyle = color; ctx.fillRect(cx-12, cy-12, 24, 24);
                            ctx.lineWidth = 3; ctx.strokeStyle = "white"; ctx.strokeRect(cx-12, cy-12, 24, 24);
                        }

                        // Vẽ % Độ tự tin
                        let p = Math.round(box.value * 100);
                        ctx.font = "bold 14px Arial"; ctx.lineWidth = 3; ctx.strokeStyle = "black";
                        ctx.strokeText(`${box.label} ${p}%`, cx - 15, cy - 20);
                        ctx.fillStyle = "white"; ctx.fillText(`${box.label} ${p}%`, cx - 15, cy - 20);
                    });
                }
                aiInterval = requestAnimationFrame(processFrame);
            } catch (e) { console.error(e); }
        }
    </script>
</body>
</html>
)rawliteral";