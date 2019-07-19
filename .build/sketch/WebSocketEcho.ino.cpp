#line 1 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
#line 1 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "http_client.h"
#include "IoT_DevKit_HW.h"
#include "SystemVersion.h"
#include "Sensor.h"
#include "parson.h"
#include "WebSocketClient.h"
DevI2C *ext_i2c;
LSM6DSLSensor *acc_gyro;
LIS2MDLSensor *magnetometer;
HTS221Sensor *ht_sensor;
IRDASensor *IrdaSensor;
LPS22HBSensor *pressureSensor;
RGB_LED rgbLed;

static bool hasWifi = false;
static int userLEDState = 0;
static int rgbLEDState = 0;
static int rgbLEDR = 0;
static int rgbLEDG = 0;
static int rgbLEDB = 0;
static bool isWsConnected;

static char buffInfo[128];
static int buttonAState = 0;
static int buttonBState = 0;

static char webSocketServerUrl[] = "ws://172.30.30.110:2001/"; // or use ws://demos.kaazing.com/echo
static WebSocketClient* wsClient;
char wsBuffer[1024];
char wifiBuff[128];
int msgCount;

#define READ_ENV_INTERVAL 2000

#line 37 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void initWiFi();
#line 57 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
bool connectWebSocket();
#line 80 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void setup();
#line 97 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void loop();
#line 122 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void initAll();
#line 263 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void readAndSendData();
#line 282 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void readSensors(char resultJson[]);
#line 315 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showIpAddress(char resultJson[]);
#line 320 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showMotionGyroSensor(char resultJson[]);
#line 327 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showMotionAccelSensor(char resultJson[]);
#line 334 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showPressureSensor(char resultJson[]);
#line 340 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showHumidTempSensor(char resultJson[]);
#line 350 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void showMagneticSensor(char resultJson[]);
#line 37 "/home/etomov/IoTWorkbenchProjects/projects/WebSocket/Device/WebSocketEcho.ino"
void initWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    snprintf(wifiBuff, 128, "WiFi Connected\r\n%s\r\n%s\r\n \r\n", WiFi.SSID(), ip.get_address());
    Screen.print(wifiBuff);

    delay(5000);
    hasWifi = true;
  }
  else
  {
    snprintf(wifiBuff, 128, "No WiFi\r\nEnter AP Mode\r\nto config\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

bool connectWebSocket()
{
  Screen.clean();
  Screen.print(0, "Connect to WS...");

  if (wsClient == NULL)
  {
    wsClient = new WebSocketClient(webSocketServerUrl);
  }

  isWsConnected = wsClient->connect();
  if (isWsConnected)
  {
    Screen.print(1, "Connect WS success.");
  }
  else
  {
    Screen.print(1, "Connect WS failed.");
  }

  return isWsConnected;
}

void setup()
{
  hasWifi = false;
  isWsConnected = false;
  msgCount = 0;
  
  initAll();
  int ret = initIoTDevKit(1);
  initWiFi();
  if (hasWifi)
  {
    connectWebSocket();
  }

  pinMode(USER_BUTTON_A, INPUT);
}

void loop()
{
  if (hasWifi)
  {
    if (!isWsConnected)
    {

      connectWebSocket();
    }

    if (isWsConnected)
    {
      readAndSendData();
    }
  }

  delay(1000);
}

bool i2cError = false;
int sensorMotion;
int sensorPressure;
int sensorMagnetometer;
int sensorHumidityAndTemperature;
int sensorIrda;
void initAll(){
  const char *firmwareVersion = getDevkitVersion();
  const char *wifiSSID = WiFi.SSID();
  int wifiRSSI = WiFi.RSSI();
  const char *wifiIP = (const char *)WiFi.localIP().get_address();
  const char *wifiMask = (const char *)WiFi.subnetMask().get_address();
  byte mac[6];
  char macAddress[18];
  WiFi.macAddress(mac);
  snprintf(macAddress, 18, "%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  try
  {
    ext_i2c = new DevI2C(D14, D15);
    i2cError = false;
  }
  catch (int error)
  {
    i2cError = true;
    sensorMotion = 0;
    sensorPressure = 0;
    sensorMagnetometer = 0;
    sensorHumidityAndTemperature = 0;
    sensorIrda = 0;
  }

  int sensorInitResult;

  if (!i2cError)
  {
    try
    {
      acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5);
      sensorInitResult = acc_gyro->init(NULL);
      acc_gyro->enableAccelerator();
      acc_gyro->enableGyroscope();

      if (sensorInitResult == 0)
      {
        sensorMotion = 1;
      }
      else
      {
        sensorMotion = 0;
      }
    }
    catch (int error)
    {
      sensorMotion = 0;
    }

    try
    {
      ht_sensor = new HTS221Sensor(*ext_i2c);
      sensorInitResult = ht_sensor->init(NULL);

      if (sensorInitResult == 0)
      {
        sensorHumidityAndTemperature = 1;
      }
      else
      {
        sensorHumidityAndTemperature = 0;
      }
    }
    catch (int error)
    {
      sensorHumidityAndTemperature = 0;
    }

    try
    {
      magnetometer = new LIS2MDLSensor(*ext_i2c);
      sensorInitResult = magnetometer->init(NULL);

      if (sensorInitResult == 0)
      {
        sensorMagnetometer = 1;
      }
      else
      {
        sensorMagnetometer = 0;
      }
    }
    catch (int error)
    {
      sensorMagnetometer = 0;
    }

    try
    {
      pressureSensor = new LPS22HBSensor(*ext_i2c);
      sensorInitResult = pressureSensor->init(NULL);

      if (sensorInitResult == 0)
      {
        sensorPressure = 1;
      }
      else
      {
        sensorPressure = 0;
      }
    }
    catch (int error)
    {
      sensorPressure = 0;
    }

    try
    {
      IrdaSensor = new IRDASensor();
      sensorInitResult = IrdaSensor->init();

      if (sensorInitResult == 0)
      {
        sensorIrda = 1;
      }
      else
      {
        sensorIrda = 0;
      }
    }
    catch (int error)
    {
      sensorIrda = 0;
    }
  }

  if (rgbLEDState == 0)
  {
    rgbLed.turnOff();
  }
  else
  {
    rgbLed.setColor(rgbLEDR, rgbLEDG, rgbLEDB);
  }

  pinMode(LED_USER, OUTPUT);
  digitalWrite(LED_USER, userLEDState);

}

void readAndSendData()
{
  char state[2024]={0};
  readSensors(state);
  // Send message to WebSocket server
  int res = wsClient->send(state,strlen(state));
  if (res > 0)
  {
    Screen.clean();
    Screen.print(0, "WSend OK:");
    msgCount++;
  }
  else
  {
    Screen.clean();
    Screen.print(0, "WSend failed:");
  }
}

void readSensors(char resultJson[])
{
  try
  {
    char resultIp[64] = {0};
    showIpAddress(resultIp);

    char resultHumidity[128] = {0}; 
    showHumidTempSensor(resultHumidity);

    char resultPressure[64] = {0}; 
    showPressureSensor(resultPressure);

    char resultGyro[64] = {0}; 
    showMotionGyroSensor(resultGyro);

    char resultAccele[64] = {0}; 
    showMotionAccelSensor(resultAccele);

    char resultMagnet[64] = {0};
    showMagneticSensor(resultMagnet);
    sprintf(resultJson, "{%s,%s,%s,%s,%s,%s}",resultIp ,resultHumidity,resultPressure, resultGyro,resultAccele,resultMagnet);
  }
  catch(int error)
  {
    Serial.print("*** Read sensor failed: \n");
    Serial.print(error);
  }
}

#define READ_ENV_INTERVAL 2000
static volatile uint64_t msReadEnvData = 0;

void showIpAddress(char resultJson[])
{
  sprintf(resultJson,"\"ipAddress\":\"%s\"",WiFi.localIP().get_address());
}

void showMotionGyroSensor(char resultJson[])
{
  int x, y, z;
  getDevKitGyroscopeValue(&x, &y, &z);
  sprintf(resultJson, "\"gyroscope\":[ %d, %d, %d ]", x, y, z);
}

void showMotionAccelSensor(char resultJson[])
{
  int x, y, z;
  getDevKitAcceleratorValue(&x, &y, &z);
  sprintf(resultJson, "\"accelerometer\":[ %d, %d, %d]" , x, y, z);
}

void showPressureSensor(char resultJson[])
{
  float pressure = getDevKitPressureValue();
  sprintf(resultJson, "\"environmentPressure\":%0.2f,\"pressureUnit\":\"hPa\"", pressure);
}

void showHumidTempSensor(char resultJson[])
{
  
  float tempC = getDevKitTemperatureValue(0);
  float tempF = tempC * 1.8 + 32;
  float humidity = getDevKitHumidityValue();

  sprintf(resultJson, "\"environmentTemp\":%0.2f,\"environmentTempUnit\":\"C\",\"humidity\":%0.2f", tempC, humidity);
}

void showMagneticSensor(char resultJson[])
{
  int x, y, z;
  getDevKitMagnetometerValue(&x, &y, &z);
  sprintf(resultJson, "\"magnetometer\":[ %d, %d, %d]", x, y, z);}
