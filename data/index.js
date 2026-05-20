// Check which page we are on
const isDashboard = document.getElementById('dashboard-page') !== null;
const isConfig = document.getElementById('config-page') !== null;


// Dashboard Logic
if (isDashboard) {
    function updateData() {
        fetch('/status')
            .then(r => r.json())
            .then(d => {
                document.getElementById('temp').innerText = d.t.toFixed(1);
                document.getElementById('hum').innerText = d.h.toFixed(1);
                updateBtn('btn1', d.d1);
                updateBtn('btn2', d.d2);
            })
            .catch(e => console.error('Error fetching status:', e));
    }

    function updateBtn(id, state) {
        const btn = document.getElementById(id);
        if(state) { 
            btn.className = 'btn btn-off'; 
            btn.innerText = 'Turn OFF'; 
        } else { 
            btn.className = 'btn btn-on'; 
            btn.innerText = 'Turn ON'; 
        }
    }

    window.toggle = function(id) { 
        fetch('/toggle'+id)
            .then(updateData)
            .catch(e => console.error('Error toggling:', e));
    };
    
    window.resetWifi = function() {
        if(confirm("Forget WiFi credentials and return to AP Mode?")) {
            fetch('/reset-wifi').then(() => {
                alert("Resetting... Connect to 'ESP32_Group_AP' in 10 seconds.");
            });
        }
    };

    // Auto update every 2 seconds
    setInterval(updateData, 2000);
    // Initial fetch
    window.onload = updateData;
}

// ==============================
// Config Page Logic
// ==============================
if (isConfig) {
    window.scanWiFi = function() {
        const btn = document.getElementById('scanBtn');
        btn.disabled = true; 
        btn.innerText = 'Scanning...';
        
        fetch('/scan')
            .then(r => r.json())
            .then(data => {
                const sel = document.getElementById('ssid');
                sel.innerHTML = '<option value="">Select Network...</option>';
                data.forEach(n => {
                    const opt = document.createElement('option');
                    opt.value = n.ssid;
                    opt.textContent = `${n.ssid} (${n.rssi} dBm)`;
                    sel.appendChild(opt);
                });
                btn.disabled = false; 
                btn.innerText = 'Scan Again';
            })
            .catch(e => { 
                console.error('Scan Failed:', e);
                alert('Scan Failed. Please try again.'); 
                btn.disabled = false; 
                btn.innerText = 'Scan Again'; 
            });
    };

    window.saveWiFi = function(e) {
        e.preventDefault();
        const ssid = document.getElementById('ssid').value;
        const pass = document.getElementById('pass').value;
        const btn = document.getElementById('saveBtn');
        
        btn.disabled = true; 
        btn.innerText = 'Saving...';
        
        fetch(`/wifi?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`)
            .then(r => {
                document.getElementById('successMsg').style.display = 'block';
                setTimeout(() => location.reload(), 5000);
            })
            .catch(e => {
                console.error('Save failed:', e);
                btn.disabled = false;
                btn.innerText = 'Save & Connect';
                alert('Failed to save credentials.');
            });
    };
}
