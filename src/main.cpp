#include "LittleFS.h"

#include <ESP8266WiFi.h>
#include <ESP8266TrueRandom.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>

bool wifi_need_reset = false;
constexpr int WIFI_RESET_PIN = FUNC_GPIO6;

// Пути файлов
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *tokenPath = "/token.txt";
const char *idPath = "/id.txt";

const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "token";
const char *PARAM_INPUT_4 = "ownerid";

String ssid;
String pass;
String token;
int owner_id;

AsyncWebServer server(80); // Инициализируем сервер


const unsigned long TIMEOUT = 30000;
const char TELEGRAM_CERTIFICATE_ROOT[] = R"=EOF=(
-----BEGIN CERTIFICATE-----
MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx
EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT
EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp
ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz
NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH
EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE
AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw
DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD
E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH
/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy
DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh
GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR
tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA
AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE
FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX
WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu
9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr
gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo
2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO
LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI
4uJEvlz36hz1
-----END CERTIFICATE-----
)=EOF=";
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
HTTPClient https;
unsigned long long bot_update_offset = 0;
unsigned long bot_lasttime;
enum States
{
    NORMAL,
    TEMPERATURE_DIALOG,
    HUMIDITY_DIALOG,
    CO2_DIALOG
};
States bot_state;

int carbon_sensor = 1000;
int temperature_sensor = 40;
int humidity_sensor = 10;

bool wifi_on = false;

int getCarbonSensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        // return ESP8266TrueRandom.random(300, 1200);
        // return readFile(LittleFS, "/co2.txt").toInt();
        return carbon_sensor;
    }
    if (sensor_id == 1)
    {
        // return ESP8266TrueRandom.random(300, 1200);
        // return readFile(LittleFS, "/co2.txt").toInt();
        return carbon_sensor;
    }
    return INT_MAX;
}
int getHumiditySensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        // return ESP8266TrueRandom.random(0, 100);
        // return readFile(LittleFS, "/humidity.txt").toInt();
        return humidity_sensor;
    }
    if (sensor_id == 1)
    {
        // return ESP8266TrueRandom.random(0, 100);
        // return readFile(LittleFS, "/humidity.txt").toInt();
        return humidity_sensor;
    }
    return INT_MAX;
}
int getTemperatureSensorInfo(int sensor_id)
{
    if (sensor_id == 0)
    {
        // return ESP8266TrueRandom.random(-20, 40);
        // return readFile(LittleFS, "/temperature.txt").toInt();
        return temperature_sensor;
    }
    if (sensor_id == 1)
    {
        // return ESP8266TrueRandom.random(-20, 40);
        // return readFile(LittleFS, "/temperature.txt").toInt();
        return temperature_sensor;
    }
    return INT_MAX;
}

// Нужно реализовать
void setHeaterRelay(bool turn_on)
{
    return;
}
void setACRelay(bool turn_on)
{
    return;
}

void setCarbonRelay(bool turn_on)
{
    return;
}
void setHumidiferRelay(bool turn_on)
{
    return;
}
void setDehumidiferRelay(bool turn_on)
{
    return;
}

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
    file.close();
    return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{ // Функция записи файла в spiffs
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, "w+");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.write(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}

void set_normal_mode()
{
    // Открываем успешную страницу index.html
    server.on("/", HTTP_GET,
              [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.serveStatic("/", LittleFS, "/");

    server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) { // Подключаем стили
        request->send(LittleFS, "/bootstrap.min.css", "text/css");
    });
    server.begin();
}

void set_ap()
{
    Serial.println("Setting AP (Access Point)"); // Раздаем точку доступа
    WiFi.mode(WIFI_AP);
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
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
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
                          { // Получаем POST запрос про токен бота
                              token = p->value().c_str();
                              Serial.print("Bot token set to: ");
                              Serial.println(token);
                              // Write file to save value
                              writeFile(LittleFS, tokenPath, token.c_str());
                          }
                        if (p->name() == PARAM_INPUT_4)
                          { // Получаем POST запрос про id пользователя
                              owner_id = p->value().toInt();
                              Serial.print("Owner id set to: ");
                              Serial.println(owner_id);
                              // Write file to save value
                              writeFile(LittleFS, idPath, String(owner_id).c_str());
                          }
                      }
                  }
                  request->send(200, "text/plain", "Успешно, esp перезагрузиться");
                  delay(3000);
                  ESP.restart(); });
    server.begin();
}

void reset_wifi()
{
    writeFile(LittleFS, ssidPath, "");
    writeFile(LittleFS, passPath, "");
    writeFile(LittleFS, tokenPath, "");
    writeFile(LittleFS, idPath, "");
}

