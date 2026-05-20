#include "taskWeb.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

WebServer server(80);
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

void setupRoutes() {
	// === API: RESET WIFI ===
	server.on("/reset-wifi", HTTP_GET, []() {
		Serial.println("[WEB] Reset requested. Clearing Credentials...");
		clearWiFi(); // Erase Flash  
		server.send(200, "text/plain", "Resetting...");
			
		delay(1000);
		ESP.restart(); // Reboot -> Main.cpp will see no password -> Start AP
	});

	// API: Toggle LED 1
	server.on("/toggle1", HTTP_GET, []() {
		if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
			webSysState->device1State = !webSysState->device1State;
			xSemaphoreGive(webSysState->mutex);
		}
		server.send(200, "text/plain", "OK");
	});

	// API: Toggle LED 2
	server.on("/toggle2", HTTP_GET, []() {
		if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
			webSysState->device2State = !webSysState->device2State;
			xSemaphoreGive(webSysState->mutex);
		}
		server.send(200, "text/plain", "OK");
	});

	// API: Status JSON
	server.on("/status", HTTP_GET, []() {
		float t = 0, h = 0; bool d1 = false, d2 = false;
		if (xSemaphoreTake(webSysState->mutex, portMAX_DELAY) == pdTRUE) {
			t = webSysState->temperature; h = webSysState->humidity;
			d1 = webSysState->device1State; d2 = webSysState->device2State;
			xSemaphoreGive(webSysState->mutex);
		}
		StaticJsonDocument<300> doc;
		doc["t"] = t; doc["h"] = h; doc["d1"] = d1; doc["d2"] = d2;

		String response;
		serializeJson(doc, response);
		server.send(200, "application/json", response);
	});

	// API: Scan (Config Mode)
	server.on("/scan", HTTP_GET, []() {
		WiFi.disconnect(); 
		delay(100);
		int n = WiFi.scanNetworks();
		String json = "[";
		for (int i = 0; i < n; i++) {
			if (i > 0) json += ",";
			json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
		}
		json += "]";
		server.send(200, "application/json", json);
	});

	// API: Save WiFi (Config Mode)
	server.on("/wifi", HTTP_GET, []() {
		if (server.hasArg("ssid") && server.hasArg("pass")) {
			String ssid = server.arg("ssid");
			String pass = server.arg("pass");
			
			ssid.replace("%20", " ");
			pass.replace("%20", " ");

			saveWiFi(ssid, pass);
			server.send(200, "text/plain", "Saved");
			delay(500);
			ESP.restart();
		} else {
			server.send(400, "text/plain", "Missing args");
		}
	});

	// Serve HTML (Home)
	server.on("/", HTTP_GET, []() {
		if (WiFi.status() == WL_CONNECTED) {
			File file = LittleFS.open("/index.html", "r");
			if (!file) {
				server.send(404, "text/plain", "Dashboard HTML not found in LittleFS");
				return;
			}
			server.streamFile(file, "text/html");
			file.close();
		} else {
			File file = LittleFS.open("/config.html", "r");
			if (!file) {
				server.send(404, "text/plain", "Config HTML not found in LittleFS");
				return;
			}
			server.streamFile(file, "text/html");
			file.close();
		}
	});

	// Serve static assets
	server.serveStatic("/style.css", LittleFS, "/style.css");
	server.serveStatic("/index.js", LittleFS, "/index.js");
}

void taskWeb(void *pvParameters) 
{
	webSysState = (SystemState *)pvParameters;
	
	// Mount LittleFS
	if(!LittleFS.begin(true)){
		Serial.println("[WEB] Error mounting LittleFS");
	} else {
		Serial.println("[WEB] LittleFS mounted successfully");
	}

	// Track previous state
	bool wasConnected = false;

	if (WiFi.status() == WL_CONNECTED) {
		WiFi.mode(WIFI_STA); 
		wasConnected = true;
		Serial.print("[WEB] Online! Dashboard at: ");
		Serial.println(WiFi.localIP());
	} else {
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP(WIFI_SSID, WIFI_PASS);
		wasConnected = false;
		Serial.print("[WEB] Config Mode. Connect to IP: ");
		Serial.println(WiFi.softAPIP());
	}

	setupRoutes();
	server.begin();

	while (1) {
		if (WiFi.status() == WL_CONNECTED && !wasConnected) {
			Serial.println("[WEB] Connection Established! Disabling AP...");
			WiFi.mode(WIFI_STA); 
			Serial.print("[WEB] Dashboard IP: "); Serial.println(WiFi.localIP());
			wasConnected = true;
		}
		else if (WiFi.status() != WL_CONNECTED && wasConnected) 
		{
			Serial.println("[WEB] Connection Lost! Enabling AP...");
			WiFi.mode(WIFI_AP_STA); 
			WiFi.softAP(WIFI_SSID, WIFI_PASS);
			Serial.print("[WEB] Config IP: "); Serial.println(WiFi.softAPIP());
			wasConnected = false;
		}

		server.handleClient();
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}