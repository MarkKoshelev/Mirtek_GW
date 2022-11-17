//by Little_S@tan
//MQTT topics: "mirtek/Request_Status", "mirtek/action"
//#define MY_CC1101
// https://github.com/1technophile/OpenMQTTGateway/issues/329

#include <MQTT.h>
#include <SPI.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "CRC8.h"
#include "CRC.h"
#include <TimerMs.h>


unsigned tmr_tele_time=30*1000; //раз в 10 cек запрашиваем и отправляем информацию в mqtt
//#include <cc1101_debug_service.h>

#ifdef MY_CC1101
  #include <cc1101.h>
  #define ADDR   0x00  // Device Address
  #define CHANNR 0x16  // Channel Number
  CC1101 cc1101;

void CC1101::MirtekSetRegs(void) 
{
	writeReg(CC1101_IOCFG2, 0x0D);   // IOCFG2              GDO2 Output Pin Configuration
	writeReg(CC1101_IOCFG1, 0x2E);   // IOCFG1              GDO1 Output Pin Configuration
	writeReg(CC1101_IOCFG0, 0x06);   // IOCFG0              GDO0 Output Pin Configuration
	writeReg(CC1101_FIFOTHR, 0x4F);  // FIFOTHR             RX FIFO and TX FIFO Thresholds
	writeReg(CC1101_PKTLEN, 0x3C);   // PKTLEN              Packet Length
	writeReg(CC1101_PKTCTRL1, 0x00); // PKTCTRL1            Packet Automation Control
	writeReg(CC1101_PKTCTRL0, 0x41); // PKTCTRL0            Packet Automation Control
	writeReg(CC1101_MDMCFG4, 0xD9);  // MDMCFG4             Modem Configuration
	writeReg(CC1101_MDMCFG3, 0x83);  // MDMCFG3             Modem Configuration
	writeReg(CC1101_MDMCFG2, 0x13);  // MDMCFG2             Modem Configuration
	writeReg(CC1101_MDMCFG1, 0xD2);  // MDMCFG1             Modem Configuration
	writeReg(CC1101_MDMCFG0, 0xAA);  // MDMCFG0             Modem Configuration
	writeReg(CC1101_DEVIATN, 0x31);  // DEVIATN             Modem Deviation Setting
	writeReg(CC1101_MCSM2, 0x07);    // MCSM2               Main Radio Control State Machine Configuration
	writeReg(CC1101_MCSM1, 0x0C);    // MCSM1               Main Radio Control State Machine Configuration
	writeReg(CC1101_MCSM0, 0x08);    // MCSM0               Main Radio Control State Machine Configuration
	writeReg(CC1101_FOCCFG, 0x16);   // FOCCFG              Frequency Offset Compensation Configuration
	writeReg(CC1101_BSCFG, 0x6C);    // BSCFG               Bit Synchronization Configuration
	writeReg(CC1101_AGCCTRL2, 0x03); // AGCCTRL2            AGC Control
	writeReg(CC1101_AGCCTRL1, 0x40); // AGCCTRL1            AGC Control
	writeReg(CC1101_AGCCTRL0, 0x91); // AGCCTRL0            AGC Control
	writeReg(CC1101_WOREVT1, 0x87);  // WOREVT1             High Byte Event0 Timeout
	writeReg(CC1101_WOREVT0, 0x6B);  // WOREVT0             Low Byte Event0 Timeout
	writeReg(CC1101_WORCTRL, 0xF8);  // WORCTRL             Wake On Radio Control
	writeReg(CC1101_FREND1, 0x56);   // FREND1              Front End RX Configuration
	writeReg(CC1101_FREND0, 0x10);   // FREND0              Front End TX Configuration
	writeReg(CC1101_FSCAL3, 0xE9);   // FSCAL3              Frequency Synthesizer Calibration
	writeReg(CC1101_FSCAL2, 0x2A);   // FSCAL2              Frequency Synthesizer Calibration
	writeReg(CC1101_FSCAL1, 0x00);   // FSCAL1              Frequency Synthesizer Calibration
	writeReg(CC1101_FSCAL0, 0x1F);   // FSCAL0              Frequency Synthesizer Calibration
	writeReg(CC1101_RCCTRL1, 0x41);  // RCCTRL1             RC Oscillator Configuration
	writeReg(CC1101_RCCTRL0, 0x00);  // RCCTRL0             RC Oscillator Configuration
	writeReg(CC1101_FSTEST,  0x59);  // FSTEST              Frequency Synthesizer Calibration Control
	writeReg(CC1101_PTEST, 0x59);    // PTEST               Production Test
	writeReg(CC1101_AGCTEST, 0x3F);  // AGCTEST             AGC Test
	writeReg(CC1101_TEST2, 0x81);    // TEST2               Various Test Settings
	writeReg(CC1101_TEST1, 0x35);    // TEST1               Various Test Settings
	writeReg(CC1101_TEST0, 0x09);    // TEST0               Various Test Settings
  
  // Send empty packet
  CCPACKET packet;
  packet.length = 0;
  sendData(packet);
}

#else