void send_message(int id, String message, String keyboard = "", bool disable_notification = false)
{
    String query = String("https://api.telegram.org/bot") + token + String("/sendMessage?chat_id=") + String(id) + "&text=" + urlEncode(message);
    if (keyboard != "")
    {
        query += String("&reply_markup=") + urlEncode(String("{\"keyboard\":") + keyboard + String("}"));
    }
    if (disable_notification)
    {
        query += "&disable_notification=" + String(disable_notification);
    }
    if (https.begin(client, query))
    { // HTTPS

        Serial.print("[HTTPS] send_message GET...\n");
        Serial.println(query);
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] send_message GET... code: %d\n", httpCode);
            String payload = https.getString();
            Serial.println(payload);
        }
        else
        {
            Serial.printf("[HTTPS] send_message GET... failed, error: %d\n", httpCode);
        }

        https.end();
    }
    else
    {
        Serial.printf("[HTTPS] send_message Unable to connect\n");
    }
}

bool is_float(const char *s, float *dest)
{
    if (s == NULL)
    {
        return false;
    }
    char *endptr;
    *dest = (float)strtod(s, &endptr);
    if (s == endptr)
    {
        return false; // no conversion;
    }
    // Look at trailing text
    while (isspace((unsigned char)*endptr))
        endptr++;
    return *endptr == '\0';
}

void handle_update(JsonObject update)
{
    if (update.containsKey("message"))
    {
        if (update["message"].containsKey("text") && update["message"].containsKey("from"))
        {
            String text = update["message"]["text"];
            int user_id = update["message"]["from"]["id"];
            if (user_id != owner_id)
            {
                return;
            }
            Serial.printf("New message from user %d: %s\n", user_id, text.c_str());
            if (bot_state == States::NORMAL)
            {
                if (text == "/start" || text == "Меню")
                {
                    send_message(user_id, "Меню", "[[\"Текущие показатели\"], [\"Эталонные "
                                                  "показатели\"], [\"Обратная связь\"]]");
                }
                if (text == "Текущие показатели")
                {
                    send_message(user_id,
                                 String("Температура: \n") +
                                     String("    Первый датчик: ") + String(getTemperatureSensorInfo(0)) + String(" °C\n") +
                                     String("    Второй датчик: ") + String(getTemperatureSensorInfo(1)) + String(" °C\n") +
                                     String("Концентрация CO2: \n") +
                                     String("    Первый датчик: ") + String(getCarbonSensorInfo(0)) + String(" ppm\n") +
                                     String("    Второй датчик: ") + String(getCarbonSensorInfo(1)) + String(" ppm\n") +
                                     String("Влажность: \n") +
                                     String("    Первый датчик: ") + String(getHumiditySensorInfo(0)) + String("%\n") +
                                     String("    Второй датчик: ") + String(getHumiditySensorInfo(1)) + String("%\n"));
                }
                else if (text == "Обратная связь")
                {
                    send_message(user_id, "Обратная связь", "[[{\"text\": \"Отзыв\"}], [{\"text\": \"Тех. поддержка\"}], [{\"text\": \"Меню\"}]]");
                }
                else if (text == "Отзыв")
                {
                    send_message(user_id, "Прислать отзыв:\nhttps://docs.google.com/forms/d/e/1FAIpQLSdX8R3fnIVoyFC3hnJy9FiiVGOYRnxxnnUrrA2fFjA-bLbtVw/viewform?usp=sf_link");
                }
                else if (text == "Тех. поддержка")
                {
                    send_message(user_id, "Тех. поддержка:\nhttps://docs.google.com/forms/d/e/1FAIpQLSeJs2QQebE52jCf_coTEecWdY9rL6e57TnbOumcUU6sJc5NUA/viewform?usp=sf_link");
                }
                else if (text == "Эталонные показатели")
                {
                    String s;
                    s += "Эталонные показатели:\n";
                    s += "  Температура: " + readFile(LittleFS, "/temperature.txt") + " °C\n";
                    s += "  Влажность: " + readFile(LittleFS, "/humidity.txt") + " %\n";
                    s += "  Концентрация CO2: " + readFile(LittleFS, "/co2.txt") + " ppm";
                    send_message(user_id, s, "[[{\"text\": \"Эталонные показатели: температура\"}], [{\"text\": \"Эталонные показатели: влажность\"}], [{\"text\": \"Эталонные показатели: концентрация CO2\"}], [{\"text\": \"Меню\"}]]");
                }
                else if (text == "Эталонные показатели: температура")
                {
                    send_message(user_id, "Введите значение температуры:");
                    bot_state = States::TEMPERATURE_DIALOG;
                }
                else if (text == "Эталонные показатели: влажность")
                {
                    send_message(user_id, "Введите значение влажности:");
                    bot_state = States::HUMIDITY_DIALOG;
                }
                else if (text == "Эталонные показатели: концентрация CO2")
                {
                    send_message(user_id, "Введите значение концентрации CO2:");
                    bot_state = States::CO2_DIALOG;
                }
            }
            else if (bot_state == States::TEMPERATURE_DIALOG)
            {
                float temp;
                if (is_float(text.c_str(), &temp))
                {
                    writeFile(LittleFS, "/temperature.txt", text.c_str());
                    send_message(user_id, "Значение записано", "[[{\"text\": \"Меню\"}]]");
                    bot_state = States::NORMAL;
                }
                else
                {
                    send_message(user_id, "Введите значение температуры:");
                }
            }
            else if (bot_state == States::HUMIDITY_DIALOG)
            {
                float humidity;
                if (is_float(text.c_str(), &humidity))
                {
                    writeFile(LittleFS, "/humidity.txt", text.c_str());
                    send_message(user_id, "Значение записано", "[[{\"text\": \"Меню\"}]]");
                    bot_state = States::NORMAL;
                }
                else
                {
                    send_message(user_id, "Введите значение влажности:");
                }
            }
            else if (bot_state == States::CO2_DIALOG)
            {
                float co2;
                if (is_float(text.c_str(), &co2))
                {
                    writeFile(LittleFS, "/co2.txt", text.c_str());
                    send_message(user_id, "Значение записано", "[[{\"text\": \"Меню\"}]]");
                    bot_state = States::NORMAL;
                }
                else
                {
                    send_message(user_id, "Введите значение концентрации CO2:");
                }
            }
        }
    }
}

