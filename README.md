Mirtek_GW
Модуль отображения информации МИРТ-830 из ардуины

При первом включении устройство создаёт AP "Mirtek_GW".
К ней можно подключиться по WiFi (пароль 12345678) и зайти браузером по 192.168.4.1.
Далее через WEB конфигурятся параметры вашего WiFi, сервера MQTT и номер счётчика.
Для доступа к страничке конфигурирования 
user: admin
passwd: [hidden ap_password]

К PIN16 (STATUS_PIN) можно подключить светодиод - будет показывать статус подключения к WiFi.

У библиотеки SmartRC-CC1101-Driver-Lib есть два режима
с использованием GDO0, и без. 
использован режим с GDO0.
 
используется для определения момента, когда пакет принят и загружен в FIFO приёмника. Подробнее вот тут написано:
https://radiokot.ru/circuit/digital/security/34/ 

Источник
https://radiokot.ru/forum/viewtopic.php?f=25&t=171991&sid=7b3c808c03e1419bcb42bf007edf50a4&start=40
Автор Vittaly76
Огромное спасибо Сергею Безрукову (Ser60), Dismas, _borisovich_, Vasfed!