//Настройки для CC1101 с форума (47 бит)
byte rfSettings[] = {
  0x0D,  // IOCFG2              GDO2 Output Pin Configuration
  0x2E,  // IOCFG1              GDO1 Output Pin Configuration
  0x06,  // IOCFG0              GDO0 Output Pin Configuration
  0x4F,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
  0xD3,  // SYNC1               Sync Word, High Byte
  0x91,  // SYNC0               Sync Word, Low Byte
  0x3C,  // PKTLEN              Packet Length
  0x00,  // PKTCTRL1            Packet Automation Control
  0x41,  // PKTCTRL0            Packet Automation Control
  0x00,  // ADDR                Device Address
  0x16,  // CHANNR              Channel Number
  //0x0B,  // CHANNR              Channel Number
  0x0F,  // FSCTRL1             Frequency Synthesizer Control
  0x00,  // FSCTRL0             Frequency Synthesizer Control
  0x10,  // FREQ2               Frequency Control Word, High Byte
  0x8B,  // FREQ1               Frequency Control Word, Middle Byte
  0x54,  // FREQ0               Frequency Control Word, Low Byte
  0xD9,  // MDMCFG4             Modem Configuration
  0x83,  // MDMCFG3             Modem Configuration
  0x13,  // MDMCFG2             Modem Configuration
  0xD2,  // MDMCFG1             Modem Configuration
  0xAA,  // MDMCFG0             Modem Configuration
  0x31,  // DEVIATN             Modem Deviation Setting
  0x07,  // MCSM2               Main Radio Control State Machine Configuration
  0x0C,  // MCSM1               Main Radio Control State Machine Configuration
  0x08,  // MCSM0               Main Radio Control State Machine Configuration
  0x16,  // FOCCFG              Frequency Offset Compensation Configuration
  0x6C,  // BSCFG               Bit Synchronization Configuration
  0x03,  // AGCCTRL2            AGC Control
  0x40,  // AGCCTRL1            AGC Control
  0x91,  // AGCCTRL0            AGC Control
  0x87,  // WOREVT1             High Byte Event0 Timeout
  0x6B,  // WOREVT0             Low Byte Event0 Timeout
  0xF8,  // WORCTRL             Wake On Radio Control
  0x56,  // FREND1              Front End RX Configuration
  0x10,  // FREND0              Front End TX Configuration
  0xE9,  // FSCAL3              Frequency Synthesizer Calibration
  0x2A,  // FSCAL2              Frequency Synthesizer Calibration
  0x00,  // FSCAL1              Frequency Synthesizer Calibration
  0x1F,  // FSCAL0              Frequency Synthesizer Calibration
  0x41,  // RCCTRL1             RC Oscillator Configuration
  0x00,  // RCCTRL0             RC Oscillator Configuration
  0x59,  // FSTEST              Frequency Synthesizer Calibration Control
  0x59,  // PTEST               Production Test
  0x3F,  // AGCTEST             AGC Test
  0x81,  // TEST2               Various Test Settings
  0x35,  // TEST1               Various Test Settings
  //0x0B,  // TEST0               Various Test Settings
  0x09,  // TEST0               Various Test Settings
};
#endif

#define MAX_METERS 4
struct {
    unsigned MeterAdress;
    unsigned DomoticzP1Idx;
    float sum;
    float t1; 
    float t2;
    float cons;
} meter[MAX_METERS];

bool  cc1101_is_ready;

#if ESP8266
    int gdo0 = 5;  // D1 for Wemos
#elif ESP32
    int gdo0 = 2; //for esp32! GDO0 on GPIO2.
#else
  #error Unsupported platform
#endif

CRC8 crc;

byte transmitt_byte1[] = {0x0F, 0x73, 0x55, 0x20, 0x00, 0, 0, 0x09, 0xff, 0, 0, 0, 0, 0, 0, 0x55};
byte transmitt_byte[] = {0x10, 0x73, 0x55, 0x21, 0x00, 0, 0, 0x09, 0xff, 0, 0, 0, 0, 0, 0, 0, 0x55};

byte resultbuffer[61] = { 0 }; //буфер конечного, сшитого принятого пакета
unsigned bytecount = 0; //указатель байтов в результирующем буфере

TimerMs tmr(2000, 0, 0); //инициализируем таймер ожидания ответа счетчика
TimerMs tmr_tele(tmr_tele_time, 0, 0); //инициализируем таймер, отправляющий значения в MQTT

int packetType = 0; //кол-во подпакетов в ответе - зависит от типа запроса (3 для запросов 1-4, 4 для запросов 5,6)

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "Mirtek_GW";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678";

#define STRING_LEN 128
#define NUMBER_LEN 6

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "mirtek_gw_v7"

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN 16

// -- Method declarations.
void handleRoot();
void mqttMessageReceived(String& topic, String& payload);
bool connectMqtt();
bool connectMqttOptions();
// -- Callback methods.
void wifiConnected();
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);
WiFiClient net;
MQTTClient mqttClient;

bool needMqttConnect = false;
bool needReset = false;
bool needSetMeterParams = false;

unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;

char telePeriodValue[STRING_LEN];
char mqttEnableValue[STRING_LEN];
char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

