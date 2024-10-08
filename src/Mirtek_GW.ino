//by Little_S@tan
//MQTT topics: "mirtek/Request_Status", "mirtek/action"
//#define MY_CC1101
// https://github.com/1technophile/OpenMQTTGateway/issues/329
// my cc1101 pinout   vcc-gd00-csn-sck-mosi-gd01-gd02-gnd
// esp8266 wemos mini 3.3v-D1--D8--D5--D7---D6---D2---gnd
 
#include <math.h>
#include <MQTT.h>
#include <SPI.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "CRC8.h"
#include "CRC.h"
#include <TimerMs.h>

#define MY_DEBUG
#ifdef  MY_DEBUG
#define DEBUG_LOG(msg, ...) DebugLog(msg, ##__VA_ARGS__)

enum State {
  DateTime = 0,
  SumT1T2,
  ConsVoltAmp,
  Wait
} RequestState;

int RequestMeterIdx;
int RequestAttempt;

void DebugLog(const char* msg, ...)
{
    char buff[256];
    va_list argptr;
    va_start(argptr, msg);
    vsnprintf(buff, sizeof(buff) , msg, argptr);
    Serial.println(buff);
}
#define DEBUG_PKTLOG(pkt, pktLen) DebugPktLog(pkt, pktLen)
void DebugPktLog(byte* pkt, unsigned pktLen )
{
    for (unsigned i = 0; i < pktLen; i++) {
        Serial.print(" ");
        Serial.print(pkt[i], HEX);
    }
    Serial.println();
}
#else
    #define DEBUG_LOG(msg, ...)
    #define DEBUG_PKTLOG(pkt, pktLen)
#endif


unsigned tmr_tele_time=30*1000; //раз в 30 cек опрашиваем cчетчики и отправляем информацию в mqtt

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
    unsigned DomoticzTextIdx;
    unsigned DomoticzP1Idx;
    unsigned DomoticzAmpersIdx;
    unsigned DomoticzVoltsIdx;
//TIME:  22:29:38
    unsigned  hour;
    unsigned  min;
    unsigned  sec;

//DATE:  2.7.2024
    unsigned  day;
    unsigned  month;
    unsigned  year; 

    float sum;
    float t1; 
    float t2;
	float a1;
	float a2;
	float a3;
	float v1;
	float v2;
	float v3;
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

TimerMs tmr(1000, 0, 0);               //инициализируем таймер ожидания ответа счетчика
TimerMs tmr_tele(tmr_tele_time, 0, 0); //инициализируем таймер, отправляющий значения в MQTT

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "Mirtek_GW";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678";

#define STRING_LEN 128
#define NUMBER_LEN 6
#define FLOATSZ    16

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "v2.1"

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

char DomoticzAmpersIdxValue1[NUMBER_LEN];
char DomoticzAmpersIdxValue2[NUMBER_LEN];
char DomoticzAmpersIdxValue3[NUMBER_LEN];
char DomoticzAmpersIdxValue4[NUMBER_LEN];