void get_updates(int timeout)
{
    DynamicJsonDocument doc(4096);
    String query = String("https://api.telegram.org/bot") + token + String("/getUpdates?timeout=") + String(timeout) + ("&limit=1&offset=") + String(bot_update_offset);
    Serial.println(query);
    if (https.begin(client, query))
    { // HTTPS

        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = https.getString();
                Serial.println(payload);
                DeserializationError error = deserializeJson(doc, payload);
                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return;
                }
                if (doc["result"].size())
                {
                    bot_update_offset = String(doc["result"][0]["update_id"]).toInt() + 1;
                    handle_update(doc["result"][0]);
                }
            }
        }
        else
        {
            Serial.printf("[HTTPS] GET... failed, error: %d\n", httpCode);
        }

        https.end();
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
}

void setup()
{
    Serial.begin(74800);
    Serial.println();
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount LittleFS");
    }
    pinMode(WIFI_RESET_PIN, INPUT);

    // Загружаем в переменные данные из LitteFS

    ssid = readFile(LittleFS, ssidPath);
    pass = readFile(LittleFS, passPath);
    token = readFile(LittleFS, tokenPath);
    owner_id = readFile(LittleFS, idPath).toInt();

    if (ssid == "" || pass == "")
    {
        Serial.println("Undefined SSID or password.");
        set_ap();
        return;
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
    }
    wifi_on = true;
    if (token == "")
    {
        Serial.println("No bot token, can't start bot");
        return;
    }

    randomSeed(analogRead(0));
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2)
    {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));

    client.setTrustAnchors(&cert);
}

void loop()
{
    
    if (digitalRead(WIFI_RESET_PIN) == LOW)
    {
        Serial.println("reset from loop");
        reset_wifi();
        ESP.reset();
    }
    if (wifi_on){
        get_updates(30);
        int carbon_reference = readFile(LittleFS, "/co2.txt").toInt();
        int temperature_reference = readFile(LittleFS, "/temperature.txt").toInt();
        int humidity_reference = readFile(LittleFS, "/humidity.txt").toInt();
        if ((getCarbonSensorInfo(0) + getCarbonSensorInfo(1)) / 2 > carbon_reference + 200)
        {
            setCarbonRelay(true);
            carbon_sensor = carbon_reference;
            send_message(owner_id, "Понижение уровня CO2", "", true);
        }
        else
        {
            setCarbonRelay(false);
        }
        if ((getTemperatureSensorInfo(0) + getTemperatureSensorInfo(1)) / 2 > temperature_reference + 1)
        {
            setACRelay(true);
            temperature_sensor = temperature_reference;
            send_message(owner_id, "Понижение уровня температуры", "", true);
        }
        else
        {
            setACRelay(false);
        }
        if ((getTemperatureSensorInfo(0) + getTemperatureSensorInfo(1)) / 2 < temperature_reference - 1)
        {
            setHeaterRelay(true);
            temperature_sensor = temperature_reference;
            send_message(owner_id, "Повышение уровня температуры", "", true);
        }
        else
        {
            setHeaterRelay(false);
        }
        if ((getHumiditySensorInfo(0) + getHumiditySensorInfo(1)) / 2 > humidity_reference + 5)
        {
            setHumidiferRelay(true);
            humidity_sensor = humidity_reference;
            send_message(owner_id, "Понижение уровня влажности", "", true);
        }
        else
        {
            setHumidiferRelay(false);
        }
        if ((getHumiditySensorInfo(0) + getHumiditySensorInfo(1)) / 2 < humidity_reference - 5)
        {
            setDehumidiferRelay(true);
            humidity_sensor = humidity_reference;
            send_message(owner_id, "Повышение уровня влажности", "", true);
        }
        else
        {
            setDehumidiferRelay(false);
        }
    }
}