char MeterAdressValue1[NUMBER_LEN];
char MeterAdressValue2[NUMBER_LEN];
char MeterAdressValue3[NUMBER_LEN];
char MeterAdressValue4[NUMBER_LEN];
char DomoticzP1IdxValue1[NUMBER_LEN];
char DomoticzP1IdxValue2[NUMBER_LEN];
char DomoticzP1IdxValue3[NUMBER_LEN];
char DomoticzP1IdxValue4[NUMBER_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfNumberParameter telePeriod = IotWebConfNumberParameter("Период опроса (сек)", "telePeriod", telePeriodValue, NUMBER_LEN, "30", "30..3600", "min='10' max='3600' step='10'");

// -- You can also use namespace formats e.g.: iotwebconf::ParameterGroup
IotWebConfParameterGroup mqttGroup = IotWebConfParameterGroup("mqtt", "MQTT configuration");
IotWebConfCheckboxParameter mqttEnableParam = IotWebConfCheckboxParameter("MQTT Enable", "mqttEnable", mqttEnableValue, STRING_LEN, false);
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
IotWebConfTextParameter mqttUserNameParam = IotWebConfTextParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN);
IotWebConfPasswordParameter mqttUserPasswordParam = IotWebConfPasswordParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN);

IotWebConfParameterGroup group1 = IotWebConfParameterGroup("group1", "Настройки Cчетчика 1");
IotWebConfNumberParameter MeterAdress1 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress1", MeterAdressValue1, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx1 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx1", DomoticzP1IdxValue1, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group2 = IotWebConfParameterGroup("group2", "Настройки Cчетчика 2");
IotWebConfNumberParameter MeterAdress2 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress2", MeterAdressValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx2 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx2", DomoticzP1IdxValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group3 = IotWebConfParameterGroup("group3", "Настройки Cчетчика 3");
IotWebConfNumberParameter MeterAdress3 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress3", MeterAdressValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx3 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx3", DomoticzP1IdxValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group4 = IotWebConfParameterGroup("group4", "Настройки Cчетчика 4");
IotWebConfNumberParameter MeterAdress4 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress4", MeterAdressValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx4 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx4", DomoticzP1IdxValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

bool iotWebConfInit(){
    iotWebConf.addSystemParameter(&telePeriod); 

    mqttGroup.addItem(&mqttEnableParam);
    mqttGroup.addItem(&mqttServerParam);
    mqttGroup.addItem(&mqttUserNameParam);
    mqttGroup.addItem(&mqttUserPasswordParam);

    group1.addItem(&MeterAdress1);
    group1.addItem(&DomoticzP1Idx1);
    
    group2.addItem(&MeterAdress2);
    group2.addItem(&DomoticzP1Idx2);

    group3.addItem(&MeterAdress3);
    group3.addItem(&DomoticzP1Idx3);

    group4.addItem(&MeterAdress4);
    group4.addItem(&DomoticzP1Idx4);

    iotWebConf.setStatusPin(STATUS_PIN);
    //iotWebConf.setConfigPin(CONFIG_PIN);
    iotWebConf.addParameterGroup(&mqttGroup);
    iotWebConf.addParameterGroup(&group1);
    iotWebConf.addParameterGroup(&group2);
    iotWebConf.addParameterGroup(&group3);
    iotWebConf.addParameterGroup(&group4);
    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setFormValidator(&formValidator);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);

    // -- Initializing the configuration.
    return iotWebConf.init();
}

