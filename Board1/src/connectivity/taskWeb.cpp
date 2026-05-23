#include "taskWeb.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>


WiFiServer server(80);
SystemState *webSysState;

// HELPERS
void saveWiFi(String ssid, String pass) {
  Preferences p;
  p.begin("wifi-config", false);
  p.putString("ssid", ssid);
  p.putString("pass", pass);
  p.end();
}

void clearWiFi() {
  Preferences p;
  p.begin("wifi-config", false);
  p.clear(); // Wipe saved data
  p.end();
}

// AP Mode
String getConfigHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>ESP32 Setup</title>
	<style>
		body { font-family: -apple-system, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; justify-content: center; align-items: center; margin: 0; padding: 20px; }
		.container { background: white; border-radius: 20px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); max-width: 400px; width: 100%; padding: 30px; text-align: center; }
		h1 { color: #333; margin-bottom: 5px; }
		p { color: #666; margin-bottom: 25px; }
		.btn { width: 100%; padding: 12px; border: none; border-radius: 10px; font-size: 16px; font-weight: 600; cursor: pointer; color: white; background: #667eea; margin-bottom: 15px; }
		.btn:disabled { opacity: 0.6; cursor: wait; }
		select, input { width: 100%; padding: 12px; margin-bottom: 15px; border: 2px solid #e0e0e0; border-radius: 10px; box-sizing: border-box; }
		.success { background: #d4edda; color: #155724; padding: 15px; border-radius: 10px; display: none; }
	</style>
</head>
<body>
	<div class="container">
		<h1>ESP32 Config</h1>
		<p>Connect to Internet</p>
		<button class="btn" id="scanBtn" onclick="scanWiFi()">Scan Networks</button>
		<form onsubmit="saveWiFi(event)">
			<select id="ssid" required><option value="">Select Network...</option></select>
			<input type="password" id="pass" placeholder="Password" required>
			<button type="submit" class="btn" id="saveBtn">Save & Connect</button>
		</form>
		<div class="success" id="successMsg">Saved! Restarting...</div>
	</div>
	<script>
		function scanWiFi() {
			var btn = document.getElementById('scanBtn');
			btn.disabled = true; btn.innerText = 'Scanning...';
			fetch('/scan').then(r => r.json()).then(data => {
				var sel = document.getElementById('ssid');
				sel.innerHTML = '<option value="">Select Network...</option>';
				data.forEach(n => {
					var opt = document.createElement('option');
					opt.value = n.ssid;
					opt.textContent = n.ssid + ' (' + n.rssi + ' dBm)';
					sel.appendChild(opt);
				});
				btn.disabled = false; btn.innerText = 'Scan Again';
			}).catch(e => { alert('Scan Failed'); btn.disabled = false; btn.innerText = 'Scan Again'; });
		}
		function saveWiFi(e) {
			e.preventDefault();
			var ssid = document.getElementById('ssid').value;
			var pass = document.getElementById('pass').value;
			var btn = document.getElementById('saveBtn');
			btn.disabled = true; btn.innerText = 'Saving...';
			fetch('/wifi?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass))
			.then(r => {
				document.getElementById('successMsg').style.display = 'block';
				setTimeout(() => location.reload(), 5000);
			});
		}
	</script>
</body>
</html>
)rawliteral";
}

// LITTLEFS SERVING HELPER
void serveFile(WiFiClient &client, const char *path, const char *contentType) {
  if (!LittleFS.exists(path)) {
    Serial.printf("[WEB] File not found: %s\n", path);
    client.println("HTTP/1.1 404 Not Found\r\nContent-Type: "
                   "text/plain\r\nConnection: close\r\n\r\nFile Not Found");
    return;
  }
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("[WEB] Failed to open file: %s\n", path);
    client.println(
        "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "
        "text/plain\r\nConnection: close\r\n\r\nFailed to Open File");
    return;
  }
  client.print("HTTP/1.1 200 OK\r\nContent-Type: ");
  client.print(contentType);
  client.print("\r\nConnection: close\r\nContent-Length: ");
  client.print(file.size());
  client.print("\r\n\r\n");

  uint8_t buffer[256];
  while (file.available()) {
    int bytesRead = file.read(buffer, sizeof(buffer));
    client.write(buffer, bytesRead);
  }
  file.close();
}

// REQUEST HANDLER
void handleRequest(WiFiClient &client, String req) {
  if (req.indexOf("GET /reset-wifi") >= 0) {
    Serial.println("[WEB] Reset requested. Clearing Credentials...");
    clearWiFi(); // Erase Flash
    client.println(
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nResetting...");
    client.stop();

    delay(1000);
    ESP.restart();
  }

  // API: Toggle LED 1
  else if (req.indexOf("GET /toggle1") >= 0) {
    if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
      webSysState->device1 = !webSysState->device1;
      xSemaphoreGive(webSysState->mutex);
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK");
  }
  // API: Toggle LED 2
  else if (req.indexOf("GET /toggle2") >= 0) {
    if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
      webSysState->device2 = !webSysState->device2;
      xSemaphoreGive(webSysState->mutex);
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK");
  }
  // API: Status JSON
  else if (req.indexOf("GET /status") >= 0) {
    float t = 0, h = 0;
    bool d1 = false, d2 = false;
    if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
      t = webSysState->temperature;
      h = webSysState->humidity;
      d1 = webSysState->device1;
      d2 = webSysState->device2;
      xSemaphoreGive(webSysState->mutex);
    }
    StaticJsonDocument<300> doc;
    doc["t"] = t;
    doc["h"] = h;
    doc["d1"] = d1;
    doc["d2"] = d2;

    client.println("HTTP/1.1 200 OK\r\nContent-Type: "
                   "application/json\r\nConnection: close\r\n");
    serializeJson(doc, client);
  }
  // API: Scan (Config Mode)
  else if (req.indexOf("GET /scan") >= 0) {
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    client.println("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    client.print("[");
    for (int i = 0; i < n; i++) {
      if (i > 0)
        client.print(",");
      client.print("{\"ssid\":\"");
      client.print(WiFi.SSID(i));
      client.print("\",\"rssi\":");
      client.print(WiFi.RSSI(i));
      client.print("}");
    }
    client.print("]");
  }
  // API: Save WiFi (Config Mode)
  else if (req.indexOf("GET /wifi?") >= 0) {
    int s1 = req.indexOf("ssid=") + 5;
    int s2 = req.indexOf("&pass=");
    int s3 = req.indexOf(" HTTP");
    if (s1 > 5 && s2 > 0) {
      String ssid = req.substring(s1, s2);
      String pass = req.substring(s2 + 6, s3);
      ssid.replace("%20", " ");
      pass.replace("%20", " ");
      saveWiFi(ssid, pass);
      client.println("HTTP/1.1 200 OK\r\n\r\nSaved");
      delay(500);
      ESP.restart();
    }
  }
  // Serve CSS from LittleFS
  else if (req.indexOf("GET /style.css") >= 0) {
    serveFile(client, "/style.css", "text/css");
  }
  // Serve JS from LittleFS
  else if (req.indexOf("GET /index.js") >= 0) {
    serveFile(client, "/index.js", "application/javascript");
  }
  // Serve HTML (Home)
  else {
    if (WiFi.status() == WL_CONNECTED) {
      serveFile(client, "/dashboard.html", "text/html");
    } else {
      client.println("HTTP/1.1 200 OK\r\nContent-Type: "
                     "text/html\r\nConnection: close\r\n");
      client.print(getConfigHTML());
    }
  }
  client.stop();
}

void taskWeb(void *pvParameters) {
  webSysState = (SystemState *)pvParameters;

  if (!LittleFS.begin(true)) {
    Serial.println("[WEB] LittleFS Mount Failed");
  } else {
    Serial.println("[WEB] LittleFS Mounted");
  }

  // Track previous state
  bool wasConnected = false;

  WiFi.setSleep(false);

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    wasConnected = true;
    Serial.print("[WEB] Online! Dashboard at: ");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    wasConnected = false;
    Serial.print("[WEB] Config Mode. Connect to IP: ");
    Serial.println(WiFi.softAPIP());
  }

  server.begin();

  while (1) {
    if (WiFi.status() == WL_CONNECTED && !wasConnected) {
      Serial.println("[WEB] Connection Established! Disabling AP...");
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      Serial.print("[WEB] Dashboard IP: ");
      Serial.println(WiFi.localIP());
      server.begin();
      wasConnected = true;
    } else if (WiFi.status() != WL_CONNECTED && wasConnected) {
      Serial.println("[WEB] Connection Lost! Enabling AP...");
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(WIFI_SSID, WIFI_PASS);
      Serial.print("[WEB] Config IP: ");
      Serial.println(WiFi.softAPIP());
      server.begin();
      wasConnected = false;
    }

    WiFiClient client = server.available();
    if (client) {
      String req = "";
      unsigned long timeout = millis() + 2000;
      while (client.connected() && millis() < timeout) {
        if (client.available()) {
          req += (char)client.read();
          if (req.endsWith("\r\n\r\n"))
            break;
        }
      }
      if (req.length() > 0)
        handleRequest(client, req);
      else
        client.stop();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}