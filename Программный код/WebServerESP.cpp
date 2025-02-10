#include "WebServerESP.h"
#include "WebPage.h"
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "NALIVATOR_3000";
const char *password = "123456789";
WebServer server(80);
String lastCommand = "";

void handleRoot() {
    server.send(200, "text/html", PAGE_HTML);
}

void handleCommand() {
    if (server.hasArg("cmd")) {
        lastCommand = server.arg("cmd");
        Serial.println("Получена команда: " + lastCommand);
        server.send(200, "text/plain", "Команда принята: " + lastCommand);
    } else {
        server.send(400, "text/plain", "Ошибка запроса");
    }
}

void startServer() {
    WiFi.softAP(ssid, password);
    
    server.on("/", handleRoot);
    server.on("/command", HTTP_GET, handleCommand);
    server.begin();
    Serial.println("Сервер запущен. Подключитесь к " + String(ssid));
}

String waitForCommand() {
    lastCommand = "";
    while (lastCommand == "") {
        server.handleClient();
        delay(10);
    }
    return lastCommand;
}