void packetSender(byte tr[])  //функция отправки пакета
{
    ELECHOUSE_cc1101.SpiStrobe(0x33);  //Calibrate frequency synthesizer and turn it off
    delay(5);
    ELECHOUSE_cc1101.SpiStrobe(0x3B);  // Flush the TX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x36);  // Exit RX / TX, turn off frequency synthesizer and exit
    ELECHOUSE_cc1101.SpiWriteReg(0x3e, 0xC4); //выставляем мощность 10dB
	Serial.print("Packet sent: "); Serial.println(*tr);
    ELECHOUSE_cc1101.SendData(tr, tr[0] + 1); //отправляем пакет
    ELECHOUSE_cc1101.SpiStrobe(0x3A);  // Flush the RX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x34);  // Enable RX
    
    for (int i = 0; i < tr[0] + 1; i++) {
        Serial.print(tr[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Request Type: 0x20, PacketLen: 0x0F
void RequestPacket1(byte tr[], unsigned addr, byte code) {
      Serial.print("RequestPacket: "); Serial.print("Addr: "); Serial.print(addr); Serial.print(" Code: "); Serial.println(code, HEX);
	
//    tr[0] = 0x0F; // длина пакета 16 байт
//    tr[1] = 0x73; // const: 
//    tr[2] = 0x55; // const: начало payload
//    tr[3] = 0x20; // const: тип запроса
//    tr[4] = 0x00; // const: 0 
    tr[5] = addr & 0xff;        //младший байт адреса счётчика
    tr[6] = (addr >> 8) & 0xff; //старший байт адреса счётчика

//    tr[7] = 0x09;  // interface CC1101
//    tr[8] = 0xff;  // const: FF 
    tr[9] = code;    // request code
//    tr[10] = 0x00; //PIN
//    tr[11] = 0x00; //PIN
//    tr[12] = 0x00; //PIN
//    tr[13] = 0x00; //PIN
    crc.restart();
    crc.setPolynome(0xA9);
    for (int i = 3; i < (tr[0] - 1); i++)
    {
        crc.add(tr[i]);
    }
    tr[14] = crc.getCRC(); //CRC
//    tr[15] = 0x55; //конец payload
    packetSender(tr);   //отправляем пакет
    packetType = 4;
}

// Request Type: 0x21, PacketLen: 0x10
void RequestPacket2(byte tr[], unsigned addr ,byte code, byte type) {
      Serial.print("RequestPacket: "); Serial.print("Addr: "); Serial.print(addr); Serial.print(" Code: "); Serial.print(code, HEX); Serial.print(" Type: "); Serial.println(type, HEX);
	
//    tr[0] = 0x10; //длина пакета 17 байт
//    tr[1] = 0x73; // const: 
//    tr[2] = 0x55; // const: начало payload
//    tr[3] = 0x21; // const: тип запроса
//    tr[4] = 0x00; // const: 0 
    tr[5] = addr & 0xff;       //младший байт адреса счётчика
    tr[6] = (addr>> 8) & 0xff; //старший байт адреса счётчика

//    tr[7] = 0x09;  // interface CC1101
//    tr[8] = 0xff;  // const: FF 
    tr[9] = code;  // request code
//    tr[10] = 0x00; //PIN
//    tr[11] = 0x00; //PIN
//    tr[12] = 0x00; //PIN
//    tr[13] = 0x00; //PIN
    tr[14] = type; //???? 0-9 // https://radiokot.ru/forum/download/file.php?id=379310 file:CQ_DAT    
    crc.restart();
    crc.setPolynome(0xA9);
    for (int i = 3; i < (tr[0] - 1); i++)
    {
        crc.add(tr[i]);
    }
    tr[15] = crc.getCRC(); //CRC
//    tr[16] = 0x55; //конец payload
    packetSender(tr);   //отправляем пакет
    packetType = 4;
}

/*
//Функция формирования 1-го (начального) пакета (date, time)
void RequestPacket_1() {
    RequestPacket(transmitt_byte1,meter[0].MeterAdress, 0x1C); 
    packetType = 3;
}

//Функция формирования 2-го пакета
void RequestPacket_2() {
    RequestPacket(transmitt_byte1,meter[0].MeterAdress, 0x10); 
    packetType = 3;
}

//Функция формирования 3-го пакета
void RequestPacket_3() {
    RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x28, 0x00); 
    packetType = 3;
}

//Функция формирования 4-го пакета
void RequestPacket_4() {
    RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x28, 0x01); 
    packetType = 3;
}

//Функция формирования 5-го пакета
void RequestPacket_5() {
    RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x05, 0x00); 
    packetType = 4;
}

//Функция формирования 6-го пакета
void RequestPacket_6() {
  RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x0a, 0x01); 
  packetType = 4;
}

//Функция формирования 7-го пакета (Действующие значения напряжения, тока)
void RequestPacket_7() {
  RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x2a, 0x01); 
  packetType = 4;
}

//Функция формирования 8-го пакета (Aabs)
void RequestPacket_8() {
  RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x05, 0x04); 
  packetType = 4;
}

//Функция формирования 9 (параметры сети)
void RequestPacket_9() {
  RequestPacket(transmitt_byte,meter[0].MeterAdress, 0x2B, 0x00); 
  packetType = 4;
}
*/

//функция приёма пакета (помещает его в resultbuffer[])
bool packetReceiver() {
//    for (int i=0; i < 61; i++){
//      resultbuffer[i]=0x00;
//    }
    
    tmr.start();
    int PackCount = 0; //счётчик принятых из эфира пакетов
    bytecount = 0;     //указатель байтов в результирующем буфере
    byte buffer[61];   //буффер пакетов, принятых из трансивера (или отправленных в трансивер)
    
    Serial.print("ReceiveDataSize:");
    while (!tmr.tick() && PackCount < packetType) {
        delay(5);
        if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
            PackCount++;
            /*
            //Rssi Level in dBm
            Serial.print("Rssi: ");
            Serial.println(ELECHOUSE_cc1101.getRssi());

            //Link Quality Indicator
            Serial.print("LQI: ");
            Serial.println(ELECHOUSE_cc1101.getLqi());
            */

            //Get received Data and calculate length
            //int size= ELECHOUSE_cc1101.SpiReadReg(CC1101_RXFIFO);
            // Serial.println("ReceiveData1: "); Serial.println(size);
            int len = ELECHOUSE_cc1101.ReceiveData(buffer);
            delay(1);
            ELECHOUSE_cc1101.SpiStrobe(0x36);  // Exit RX / TX, turn off frequency synthesizer and exit
            ELECHOUSE_cc1101.SpiStrobe(0x3A);  // Flush the RX FIFO buffer
            ELECHOUSE_cc1101.SpiStrobe(0x3B);  // Flush the TX FIFO buffer
            ELECHOUSE_cc1101.SpiStrobe(0x34);  // Enable RX

            //buffer[len] = '\0';
            Serial.print(" "); Serial.print(len); Serial.print(" "); Serial.print(buffer[0]); 

            
            //Print received in bytes format.
            /*
            for (int i = 0; i < len; i++) {
              //sprintf(s, "%02x", buffer[i]);
              //Serial.print(s);
              Serial.print(buffer[i], HEX);
              Serial.print(" ");
            }
            Serial.println();
            */
            int start = 1;
        //    if (PackCount == 4 && len == 6 ){
        //        start = 2;
        //    }
            //подшиваем 1/3 пакета в общий пакет
            for (int i = start; i < len && bytecount < sizeof(resultbuffer); i++) {
                resultbuffer[bytecount] = buffer[i];
                bytecount++;
            }

        }
    }
    Serial.println("");
    Serial.print("Packets received: ");
    Serial.println(PackCount);
    //Печатаем общий пакет
    for (unsigned i = 0; i < bytecount; i++)
    {
        Serial.print(resultbuffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    Serial.print("Packet length: "); Serial.println(bytecount);
    // Test CRC ------------------------------
    crc.reset();
    crc.setPolynome(0xA9);
    for (unsigned i = 2; i < (bytecount - 2); i++)
    {
        crc.add(resultbuffer[i]);
    }
  
    uint8_t myCRC = crc.getCRC();
    Serial.print("R_CRC: "); Serial.print(resultbuffer[bytecount - 2], HEX); Serial.print(" C_CRC: "); Serial.println(myCRC, HEX);
    if(resultbuffer[bytecount - 2] == myCRC){
        return true;
    }
    else{
        return false;
    }
}

void packetParser_7(unsigned meterIdx) {
  if ( (bytecount == 45) and (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x2B) and (resultbuffer[12]==0x0) )
  {
    Serial.print("A:  ");
	Serial.print(resultbuffer[20], HEX); Serial.print(resultbuffer[19], HEX); Serial.print(resultbuffer[18], HEX); Serial.print("  ");
    meter[meterIdx].cons = float((resultbuffer[18] | (resultbuffer[19] << 8) | (resultbuffer[20] << 16)));
	Serial.println(meter[meterIdx].cons);

    Serial.print("R:  ");
	Serial.print(resultbuffer[23], HEX); Serial.print(resultbuffer[22], HEX); Serial.print(resultbuffer[21], HEX); Serial.print("  ");
    Serial.println(float((resultbuffer[21] | (resultbuffer[22] << 8) | (resultbuffer[23] << 16) )));

    Serial.print("Freq:  ");
    Serial.println(float((resultbuffer[24] | (resultbuffer[25] << 8)))/100);

    Serial.print("Cos:  ");
    Serial.println(float((resultbuffer[26] | (resultbuffer[27] << 8))) / 1000);

    Serial.print("V1:  ");
	Serial.print(resultbuffer[29], HEX); Serial.print(resultbuffer[28], HEX); Serial.print("  ");
    Serial.println(float((resultbuffer[28] | (resultbuffer[29] << 8))) / 100);

    Serial.print("V2:  ");
	Serial.print(resultbuffer[31], HEX); Serial.print(resultbuffer[30], HEX); Serial.print("  ");
    Serial.println(float((resultbuffer[30] | (resultbuffer[31] << 8))) / 100);

    Serial.print("V3:  ");
	Serial.print(resultbuffer[33], HEX); Serial.print(resultbuffer[32], HEX); Serial.print("  ");
    Serial.println(float((resultbuffer[32] | (resultbuffer[33] << 8))) / 100);

    Serial.print("I1  ");
	Serial.print(resultbuffer[36], HEX); Serial.print(resultbuffer[35], HEX); Serial.print(resultbuffer[34], HEX); Serial.print("  ");
    Serial.println(float(resultbuffer[34] | (resultbuffer[35] << 8) | (resultbuffer[36] << 16)) / 1000);

    Serial.print("I2  ");
	Serial.print(resultbuffer[39], HEX); Serial.print(resultbuffer[38], HEX); Serial.print(resultbuffer[37], HEX); Serial.print("  ");
    Serial.println(float(resultbuffer[37] | (resultbuffer[38] << 8) | (resultbuffer[39] << 16)) / 1000);

    Serial.print("I3  ");
	Serial.print(resultbuffer[42], HEX); Serial.print(resultbuffer[41], HEX); Serial.print(resultbuffer[40], HEX); Serial.print("  ");
    Serial.println(float(resultbuffer[40] | (resultbuffer[41] << 8) | (resultbuffer[42] << 16)) / 1000);
  }else{
    Serial.println("PARSING ERROR!");
  }
}

void packetParser_7_mqtt() {
  if ( (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x2B) and (resultbuffer[12]==0x0) )
  {
    mqttClient.publish("mirtek/Freq", String(float((resultbuffer[24] | (resultbuffer[25] << 8)))/100));
    mqttClient.publish("mirtek/Cos", String(float((resultbuffer[26] | (resultbuffer[27] << 8)))/100));
    mqttClient.publish("mirtek/V1", String(float((resultbuffer[28] | (resultbuffer[29] << 8)))/100));
    mqttClient.publish("mirtek/V2", String(float((resultbuffer[30] | (resultbuffer[31] << 8)))/100));
    mqttClient.publish("mirtek/V3", String(float((resultbuffer[32] | (resultbuffer[33] << 8)))/100));
    mqttClient.publish("mirtek/I1", String(float(resultbuffer[34] | (resultbuffer[35] << 8) | (resultbuffer[36] << 16)) / 1000));
    mqttClient.publish("mirtek/I2", String(float(resultbuffer[37] | (resultbuffer[38] << 8) | (resultbuffer[39] << 16)) / 1000));
    mqttClient.publish("mirtek/I3", String(float(resultbuffer[40] | (resultbuffer[41] << 8) | (resultbuffer[42] << 16)) / 1000));
  }else{
    Serial.println("PARSING ERROR!");
  }
}


// RequestPacket(transmitt_byte,MeterAdress[0], 0x05, 0); 
//enum code		
// 4 - Aabs = "A+" + "A-"
// 0 - A+
// 1 - A-
// 5 - Rabs = "R+" + "R-"
// 2 - R+   =  R1 + R2 
// 3 - R-   =  R3 + R4
// 6 - R1 
// 7 - R2
// 8 - R3
// 9 - R4
void packetParser_5(unsigned meterIdx) {
  if ( (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x5) and (resultbuffer[17]==0x1) and (resultbuffer[44]==0x55) )
  {
    Serial.print("SUM:  ");
	Serial.print(resultbuffer[26], HEX); Serial.print(resultbuffer[25], HEX); Serial.print(resultbuffer[24], HEX); Serial.print(resultbuffer[23], HEX); Serial.print("  ");
    meter[meterIdx].sum = float((resultbuffer[25] << 16) | (resultbuffer[24] << 8) | resultbuffer[23]) * 10;
	Serial.println(meter[meterIdx].sum / 1000);

    Serial.print("T1:  ");
	Serial.print(resultbuffer[30], HEX); Serial.print(resultbuffer[29], HEX); Serial.print(resultbuffer[28], HEX); Serial.print(resultbuffer[27], HEX); Serial.print("  ");
    meter[meterIdx].t1  = float((resultbuffer[29] << 16) | (resultbuffer[28] << 8) | resultbuffer[27]) * 10;
    Serial.println(meter[meterIdx].t1 / 1000);

    Serial.print("T2:  ");
	Serial.print(resultbuffer[34], HEX); Serial.print(resultbuffer[33], HEX); Serial.print(resultbuffer[32], HEX); Serial.print(resultbuffer[31], HEX); Serial.print("  ");
    meter[meterIdx].t2  = float((resultbuffer[33] << 16) | (resultbuffer[32] << 8) | resultbuffer[31]) * 10;
    Serial.println(meter[meterIdx].t2 / 1000);   
  }else{
    Serial.println("PARSING ERROR!");
  }
}

void packetParser_1() {
  //73 55 7 0 9 FF E9 99 1C A8 3 5B 0 29 4 0 1 1F A 16 79 55 
  if ( (bytecount == 22) )
  {
    Serial.print("TIME:  ");
	Serial.print(resultbuffer[13], HEX); Serial.print(resultbuffer[14], HEX); Serial.print(resultbuffer[15], HEX); Serial.print("  ");
	Serial.print(resultbuffer[15]); Serial.print(":"); Serial.print(resultbuffer[14]); Serial.print(":"); Serial.println(resultbuffer[13]);

    Serial.print("DATE:  ");
	Serial.print(resultbuffer[19], HEX); Serial.print(resultbuffer[18], HEX); Serial.print(resultbuffer[17], HEX); Serial.print("  ");
 	Serial.print(resultbuffer[17]); Serial.print("."); Serial.print(resultbuffer[18]); Serial.print(".20"); Serial.println(resultbuffer[19]);
   }else{
    Serial.println("PARSING ERROR!");
  }
}

void domoticzP1Publish(unsigned meterIdx ){
//(unsigned idx, float t1, float t2, float cons
// domoticz
// https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Electricity_P1_smart_meter
// idx=IDX&nvalue=0&svalue=USAGE1;USAGE2;RETURN1;RETURN2;CONS;PROD
//{
//  "idx": 281,
//  "nvalue": 0,
//  "svalue": "0.0; 554866.4; 0.0;0.0; 425;0",
//  "Battery": 100,
//  "RSSI": 7
//}
//oid setup() {
//  DynamicJsonDocument doc(1024);
//  JsonObject obj = doc.as<JsonObject>();

  if(meter[meterIdx].t1 != 0 && meter[meterIdx].t2 != 0 ){
    char buffer[128];
    sprintf(buffer, "{\"idx\": %d, \"nvalue\": 0, \"svalue\":\"%d;%d; 0.0;0.0; %d;0\"}",
        meter[meterIdx].DomoticzP1Idx, unsigned(meter[meterIdx].t1), unsigned(meter[meterIdx].t2), unsigned(meter[meterIdx].cons) );
    Serial.print("Domotics: "); Serial.println(buffer);
    mqttClient.publish("domoticz/in", buffer);
  }
}

void packetParser_5_mqtt() {
  if ( (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x5) and (resultbuffer[17]==0x1) and (resultbuffer[44]==0x55) )
  {
//    sum = float((resultbuffer[25] << 16) | (resultbuffer[24] << 8) | resultbuffer[23]) * 10;
//    t1  = float((resultbuffer[29] << 16) | (resultbuffer[28] << 8) | resultbuffer[27]) * 10;
//    t2  = float((resultbuffer[33] << 16) | (resultbuffer[32] << 8) | resultbuffer[31]) * 10;

//    mqttClient.publish("mirtek/SUM", String(sum,2));
//    mqttClient.publish("mirtek/T1", String(t1,2));
//    mqttClient.publish("mirtek/T2", String(t2,2));
  }else{
    Serial.println("PARSING ERROR!");
  }
}


void requestMeters(void) {
    for(int i = 0; i < MAX_METERS; i++) {
        Serial.print("MeterAdress: "); Serial.println(meter[i].MeterAdress);
        if(meter[i].MeterAdress != 0){
            packetType = 3;
            // date time
            RequestPacket1(transmitt_byte1, meter[i].MeterAdress, 0x1C);
            //packetSender(transmitt_byte1);
            if(packetReceiver()){
                packetParser_1();
            }
            // T1, T2
            packetType = 4;
            RequestPacket2(transmitt_byte, meter[i].MeterAdress, 0x05, 0); 
            if(packetReceiver()){
                packetParser_5(i);
            }
          
            // Consume Active Energy, Volts, Ampers
            RequestPacket2(transmitt_byte, meter[i].MeterAdress, 0x2b, 0); 
            if(packetReceiver()){
                packetParser_7(i);
            }

            if(meter[i].DomoticzP1Idx != 0 && mqttClient.connected() ){
                domoticzP1Publish(i);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    //SPI.setFrequency
    bool validConfig = iotWebConfInit(); 
    if (!validConfig)
    {
        mqttServerValue[0] = '\0';
        mqttUserNameValue[0] = '\0';
        mqttUserPasswordValue[0] = '\0';

        MeterAdressValue1[0] = '\0';
        MeterAdressValue2[0] = '\0';
        MeterAdressValue3[0] = '\0';
        MeterAdressValue4[0] = '\0';
        DomoticzP1IdxValue1 [0] = '\0';
        DomoticzP1IdxValue2 [0] = '\0';
        DomoticzP1IdxValue3 [0] = '\0';
        DomoticzP1IdxValue4 [0] = '\0';
    }

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });

    mqttClient.begin(mqttServerValue, net);
//    mqttClient.onMessage(mqttMessageReceived);

    Serial.println("Ready.");
#ifdef MY_CC1101
    byte ver = cc1101.readStatusReg(CC1101_VERSION);
    if(ver != 0) {
        Serial.print("CC1101 ver: "); Serial.println(ver);
        pinMode(gdo0, INPUT);             // Config GDO0 as input

		cc1101.cmdStrobe(CC1101_SRES);    //reset
		cc1101.setCCregs();
		//cc1101.init();

		cc1101.setSyncWord(ADDR, CHANNR); //setSyncWord(SYNC_WORD, false);

	//   cc1101.setCarrierFreq(CFREQ_433);   //from Mirtek_GW
		cc1101.writeReg(CC1101_FREQ2,  0x10); //from Mirtek_GW
		cc1101.writeReg(CC1101_FREQ1,  0x8B); //from Mirtek_GW
		cc1101.writeReg(CC1101_FREQ0,  0x54); //from Mirtek_GW 

	//    cc1101.disableAddressCheck();
		cc1101.writeReg(CC1101_PKTCTRL1,  0x00); //from Mirtek_GW
		cc1101.writeReg(CC1101_PKTCTRL0,  0x41); //from Mirtek_GW
		
		cc1101.cmdStrobe(CC1101_SCAL);   // Calibrate frequency synthesizer and turn it off
		cc1101.cmdStrobe(CC1101_SFRX);   // Flush the RX FIFO buffer  
		cc1101.cmdStrobe(CC1101_SFTX);   // Flush the TX FIFO buffer
		cc1101.cmdStrobe(CC1101_SRX);    // Enable RX
	}
	else{
        Serial.println("CC1101 Connection Error");
	}
	
#else
    ELECHOUSE_cc1101.setGDO0(gdo0);
    
   if (ELECHOUSE_cc1101.getCC1101() ){
    
        unsigned s = ELECHOUSE_cc1101.SpiReadStatus(0x31);
        Serial.print("CC1101 Connection OK, Status: "); Serial.println(s);
        cc1101_is_ready = true;
    //Инициализируем cc1101
		ELECHOUSE_cc1101.SpiStrobe(0x30);  //reset
    
		ELECHOUSE_cc1101.SpiWriteBurstReg(0x00, rfSettings, 0x2F);
		ELECHOUSE_cc1101.SpiStrobe(0x33);  //Calibrate frequency synthesizer and turn it off
		delay(1);
		ELECHOUSE_cc1101.SpiStrobe(0x3A);  // Flush the RX FIFO buffer  CC1101_SFRX
		ELECHOUSE_cc1101.SpiStrobe(0x3B);  // Flush the TX FIFO buffer  CC1101_SFTX
		ELECHOUSE_cc1101.SpiStrobe(0x34);  // Enable RX                 CC1101_SRX
        meter[0].MeterAdress = atoi(MeterAdressValue1); 
        meter[1].MeterAdress = atoi(MeterAdressValue2);
        meter[2].MeterAdress = atoi(MeterAdressValue3);
        meter[3].MeterAdress = atoi(MeterAdressValue4);
        meter[0].DomoticzP1Idx = atoi(DomoticzP1IdxValue1);
        meter[1].DomoticzP1Idx = atoi(DomoticzP1IdxValue2);
        meter[2].DomoticzP1Idx = atoi(DomoticzP1IdxValue3);
        meter[3].DomoticzP1Idx = atoi(DomoticzP1IdxValue4);
        //requestMeters();
    }
    else {
        unsigned s = ELECHOUSE_cc1101.SpiReadStatus(0x31);
        Serial.print("CC1101 Connection Error, Status: "); Serial.println(s);
    }
#endif
    //Serial.println("Rx Mode");
    tmr_tele.setTime(atoi(telePeriodValue) * 1000);
    tmr_tele.start(); //старт таймера MQTT
}

void loop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();
      
    if (needReset) {
        Serial.println("Rebooting after 1 second.");
        iotWebConf.delay(1000);
        ESP.restart();
    }
    
    if(mqttEnableParam.isChecked()){
        mqttClient.loop();
        if (needMqttConnect) {
            if (connectMqtt()) {
                needMqttConnect = false;
            }
        }
        else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient.connected())) {
            Serial.println("MQTT reconnect");
            connectMqtt();
        }
    }

  if ((cc1101_is_ready == true) && (iotWebConf.getState() == iotwebconf::OnLine) ){
    if (tmr_tele.tick()) // Запрос информации по таймеру
      {
        Serial.println("--------------------->Request MIRTEK by timer");
        requestMeters();
        Serial.println("<---------------------Request MIRTEK by timer");Serial.println("");
      }
    }
    //delay(2000);
}

void handleRoot()
{
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Mirtek-to-MQTT</title></head><body>Gateway";
    s += "<ul>";
    s += "<li>MQTT server: ";
    s += mqttServerValue;
    s += "</ul>";
    s += "<a href='config'>configu page</a>";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
}

void wifiConnected()
{
    needMqttConnect = true;
}

void configSaved()
{
    Serial.println("Configuration was updated.");
    needReset = true;
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
    Serial.println("Validating form.");
    bool valid = true;

    int l = webRequestWrapper->arg(mqttServerParam.getId()).length();
    if (l < 3)
    {
        mqttServerParam.errorMessage = "Please provide at least 3 characters!";
        valid = false;
    }

    return valid;
}

bool connectMqtt() {
    unsigned long now = millis();
    if (1000 > now - lastMqttConnectionAttempt)
    {
        // Do not repeat within 1 sec.
        return false;
    }
    Serial.println("Connecting to MQTT server...");
    if (!connectMqttOptions()) {
        lastMqttConnectionAttempt = now;
        return false;
    }
    Serial.println("MQTT Connected!");

    mqttClient.subscribe("mirtek/action");
    return true;
}

bool connectMqttOptions()
{
    bool result;
    if (mqttUserPasswordValue[0] != '\0')
    {
        result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue, mqttUserPasswordValue);
    }
    else if (mqttUserNameValue[0] != '\0')
    {
        result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue);
    }
    else
    {
        result = mqttClient.connect(iotWebConf.getThingName());
    }
    return result;
}

/*
void mqttMessageReceived(String& topic, String& payload)
{
    //Очищаем выходной буфер
    for (int i = 0; i < 61; i++) {
        resultbuffer[i] = 0;
    }
    //----------------------
    bool Request_Status = 0;
    Serial.println(payload);
    switch (payload.toInt()) {
    case 1:
        Serial.println("1 reseived from MQTT");
        RequestPacket_1();
        break;
    case 2:
        Serial.println("2 reseived from MQTT");
        RequestPacket_2();
        break;
    case 3:
        Serial.println("3 reseived from MQTT");
        RequestPacket_3();
        break;
    case 4:
        Serial.println("4 reseived from MQTT");
        RequestPacket_4();
        break;
    case 5:
        Serial.println("5 reseived from MQTT");
        RequestPacket_5();
        break;
    case 6:
        Serial.println("6 reseived from MQTT");
        RequestPacket_6();
        break;
    case 7:
        Serial.println("7 reseived from MQTT");
        RequestPacket_7();
        break;
    case 8:
        Serial.println("8 reseived from MQTT");
        RequestPacket_8();
        break;
    case 9:
        Serial.println("9 reseived from MQTT");
//        RequestPacket_9_pre();
//        delay(5000);
        RequestPacket_9();
        break;
        
    }

    for (int k = 0; k < 3; k++) {  //Пробуем 5 раз сделать запросы и получить ответы
        packetSender(); //отправляем пакет
        packetReceiver(); //принимаем и склеиваем пакет
        if ((myCRC == resultbuffer[bytecount - 2]) & ((atoi(MeterAdressValue) & 0xff) == resultbuffer[6]) & (((atoi(MeterAdressValue) >> 8) & 0xff) == resultbuffer[7])) {
            //if ((myCRC == resultbuffer[bytecount - 2]) {
            Serial.println("CRC & adress ok");
            switch (payload.toInt()) {
            case 1:

                break;
            case 2:

                break;
            case 3:

                break;
            case 4:

                break;
            case 5:
                packetParser_5_mqtt();
                break;
            case 6:

                break;
            case 7:
                //packetParser_7();
                break;
            case 9:
                packetParser_7_mqtt();
                break;
            }
            Request_Status = 1;
            mqttClient.publish("mirtek/Request_Status", "Ok");
            break;
        }

    }
    if (!Request_Status == 1) {
        mqttClient.publish("mirtek/Request_Status", "Error");
    }
}
*/
