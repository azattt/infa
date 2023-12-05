#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP8266TrueRandom.h>
#include "LittleFS.h"

#define WIFI_SSID "Keenetic-0982"
#define WIFI_PASSWORD "Jh9tjDvU"
#define BOT_TOKEN "6557872997:AAGxV647TEkipfrIFkzHNRtArsDuUqxvC74"
const unsigned long BOT_MTBS = 100;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;
int ledStatus = 0;

int getCarbonSensorInfo(int sensor_id){
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
int getHumiditySensorInfo(int sensor_id){
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
int getTemperatureSensorInfo(int sensor_id){
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


void handleNewMessages(int numNewMessages)
{
    Serial.println("new message");
    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        if (from_name == "")
            from_name = "Guest";
        if (text == "/start")
        {
            bot.sendMessageWithReplyKeyboard(chat_id, "Меню", "", "[[\"Текущие показатели\"], [\"Эталонные показатели\"], [\"Обратная связь\"]]");
        }
        if (text == "Текущие показатели")
        {
            bot.sendMessage(chat_id, 
                String("Температура: \n") +
                String("    Первый датчик: ") + String(getTemperatureSensorInfo(0)) + String(" °C\n") +
                String("    Второй датчик: ") + String(getTemperatureSensorInfo(1)) + String(" °C\n") +
                String("Концентрация CO2: \n") +
                String("    Первый датчик: ") + String(getCarbonSensorInfo(0)) + String(" ppm\n") +
                String("    Второй датчик: ") + String(getCarbonSensorInfo(1)) + String(" ppm\n") +
                String("Влажность: \n") +
                String("    Первый датчик: ") + String(getHumiditySensorInfo(0)) + String("%\n") +
                String("    Второй датчик: ") + String(getHumiditySensorInfo(1)) + String("%\n")
            );
        }
        // if (text == "Обратная связь")
        // {
        //     bot.sendMessageWithReplyKeyboard(chat_id, "Обратная связь", "", "[[{\"text\": \"Отзыв\", \"url\": \"https://google.com\"}], [{\"text\": \"Тех. поддержка\", \"url\": \"https://yandex.ru\"}]]");
        // }
    }
}

void setup()
{
    Serial.begin(9600);
    if(!LittleFS.begin()){
        Serial.println("Failed to mount LittleFS");
    }else{
        Serial.println("zaebis");
    }
    File file = LittleFS.open("test.txt", "r");
    Serial.println(file.available());
    char* buffer = new char[128];
    file.read(reinterpret_cast<uint8_t*>(buffer), 128);
    Serial.println(buffer);
    // randomSeed(analogRead(0));
    // Serial.begin(9600);
    // Serial.println();
    // configTime(0, 0, "pool.ntp.org");
    // secured_client.setTrustAnchors(&cert);
    // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // while (WiFi.status() != WL_CONNECTED)
    // {

    //     delay(500);
    // }
    // Serial.print("WiFi connected. IP address: ");
    // Serial.println(WiFi.localIP());
    // Serial.print("Retrieving time: ");
    // time_t now = time(nullptr);
    // while (now < 24 * 3600)
    // {
    //     delay(100);
    //     now = time(nullptr);
    // }

}

void loop()
{
    // if (millis() - bot_lasttime > BOT_MTBS)
    // {
    //     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    //     while (numNewMessages)
    //     {

    //         handleNewMessages(numNewMessages);
    //         numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    //     }
    //     bot_lasttime = millis();
    // }
}