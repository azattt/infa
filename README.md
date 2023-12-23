Используется ESP8266 (что-то типо ардуино с WI-FI) для сбора значений с двух датчиков температуры, влажности и концентрации CO2 (ну нужно реализовать методы получения этих данных)
Далее, эти значения сравниваются с эталонными и вызываются функции set*****Relay (тоже нужно реализовать), которые включают разные устройства для коррекции этих значений до эталонных.<br><br>
Пользователю доступен интерфейс - телеграм-бот.<br><br>
Для настройки WIFI-подключения используется веб-интерфейс:
<br>
```
WIFI Менеджер для ESP32. Блог Амперкот.ру
https://amperkot.ru/blog/wifi-manager-esp32/?ysclid=lpspybwo2a967954500
```
<br>
По умолчанию SSID и пароль WIFI не заданы, поэтому ESP8266 сама раздает WIFI-точку (Access Point). Далее, пользователь подключается к WIFI и вводит данные от реальной WIFI-сети. Для сброса нужно сбросить конфигурационные файлы во флеш-памяти (ssid.txt, pass.txt, ...) или подключить настоящую кнопку к какому-то там пину (настраивается в коде GPIO чето там).<br><br>
Демонстрация работы:<br><br>
щас ссылка здесь будет