char DomoticzVoltsIdxValue1[NUMBER_LEN];
char DomoticzVoltsIdxValue2[NUMBER_LEN];
char DomoticzVoltsIdxValue3[NUMBER_LEN];
char DomoticzVoltsIdxValue4[NUMBER_LEN];


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
IotWebConfNumberParameter DomoticzAmpersIdx1 = IotWebConfNumberParameter("Ampers Idx", "DomoticzAmpersIdx1", DomoticzAmpersIdxValue1, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzVoltsIdx1 = IotWebConfNumberParameter("Volts Idx", "DomoticzVoltsIdx1", DomoticzVoltsIdxValue1, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group2 = IotWebConfParameterGroup("group2", "Настройки Cчетчика 2");
IotWebConfNumberParameter MeterAdress2 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress2", MeterAdressValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx2 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx2", DomoticzP1IdxValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzAmpersIdx2 = IotWebConfNumberParameter("Ampers Idx", "DomoticzAmpersIdx2", DomoticzAmpersIdxValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzVoltsIdx2 = IotWebConfNumberParameter("Volts Idx", "DomoticzVoltsIdx2", DomoticzVoltsIdxValue2, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group3 = IotWebConfParameterGroup("group3", "Настройки Cчетчика 3");
IotWebConfNumberParameter MeterAdress3 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress3", MeterAdressValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx3 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx3", DomoticzP1IdxValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzAmpersIdx3 = IotWebConfNumberParameter("Ampers Idx", "DomoticzAmpersIdx3", DomoticzAmpersIdxValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzVoltsIdx3 = IotWebConfNumberParameter("Volts Idx", "DomoticzVoltsIdx3", DomoticzVoltsIdxValue3, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

IotWebConfParameterGroup group4 = IotWebConfParameterGroup("group4", "Настройки Cчетчика 4");
IotWebConfNumberParameter MeterAdress4 = IotWebConfNumberParameter("Адрес счётчика", "MeterAdress4", MeterAdressValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzP1Idx4 = IotWebConfNumberParameter("P1 Smart Meter Idx", "DomoticzP1Idx4", DomoticzP1IdxValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzAmpersIdx4 = IotWebConfNumberParameter("Ampers Idx", "DomoticzAmpersIdx4", DomoticzAmpersIdxValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");
IotWebConfNumberParameter DomoticzVoltsIdx4 = IotWebConfNumberParameter("Volts Idx", "DomoticzVoltsIdx4", DomoticzVoltsIdxValue4, NUMBER_LEN, "0", "0..65534", "min='0' max='65534' step='1'");

bool iotWebConfInit(){
    iotWebConf.addSystemParameter(&telePeriod); 

    mqttGroup.addItem(&mqttEnableParam);
    mqttGroup.addItem(&mqttServerParam);
    mqttGroup.addItem(&mqttUserNameParam);
    mqttGroup.addItem(&mqttUserPasswordParam);

    group1.addItem(&MeterAdress1);
    group1.addItem(&DomoticzP1Idx1);
    group1.addItem(&DomoticzAmpersIdx1);
    group1.addItem(&DomoticzVoltsIdx1);
    
    group2.addItem(&MeterAdress2);
    group2.addItem(&DomoticzP1Idx2);
    group2.addItem(&DomoticzAmpersIdx2);
    group2.addItem(&DomoticzVoltsIdx2);

    group3.addItem(&MeterAdress3);
    group3.addItem(&DomoticzP1Idx3);
    group3.addItem(&DomoticzAmpersIdx3);
    group3.addItem(&DomoticzVoltsIdx3);

    group4.addItem(&MeterAdress4);
    group4.addItem(&DomoticzP1Idx4);
    group4.addItem(&DomoticzAmpersIdx4);
    group4.addItem(&DomoticzVoltsIdx4);

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
    bool validConfig = iotWebConf.init();
    DEBUG_LOG("Configuration: %d", validConfig);
    if (!validConfig)
    {
        DEBUG_LOG("Configuration is not valid!!! Update it!");
        iotWebConf.saveConfig();
/*
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

        DomoticzAmpersIdxValue1 [0] = '\0';
        DomoticzAmpersIdxValue2 [0] = '\0';
        DomoticzAmpersIdxValue3 [0] = '\0';
        DomoticzAmpersIdxValue4 [0] = '\0';

        DomoticzVoltsIdxValue1 [0] = '\0';
        DomoticzVoltsIdxValue2 [0] = '\0';
        DomoticzVoltsIdxValue3 [0] = '\0';
        DomoticzVoltsIdxValue4 [0] = '\0';
*/
    }

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });
    return validConfig;
}

void packetSend(byte tr[], int tc)  //функция отправки пакета
{
    ELECHOUSE_cc1101.SpiStrobe(0x33);  //Calibrate frequency synthesizer and turn it off
//    delay(10);
    ELECHOUSE_cc1101.SpiStrobe(0x3B);  // Flush the TX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x36);  // Exit RX / TX, turn off frequency synthesizer and exit
    ELECHOUSE_cc1101.SpiWriteReg(0x3e, 0xC4); //выставляем мощность 10dB
    DEBUG_LOG("Packet %d sent", tc);
//    DEBUG_LOG("Packet %d sent: %d", tc, *tr);
    delay(10);
    ELECHOUSE_cc1101.SendData(tr, tr[0] + 1); //отправляем пакет
    ELECHOUSE_cc1101.SpiStrobe(0x3A);  // Flush the RX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x34);  // Enable RX
//    DEBUG_PKTLOG(tr, tr[0] + 1);
}

// Request Type: 0x20, PacketLen: 0x0F
int CreateRequestPacket1(byte tr[], unsigned addr, byte code) {
    DEBUG_LOG("RequestPacket1: Addr: %d Code: %X", addr, code);
	
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
//    packetSend(tr);   //отправляем пакет
    return 3;
}

// Request Type: 0x21, PacketLen: 0x10
int CreateRequestPacket2(byte tr[], unsigned addr ,byte code, byte type) {
    DEBUG_LOG("RequestPacket2: Addr: %d Code: %X Type: %X", addr, code, type);
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
//    packetSend(tr);   //отправляем пакет
    return 4;
}

// функция приёма пакета (помещает его в resultbuffer[])
// packetType кол-во подпакетов в ответе - зависит от типа запроса (3 для запросов 1-4, 4 для запросов 5,6)
bool packetReceive(int packetType) {

    tmr.start();
    bytecount = 0;       // количество байтов в результирующем буфере (resultbuffer)
    byte result[61];
    unsigned resultBytes = 0; 

    int PackCount = 0;   // счётчик принятых пакетов
    byte buffer[4][61];  // массив буфферов пакетов
    int rssi = 0; 
    while (!tmr.tick() && PackCount < packetType && PackCount < 4) {
        delay(1);
        if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
            
            //Rssi Level in dBm
            //Serial.print("Rssi: ");
            //Serial.println(ELECHOUSE_cc1101.getRssi());
            rssi += ELECHOUSE_cc1101.getRssi();

            //Link Quality Indicator
            //Serial.print("LQI: ");
            //Serial.println(ELECHOUSE_cc1101.getLqi());
            

            ELECHOUSE_cc1101.ReceiveData(buffer[PackCount]);
            
            if( buffer[PackCount][0] > 60) {
                DEBUG_LOG("Packet %d, Income Buffer Len: %d", PackCount, buffer[PackCount][0]);
                DEBUG_PKTLOG(buffer[PackCount], buffer[PackCount][0] + 1);
                return false;
            }
    //        delay(5);
            ELECHOUSE_cc1101.SpiStrobe(0x36);  // Exit RX / TX, turn off frequency synthesizer and exit
            ELECHOUSE_cc1101.SpiStrobe(0x3A);  // Flush the RX FIFO buffer
            ELECHOUSE_cc1101.SpiStrobe(0x3B);  // Flush the TX FIFO buffer
            ELECHOUSE_cc1101.SpiStrobe(0x34);  // Enable RX
            PackCount++;
        }
    }
    if(PackCount == 0) {
        // no packets received
        DEBUG_LOG("ERROR: PackCount: 0");
        return false;
    }

    for(int p = 0; p < PackCount; p++ ){
        //подшиваем пакеты в общий пакет; buffer[p][0] - size of received packet 
        for (int i = 1; i <= buffer[p][0] && resultBytes < sizeof(result); i++) {
            byte b = buffer[p][i];
            if(b == 0x73 ){
                if(buffer[p][i+1] == 0x11){
                    result[resultBytes++] = 0x55;
                    i++;
                    continue;
                }
                if(buffer[p][i+1] == 0x22){
                    result[resultBytes++] = 0x73;
                    i++;
                    continue;
                }
                if(buffer[p][i+1] != 0x55){
                    DEBUG_LOG("NEW ESCAPE:0x73 %X", buffer[p][i+1]);
                }
            }
            result[resultBytes++] = b;
        }
    }

    if(resultBytes < 4) {
        DEBUG_LOG("ERROR: PackCount: %d Result Bytes: %d", PackCount, resultBytes);
        return false;
    }

    // Test CRC ------------------------------
#if 0
    for(byte j=0; j < 255; j++){
        crc.reset();
        crc.setPolynome(0xA9);
        bool find = false;
        for (unsigned i = 2; i < (resultBytes - 2); i++){
//            if(result[i] == 0x73 && result[i+1] == 0x11) { // escape 0x73 0x11
            if(result[i] == 0x73 && result[i+1] == 0x22) { // escape 0x73 0x22  
                find = true;
                crc.add(j);
                i++;
            }
            else{
                crc.add(result[i]);
            }
        }
        uint8_t myCRC = crc.getCRC();
        if(find == true && result[resultBytes - 2] == myCRC){
            DEBUG_LOG("CRC!!!!: %X ", j);
        }
    }
#else
        crc.reset();
        crc.setPolynome(0xA9);
        for (unsigned i = 2; i < (resultBytes - 2); i++){
             crc.add(result[i]);
        }
#endif

    uint8_t myCRC = crc.getCRC();
    if(result[resultBytes - 2] == myCRC){
        bytecount = resultBytes;
        for(unsigned i = 0; i < resultBytes; i++) {
           resultbuffer[i] = result[i];
        }
        DEBUG_LOG("CRC OK: Rssi: %d PackCount: %d Packet length: %d", rssi/PackCount, PackCount, resultBytes);
       
        return true;
    }
    else {
        DEBUG_LOG("ERROR: R_CRC: %X C_CRC: %X", result[resultBytes - 2], myCRC);
        DEBUG_LOG("ERROR: PackCount: %d Packet length: %d", PackCount, resultBytes);
        DEBUG_PKTLOG(result, resultBytes);
        return false;
    }
}

// RequestPacket(transmitt_byte,MeterAdress[0], 0x2B, 0x00);
bool parse_current_state(unsigned meterIdx) 
{ 
  bool ret = true;
//  if ( (bytecount == 45) and (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x2B) and (resultbuffer[12]==0x0) and (resultbuffer[44]==0x55)){
//     Serial.println("PARSING ERROR! parse_current_state");
//  }

    if(bytecount != 45) {DEBUG_LOG("parse_current_state: bytecount != 45: %d",bytecount); ret = false;}
    if(resultbuffer[0]!=0x73) {Serial.println("parse_current_state: resultbuffer[0]!=0x73");ret = false;}
    if(resultbuffer[1]!=0x55) {Serial.println("parse_current_state: resultbuffer[1]!=0x55");ret = false;}
    if(resultbuffer[2]!=0x1E) {Serial.println("parse_current_state: resultbuffer[2]!=0x1E" );ret = false;}
//    if(resultbuffer[8]!=0x05) {Serial.println("parse_current_state: resultbuffer[8]!=0x2B");ret = false;}
//    if(resultbuffer[17]!=0x1) {Serial.println("parse_current_state: resultbuffer[12]!=0x0");ret = false;}
    if(resultbuffer[44]!=0x55) {Serial.println("parse_current_state: resultbuffer[44]!=0x55");ret = false;}
   
    if (ret == false) {
        return ret;
    }
    
    Serial.print("Active:  ");
	Serial.print(resultbuffer[20], HEX); Serial.print(resultbuffer[19], HEX); Serial.print(resultbuffer[18], HEX); Serial.print("  ");
    float cons = float((resultbuffer[18] | (resultbuffer[19] << 8) | (resultbuffer[20] << 16))); //watts
    if(cons > 50000) // current power consumption < 50 kwatt
		ret = false;
	
	meter[meterIdx].cons = cons;
    Serial.println(cons);

    Serial.print("Reactive: ");
	Serial.print(resultbuffer[23], HEX); Serial.print(resultbuffer[22], HEX); Serial.print(resultbuffer[21], HEX); Serial.print("  ");
    Serial.println(float((resultbuffer[21] | (resultbuffer[22] << 8) | (resultbuffer[23] << 16) )));

    Serial.print("Freq: ");
    float freq = float((resultbuffer[24] | (resultbuffer[25] << 8)))/100;
	if(freq > 51 || freq < 49)
		ret = false;
	Serial.println(freq);
			
    Serial.print("Cos: ");
    Serial.println(float((resultbuffer[26] | (resultbuffer[27] << 8))) / 1000);

    Serial.print("V1: ");
    float v1 = float((resultbuffer[28] | (resultbuffer[29] << 8))) / 100;
    if(v1 > 400)
		ret = false;

	meter[meterIdx].v1 = v1;
	Serial.print(resultbuffer[29], HEX); Serial.print(resultbuffer[28], HEX); Serial.print("  ");
    Serial.println(v1);

    Serial.print("V2: ");
    float v2 = float((resultbuffer[30] | (resultbuffer[31] << 8))) / 100;
    if(v2 > 400)
		ret = false;
	meter[meterIdx].v2 = v2;
	Serial.print(resultbuffer[31], HEX); Serial.print(resultbuffer[30], HEX); Serial.print("  ");
    Serial.println(v2);

    Serial.print("V3:  ");
    float v3 = float((resultbuffer[32] | (resultbuffer[33] << 8))) / 100;
    if(v3 > 400)
		ret = false;
	meter[meterIdx].v3 = v3;
	Serial.print(resultbuffer[33], HEX); Serial.print(resultbuffer[32], HEX); Serial.print("  ");
    Serial.println(v3);

    Serial.print("I1  ");
    float a1 = float(resultbuffer[34] | (resultbuffer[35] << 8) | (resultbuffer[36] << 16)) / 1000;
    if(a1 > 100)
		ret = false;
	meter[meterIdx].a1 = a1;
	Serial.print(resultbuffer[36], HEX); Serial.print(resultbuffer[35], HEX); Serial.print(resultbuffer[34], HEX); Serial.print("  ");
    Serial.println(a1);

    Serial.print("I2  ");
    float a2 = float(resultbuffer[37] | (resultbuffer[38] << 8) | (resultbuffer[39] << 16)) / 1000;
    if(a2 > 100)
		ret = false;
	meter[meterIdx].a2 = a2;
	Serial.print(resultbuffer[39], HEX); Serial.print(resultbuffer[38], HEX); Serial.print(resultbuffer[37], HEX); Serial.print("  ");
    Serial.println(a2);

    Serial.print("I3  ");
    float a3 = float(resultbuffer[40] | (resultbuffer[41] << 8) | (resultbuffer[42] << 16)) / 1000;
    if(a3 > 100)
		ret = false;
	meter[meterIdx].a3 = a3;
	Serial.print(resultbuffer[42], HEX); Serial.print(resultbuffer[41], HEX); Serial.print(resultbuffer[40], HEX); Serial.print("  ");
    Serial.println(a3);
  return ret;
}

// RequestPacket(transmitt_byte,MeterAdress[0], 0x05, 0); 
// enum code		
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
bool parse_sum_t1_t2(unsigned meterIdx) {
  //if ( (bytecount == 45) and (resultbuffer[0]==0x73) and (resultbuffer[1]==0x55) and (resultbuffer[2]==0x1E) and (resultbuffer[8]==0x05) and (resultbuffer[17]==0x1) and (resultbuffer[44]==0x55) ){
    bool ret = true;
        if(bytecount != 45) {DEBUG_LOG("parse_sum_t1_t2: bytecount != 45: %d",bytecount); ret = false;}
        if(resultbuffer[0]!=0x73) {Serial.println("parse_sum_t1_t2: resultbuffer[0]!=0x73");ret = false;}
        if(resultbuffer[1]!=0x55) {Serial.println("parse_sum_t1_t2: resultbuffer[1]!=0x55");ret = false;}
        if(resultbuffer[2]!=0x1E) {Serial.println("parse_sum_t1_t2: resultbuffer[2]!=0x1E");ret = false;}
    //    if(resultbuffer[8]!=0x05) {Serial.println("parse_sum_t1_t2: resultbuffer[8]!=0x05");ret = false;}
    //    if(resultbuffer[17]!=0x1) {Serial.println("parse_sum_t1_t2: resultbuffer[17]!=0x1");ret = false;}
        if(resultbuffer[44]!=0x55) {Serial.println("parse_sum_t1_t2: resultbuffer[44]!=0x55");ret = false;}
    //}

    if (ret == false)
        return ret;

    Serial.print("SUM:  ");
	Serial.print(resultbuffer[26], HEX); Serial.print(resultbuffer[25], HEX); Serial.print(resultbuffer[24], HEX); Serial.print(resultbuffer[23], HEX); Serial.print("  ");
    float sum = float((resultbuffer[26] << 24) | (resultbuffer[25] << 16) | (resultbuffer[24] << 8) | resultbuffer[23]) * 10;
    meter[meterIdx].sum = sum;
	Serial.println(sum / 1000);

    Serial.print("T1:  ");
    float t1 = float((resultbuffer[30] << 24) | (resultbuffer[29] << 16) | (resultbuffer[28] << 8) | resultbuffer[27]) * 10;
	Serial.print(resultbuffer[30], HEX); Serial.print(resultbuffer[29], HEX); Serial.print(resultbuffer[28], HEX); Serial.print(resultbuffer[27], HEX); Serial.print("  ");
    meter[meterIdx].t1  = t1;
    Serial.println(t1 / 1000);

    Serial.print("T2:  ");
    float t2 = float((resultbuffer[34] << 24) | (resultbuffer[33] << 16) | (resultbuffer[32] << 8) | resultbuffer[31]) * 10;
	Serial.print(resultbuffer[34], HEX); Serial.print(resultbuffer[33], HEX); Serial.print(resultbuffer[32], HEX); Serial.print(resultbuffer[31], HEX); Serial.print("  ");
    meter[meterIdx].t2  = t2;
    Serial.println(t2 / 1000);   
    return ret;
}

bool parse_date_time(unsigned meterIdx) {
  //73 55 7 0 9 FF E9 99 1C A8 3 5B 0 29 4 0 1 1F A 16 79 55
// TIME
//    unsigned  hour;
//    unsigned  min;
//    unsigned  sec;

//DATE:  2.7.2024
//    unsigned  day;
//    unsigned  month;
//    unsigned  year;
    bool ret = true;

    if ( bytecount != 22 ){Serial.println("parse_date_time: bytecount != 22"); ret = false;}
    if(resultbuffer[0]!=0x73) {Serial.println("parse_date_time: resultbuffer[0]!=0x73");ret = false;}
    if(resultbuffer[1]!=0x55) {Serial.println("parse_date_time: resultbuffer[1]!=0x55");ret = false;}
    if(resultbuffer[21]!=0x55) {Serial.println("parse_date_time: resultbuffer[44]!=0x55");ret = false;}
    if(ret == false)
        return ret;

    Serial.print("TIME: ");
	Serial.print(resultbuffer[13], HEX); Serial.print(resultbuffer[14], HEX); Serial.print(resultbuffer[15], HEX); Serial.print("  ");
	Serial.print(resultbuffer[15]); Serial.print(":"); Serial.print(resultbuffer[14]); Serial.print(":"); Serial.println(resultbuffer[13]);
    meter[meterIdx].hour = resultbuffer[15];
    meter[meterIdx].min  = resultbuffer[14];
    meter[meterIdx].sec  = resultbuffer[13];


    Serial.print("DATE:  ");
	Serial.print(resultbuffer[19], HEX); Serial.print(resultbuffer[18], HEX); Serial.print(resultbuffer[17], HEX); Serial.print("  ");
 	Serial.print(resultbuffer[17]); Serial.print("."); Serial.print(resultbuffer[18]); Serial.print(".20"); Serial.println(resultbuffer[19]);

    meter[meterIdx].day   = resultbuffer[17];
    meter[meterIdx].month = resultbuffer[18];
    meter[meterIdx].year  = resultbuffer[19];
    return ret;
}

// TODO domoticzTextPublish
void domoticzTextPublish(unsigned meterIdx ){
    if(meter[meterIdx].t1 > 0 && meter[meterIdx].t2 > 0 && meter[meterIdx].cons > 0 ){
        char buffer[128];
        char t1_chr[FLOATSZ];
        char t2_chr[FLOATSZ];
        
        dtostrf(meter[meterIdx].t1, -FLOATSZ, 1, t1_chr);
        dtostrf(meter[meterIdx].t2, -FLOATSZ, 1, t2_chr);
 
        snprintf(buffer,sizeof(buffer),"{\"idx\":%d, \"nvalue\":0, \"svalue\":\"%s;%s; 0.0;0.0; %d;0\"}",
            meter[meterIdx].DomoticzTextIdx, t1_chr, t2_chr, unsigned(meter[meterIdx].cons) );
        Serial.print("Domotics: "); Serial.println(buffer);
        mqttClient.publish("domoticz/in", buffer);
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

  if(meter[meterIdx].t1 > 0 && meter[meterIdx].t2 > 0 && meter[meterIdx].cons > 0 ){
    char buffer[128];
 
    char t1_chr[FLOATSZ];
    char t2_chr[FLOATSZ];
    
    dtostrf(meter[meterIdx].t1, -FLOATSZ, 1, t1_chr);
    dtostrf(meter[meterIdx].t2, -FLOATSZ, 1, t2_chr);

    snprintf(buffer,sizeof(buffer),"{\"idx\":%d, \"nvalue\":0, \"svalue\":\"%s;%s; 0.0;0.0; %d;0\"}",
        meter[meterIdx].DomoticzP1Idx, t1_chr, t2_chr, unsigned(meter[meterIdx].cons) );
    Serial.print("Domotics: "); Serial.println(buffer);
    mqttClient.publish("domoticz/in", buffer);
  }
}

void domoticzAmpersPublish(unsigned meterIdx ){
  if(meter[meterIdx].a1 > 0 && meter[meterIdx].a2 > 0 && meter[meterIdx].a3 > 0 ){
    char buffer[128];
    char ampers_chr1[FLOATSZ];
    char ampers_chr2[FLOATSZ];
    char ampers_chr3[FLOATSZ];
    dtostrf(meter[meterIdx].a1, -FLOATSZ, 2, ampers_chr1);
    dtostrf(meter[meterIdx].a2, -FLOATSZ, 2, ampers_chr2);
    dtostrf(meter[meterIdx].a3, -FLOATSZ, 2, ampers_chr3);

    snprintf(buffer,sizeof(buffer), "{\"idx\":%d, \"nvalue\":0, \"svalue\":\"%s;%s;%s\"}",
        meter[meterIdx].DomoticzAmpersIdx, ampers_chr1, ampers_chr2, ampers_chr3 );
    Serial.print("Domotics: "); Serial.println(buffer);
    mqttClient.publish("domoticz/in", buffer);
  }
}

void domoticzVoltsPublish(unsigned meterIdx ){
  if(meter[meterIdx].v1 > 0 && meter[meterIdx].v2 > 0 && meter[meterIdx].v3 > 0 ){
    char buffer[128];
    snprintf(buffer,sizeof(buffer), "{\"idx\": %d, \"nvalue\": 0, \"svalue\":\"%d;%d;%d\"}",
        meter[meterIdx].DomoticzVoltsIdx, unsigned(meter[meterIdx].v1), unsigned(meter[meterIdx].v2), unsigned(meter[meterIdx].v3) );
    Serial.print("Domotics: "); Serial.println(buffer);
    mqttClient.publish("domoticz/in", buffer);
  }
}

void domoticzPublish(int mIdx){
    DEBUG_LOG("domoticzPublish: mqttClient.connected %d", mqttClient.connected());
    if (mqttClient.connected() == false ){
        connectMqtt();
    }
    if(meter[mIdx].DomoticzP1Idx != 0 && mqttClient.connected() ){
        domoticzP1Publish(mIdx);
    }
    if(meter[mIdx].DomoticzAmpersIdx != 0 && mqttClient.connected() ){
        domoticzAmpersPublish(mIdx);
    }
    if(meter[mIdx].DomoticzVoltsIdx != 0 && mqttClient.connected() ){
        domoticzVoltsPublish(mIdx);
    }
}

void requestMeters(int mIdx, State state) {
    int tryCount = 1;
    for(int i = 0; i < MAX_METERS; i++) {
        Serial.print("MeterAdress: "); Serial.println(meter[i].MeterAdress);
        if(meter[i].MeterAdress != 0){
            int packetType;
            // date time
            DEBUG_LOG("RequestPacket: date time");
            packetType = CreateRequestPacket1(transmitt_byte1, meter[i].MeterAdress, 0x1C);
            for(int tc = 0; tc < tryCount; tc++) { 
                packetSend(transmitt_byte1, tc);   //отправляем пакет
                if(packetReceive(packetType)){
                    parse_date_time(i);
                    break;
                }
                delay(100);
            }

            // T1, T2
            DEBUG_LOG("RequestPacket: T1, T2");
            packetType = CreateRequestPacket2(transmitt_byte, meter[i].MeterAdress, 0x05, 0); 
            for(int tc = 0 ; tc < tryCount; tc++) { 
                packetSend(transmitt_byte, tc);   //отправляем пакет
                if(packetReceive(packetType)){
                    parse_sum_t1_t2(i);
                    break;
                }
                delay(100);
            }
          
            // Consume Active Energy, Volts, Ampers
            DEBUG_LOG("RequestPacket: Consume Active Energy, Volts, Ampers");
            packetType = CreateRequestPacket2(transmitt_byte, meter[i].MeterAdress, 0x2b, 0); 
            for(int tc = 0; tc < tryCount; tc++) { 
                packetSend(transmitt_byte, tc);   //отправляем пакет
                if(packetReceive(packetType)){
                    if(parse_current_state(i)){
                        DEBUG_LOG("domoticzPublish0: mqttClient.connected %d", mqttClient.connected());
                        if (mqttClient.connected() == false ){
                            connectMqtt();
                        }
                        if(meter[i].DomoticzP1Idx != 0 && mqttClient.connected() ){
                            domoticzP1Publish(i);
                            meter[i].t1 = -1;
                            meter[i].t2 = -1;
                            meter[i].cons = -1;
                        }
                        if(meter[i].DomoticzAmpersIdx != 0 && mqttClient.connected() ){
                            domoticzAmpersPublish(i);
                            meter[i].a1 = -1;
                            meter[i].a2 = -1;
                            meter[i].a3 = -1;
                        }
                        if(meter[i].DomoticzVoltsIdx != 0 && mqttClient.connected() ){
                            domoticzVoltsPublish(i);
                            meter[i].v1 = -1;
                            meter[i].v2 = -1;
                            meter[i].v3 = -1;
                        }
                    }
                    DEBUG_LOG("domoticzPublish1: mqttClient.connected %d", mqttClient.connected());
                    break;
                }
                delay(100);
            }
        }
    }
}

bool requestMetersEx(int mIdx, State state) {
    int packetType;
    switch(state){
        case DateTime:
            DEBUG_LOG("RequestPacket: date time");
            packetType = CreateRequestPacket1(transmitt_byte1, meter[mIdx].MeterAdress, 0x1C);
            packetSend(transmitt_byte1, 0);
            if(packetReceive(packetType)){
                return parse_date_time(mIdx);
            }
            return false;
        case SumT1T2:
            DEBUG_LOG("RequestPacket: T1, T2");
            packetType = CreateRequestPacket2(transmitt_byte, meter[mIdx].MeterAdress, 0x05, 0); 
            packetSend(transmitt_byte, 0);
            if(packetReceive(packetType)){
                return parse_sum_t1_t2(mIdx);
            }
            return false;
        case ConsVoltAmp:
            // Consume Active Energy, Volts, Ampers
            DEBUG_LOG("RequestPacket: Consume Active Energy, Volts, Ampers");
            packetType = CreateRequestPacket2(transmitt_byte, meter[mIdx].MeterAdress, 0x2b, 0); 
            packetSend(transmitt_byte, 0);
            if(packetReceive(packetType)){
                return parse_current_state(mIdx);
            }
            return false;
        default:
            DEBUG_LOG("RequestPacket: Wait");
            return true;
    }
}


void setup() {
    Serial.begin(115200);
    iotWebConfInit();
    mqttClient.begin(mqttServerValue, net);
//    mqttClient.onMessage(mqttMessageReceived);
    Serial.println("Web is Ready.");


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

        meter[0].DomoticzAmpersIdx = atoi(DomoticzAmpersIdxValue1);
        meter[1].DomoticzAmpersIdx = atoi(DomoticzAmpersIdxValue2);
        meter[2].DomoticzAmpersIdx = atoi(DomoticzAmpersIdxValue3);
        meter[3].DomoticzAmpersIdx = atoi(DomoticzAmpersIdxValue4);

        meter[0].DomoticzVoltsIdx = atoi(DomoticzVoltsIdxValue1);
        meter[1].DomoticzVoltsIdx = atoi(DomoticzVoltsIdxValue2);
        meter[2].DomoticzVoltsIdx = atoi(DomoticzVoltsIdxValue3);
        meter[3].DomoticzVoltsIdx = atoi(DomoticzVoltsIdxValue4);
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

#define MAX_ATTEMPT 3

void processRequest(State nextState){
    DEBUG_LOG("------>processRequest: MeterIdx %d RequestState: %d RequestAttempt: %d", RequestMeterIdx, RequestState, RequestAttempt);

    if( meter[RequestMeterIdx].MeterAdress == 0){
        RequestMeterIdx++;
        if (RequestMeterIdx == MAX_METERS) {
            RequestState = Wait; // Wait next loop
            RequestMeterIdx = 0;
            RequestAttempt = 0;
            DEBUG_LOG("<----->processRequest: MAX_METERS 1");
        }
        else {
            RequestState = DateTime; // start net meter with DateTime
            RequestAttempt = 0;
        }
    }
    else { 
        if( requestMetersEx(RequestMeterIdx, RequestState)){ // receive valid data, switch to next state
            RequestState = nextState;
            if(RequestState == Wait){
                domoticzPublish(RequestMeterIdx);
                RequestMeterIdx++;
                if (RequestMeterIdx == MAX_METERS) {
                    RequestMeterIdx = 0;
                    DEBUG_LOG("<----->processRequest: MAX_METERS 2");
                }
            }
            RequestAttempt = 0; 
        } 
        else {
            RequestAttempt++;
            if(RequestAttempt == MAX_ATTEMPT){ // meter error after MAX_ATTEMPT
                RequestMeterIdx++;
                RequestAttempt = 0;
                if (RequestMeterIdx == MAX_METERS) {
                    RequestState = Wait; // Wait next loop
                    RequestMeterIdx = 0;
                    DEBUG_LOG("<----->processRequest: MAX_METERS 3");
                } 
                else{
                    RequestState = DateTime; // start net meter with DateTime
                }
            }
        }
    }
    DEBUG_LOG("<------processRequest: MeterIdx %d RequestState: %d RequestAttempt: %d", RequestMeterIdx, RequestState, RequestAttempt);
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
        else if (iotWebConf.getState() == iotwebconf::OnLine && mqttClient.connected() == false) {
            DEBUG_LOG("MQTT reconnect: connected %d", mqttClient.connected());
            connectMqtt();
        }
    }
    
    if ((cc1101_is_ready == true) && (iotWebConf.getState() == iotwebconf::OnLine) ){

#if 1    
        switch (RequestState)
        {
        case DateTime:
            processRequest(SumT1T2);
            break;
        case SumT1T2:
            processRequest(ConsVoltAmp);
            break;
        case ConsVoltAmp:
            processRequest(Wait);
            break;
        case Wait:
            if (tmr_tele.tick()) {
                RequestState = DateTime;
            }
            break;
        default:
            break;
        }
#else
        if (tmr_tele.tick()){ // Запрос информации по таймеру
            Serial.println("----->Request MIRTEK by timer");
            requestMeters(0, RequestState);
            Serial.println("<-----Request MIRTEK by timer");Serial.println("");
        }
#endif
    }
}

void handleRoot()
{
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }

#if 1
//TODO use partial page sending 
//https://htmled.it/redaktor/
//  https://stackoverflow.com/questions/57242057/sending-large-amounts-of-data-from-an-esp8266-server

   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   server.sendHeader("Pragma", "no-cache");
   server.sendHeader("Expires", "-1");
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   server.send(200, "text/html", "");

    String s = "<!DOCTYPE html><html lang=\"en\">";
	s += "<head>";
	s += "<meta charset=\"utf-8\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Mirtek-to-MQTT</title>";
    s += "<style type=\"text/css\">";
    s += "p {font-weight:bold;}";
 	s += "table {border:1px;width:300px;border-collapse:collapse;border-style:solid;background:white;color:black;}";
    s += "tr:nth-child(odd) {background-color:#D6EEEE;height:20px;}";
    s += "td {text-align:center;height:12px;}";
    s += "</style>";
	s += "</head>";
    server.sendContent(s);

	s = "<body>MirtekGateway";

    s += "<ul>";
    s += "<li>MQTT server: ";
    s += mqttServerValue;
    s += "</ul>";
 //   s += "<form action=\"config\"><button>Configure gateway</button></form>";
    s += "<button onclick=\"document.location='config'\">Configure gateway</button>";
 
    server.sendContent(s);
/*
struct {
    unsigned MeterAdress;
    unsigned DomoticzTextIdx;
    unsigned DomoticzP1Idx;
    unsigned DomoticzAmpersIdx;
    unsigned DomoticzVoltsIdx;
//TIME:  22:29:38
    unsigned  hour;
    unsigned  min;
    unsigned  sec;

//DATE:  2.7.2024
    unsigned  day;
    unsigned  month;
    unsigned  year; 

    float sum;
    float t1; 
    float t2;
	float a1;
	float a2;
	float a3;
	float v1;
	float v2;
	float v3;
    float cons;
} meter[MAX_METERS];
*/

    for(int mIdx=0; mIdx < 4 ; mIdx++ ){
        char buffer[128];
        if( meter[mIdx].MeterAdress == 0) {
            continue;
        }

//     "<p>Meter: 12345<br />2023-02-22 21:39:26<br />cons: 12345</p>"
        char chr1[FLOATSZ];
        char chr2[FLOATSZ];
        char chr3[FLOATSZ];
        dtostrf(meter[mIdx].cons, -FLOATSZ, 0, chr1);
        snprintf(buffer,sizeof(buffer),"<p>Meter address: %u Date: 20%u-%u-%u Time:%u:%u:%u<br/>Power consumption: %sW</p>",
            meter[mIdx].MeterAdress, meter[mIdx].year,meter[mIdx].month,meter[mIdx].day,meter[mIdx].hour,meter[mIdx].min,meter[mIdx].sec, chr1 );
        server.sendContent(buffer);
        s = "<table>";
        server.sendContent(s);

        s = "<tr><td><strong>Sum</strong></td><td><strong>T1</strong></td><td><strong>T2</strong></td></tr>";
        server.sendContent(s);
//        "<tr><td>123</td><td>234</td><td>345</td></tr>"
        dtostrf(meter[mIdx].sum/1000, -FLOATSZ, 0, chr1);
        dtostrf(meter[mIdx].t1/1000, -FLOATSZ, 0, chr2);
        dtostrf(meter[mIdx].t2/1000, -FLOATSZ, 0, chr3);
    
        snprintf(buffer,sizeof(buffer),"<tr><td>%s</td><td>%s</td><td>%s</td></tr>",
            chr1, chr2, chr3);
        server.sendContent(buffer);


        s = "<tr><td><strong>V1</strong></td><td><strong>V2</strong></td><td><strong>V3</strong></td></tr>";
        server.sendContent(s);
//      "<tr><td>123</td><td>234</td><td>345</td></tr>";
        dtostrf(meter[mIdx].v1, -FLOATSZ, 0, chr1);
        dtostrf(meter[mIdx].v2, -FLOATSZ, 0, chr2);
        dtostrf(meter[mIdx].v3, -FLOATSZ, 0, chr3);

        snprintf(buffer,sizeof(buffer),"<tr><td>%s</td><td>%s</td><td>%s</td></tr>",
            chr1,chr2,chr3);
        server.sendContent(buffer);

        s = "<tr><td><strong>A1</strong></td><td><strong>A1</strong></td><td><strong>A3</strong></td></tr>";
        server.sendContent(s);

        dtostrf(meter[mIdx].a1, -FLOATSZ, 2, chr1);
        dtostrf(meter[mIdx].a2, -FLOATSZ, 2, chr2);
        dtostrf(meter[mIdx].a3, -FLOATSZ, 2, chr3);
//      "<tr><td>123</td><td>234</td><td>345</td></tr>";
        snprintf(buffer,sizeof(buffer),"<tr><td>%s</td><td>%s</td><td>%s</td></tr>",
            chr1,chr2,chr3);
        server.sendContent(buffer);
        server.sendContent("</table>");
    }
    server.sendContent("</body></html>\n");

#else
    String s = "<!DOCTYPE html><html lang=\"en\">";
	s += "<head>";
	s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Mirtek-to-MQTT</title>";
	s += "</head>";
	s += "<body>Gateway";
    s += "<ul>";
    s += "<li>MQTT server: ";
    s += mqttServerValue;
    s += "</ul>";
    s += "<a href='config'>configure page</a>";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
#endif 

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

bool connectMqtt() {
    unsigned long now = millis();
    if (1000 > now - lastMqttConnectionAttempt)
    {
        // Do not repeat within 1 sec.
        return false;
    }
    DEBUG_LOG("Connecting to MQTT server...");
    //Serial.println("Connecting to MQTT server...");
    
    if (!connectMqttOptions()) {
        lastMqttConnectionAttempt = now;
        return false;
    }
    mqttClient.subscribe("mirtek/action");
 
    DEBUG_LOG("MQTT Connected!");
    return true;
}
