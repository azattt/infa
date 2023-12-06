#include "LittleFS.h"
#include <ESP8266TrueRandom.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Пути файлов
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_5 = "token";

String ssid;
String pass;
String ip;
String gateway;
String token;

AsyncWebServer server(80); // Инициализируем сервер
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0); // Маска

unsigned long previousMillis = 0;
const long interval = 10000; // Сколько ждем перед открытием портала
const int ledPin = 2;
String ledState;

const unsigned long BOT_MTBS = 100;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
unsigned long bot_lasttime;
UniversalTelegramBot bot;

int getCarbonSensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        return ESP8266TrueRandom.random(300, 1200);
    }
    if (sensor_id == 1)
    {
        return ESP8266TrueRandom.random(300, 1200);
    }
    return INT_MAX;
}
int getHumiditySensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        return ESP8266TrueRandom.random(0, 100);
    }
    if (sensor_id == 1)
    {
        return ESP8266TrueRandom.random(0, 100);
    }
    return INT_MAX;
}
int getTemperatureSensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        return ESP8266TrueRandom.random(-20, 40);
    }
    if (sensor_id == 1)
    {
        return ESP8266TrueRandom.random(-20, 40);
    }
    return INT_MAX;
}

// void handleNewMessages(int numNewMessages)
// {
//     Serial.printf("new message");
//     for (int i = 0; i < numNewMessages; i++)
//     {
//         String chat_id = bot.messages[i].chat_id;
//         String text = bot.messages[i].text;
//         String from_name = bot.messages[i].from_name;
//         if (from_name == "")
//             from_name = "Guest";
//         if (text == "/start")
//         {
//             bot.sendMessageWithReplyKeyboard(chat_id, "Меню", "", "[[\"Текущие показатели\"], [\"Эталонные
//             показатели\"], [\"Обратная связь\"]]");
//         }
//         if (text == "Текущие показатели")
//         {
//             bot.sendMessage(chat_id,
//                 String("Температура: \n") +
//                 String("    Первый датчик: ") + String(getTemperatureSensorInfo(0)) + String(" °C\n") +
//                 String("    Второй датчик: ") + String(getTemperatureSensorInfo(1)) + String(" °C\n") +
//                 String("Концентрация CO2: \n") +
//                 String("    Первый датчик: ") + String(getCarbonSensorInfo(0)) + String(" ppm\n") +
//                 String("    Второй датчик: ") + String(getCarbonSensorInfo(1)) + String(" ppm\n") +
//                 String("Влажность: \n") +
//                 String("    Первый датчик: ") + String(getHumiditySensorInfo(0)) + String("%\n") +
//                 String("    Второй датчик: ") + String(getHumiditySensorInfo(1)) + String("%\n")
//             );
//         }
//         // if (text == "Обратная связь")
//         // {
//         //     bot.sendMessageWithReplyKeyboard(chat_id, "Обратная связь", "", "[[{\"text\": \"Отзыв\", \"url\":
//         \"https://google.com\"}], [{\"text\": \"Тех. поддержка\", \"url\": \"https://yandex.ru\"}]]");
//         // }
//     }
// }

String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return String();
    }

    String fileContent;
    while (file.available())
    {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{ // Функция записи файла в spiffs
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- frite failed");
    }
}

// Функция инициализации wifi
bool initWiFi()
{
    if (ssid == "" || ip == "" || token == "")
    {
        Serial.println("Undefined SSID or IP address.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    localIP.fromString(ip.c_str());
    localGateway.fromString(gateway.c_str());

    if (!WiFi.config(localIP, localGateway, subnet))
    {
        Serial.println("STA Failed to configure");
        return false;
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;

    while (WiFi.status() != WL_CONNECTED)
    {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            Serial.println("Failed to connect.");
            return false;
        }
    }

    Serial.println(WiFi.localIP());
    return true;
}

void setup()
{
    Serial.begin(74800);
    Serial.println();
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount LittleFS");
    }

    pinMode(ledPin, OUTPUT); // Светодиод на выход
    digitalWrite(ledPin, LOW);

    // Загружаем в переменные данные из LitteFS

    ssid = LittleFS.open(ssidPath, "r").readString();
    pass = LittleFS.open(passPath, "r").readString();
    ip = LittleFS.open(ip, "r").readString();
    gateway = LittleFS.open(gatewayPath, "r").readString();
    if (initWiFi())
    {
        // Открываем успешную страницу index.html
        server.on("/", HTTP_GET,
                  [](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.html", "text/html"); });
        server.serveStatic("/", LittleFS, "/");

        server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) { // Подключаем стили
            request->send(LittleFS, "/bootstrap.min.css", "text/css");
        });
        server.begin();
    }
    else
    {
        Serial.println("Setting AP (Access Point)"); // Раздаем точку доступа
        WiFi.softAP("ESP-CONNECT", NULL); // Создаем открытую точку доступа с именем ESP-CONNECT

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { // Запускаем сервер с формой конфигурации
            request->send(LittleFS, "/conf.html", "text/html");
        });

        server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) { // Подключаем стили
            request->send(LittleFS, "/bootstrap.min.css", "text/css");
        });

        server.serveStatic("/", LittleFS, "/");

        // Получаем данные из формы, если пришел запрос
        server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
            int params = request->params();
            for (int i = 0; i < params; i++)
            {
                AsyncWebParameter *p = request->getParam(i);
                if (p->isPost())
                {
                    if (p->name() == PARAM_INPUT_1)
                    { // Получаем имя сети из формы
                        ssid = p->value().c_str();
                        Serial.print("SSID set to: ");
                        Serial.println(ssid);
                        // Write file to save value
                        writeFile(LittleFS, ssidPath, ssid.c_str());
                    }
                    if (p->name() == PARAM_INPUT_2)
                    { // Получаем пароль из формы
                        pass = p->value().c_str();
                        Serial.print("Password set to: ");
                        Serial.println(pass);
                        // Write file to save value
                        writeFile(LittleFS, passPath, pass.c_str());
                    }
                    if (p->name() == PARAM_INPUT_3)
                    { // Получаем POST запрос про IP
                        ip = p->value().c_str();
                        Serial.print("IP Address set to: ");
                        Serial.println(ip);
                        // Write file to save value
                        writeFile(LittleFS, ipPath, ip.c_str());
                    }
                    if (p->name() == PARAM_INPUT_4)
                    { // Получаем POST запрос про Gateway путь
                        gateway = p->value().c_str();
                        Serial.print("Gateway set to: ");
                        Serial.println(gateway);
                        // Write file to save value
                        writeFile(LittleFS, gatewayPath, gateway.c_str());
                    }
                }
            }
            request->send(200, "text/plain", "Успешно, esp перезагрузиться и получит адрес:" + ip);
            delay(3000);
            ESP.restart(); // Перезагружаем esp
        });
        server.begin(); // Запускаем сервер в любом случае
    }

    randomSeed(analogRead(0));
    configTime(0, 0, "pool.ntp.org");
    secured_client.setTrustAnchors(&cert);

    UniversalTelegramBot bot(token, secured_client);

    // time_t now = time(nullptr);
    // while (now < 24 * 3600)
    // {
    //     delay(100);
    //     now = time(nullptr);
    // }
}

void loop()
{
    if (millis() - bot_lasttime > BOT_MTBS)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages)
        {

            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        bot_lasttime = millis();
    }
}