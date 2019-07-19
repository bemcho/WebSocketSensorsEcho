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

void initWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    snprintf(wifiBuff, 128, "WiFi Connected\r\n%s\r\n%s\r\n \r\n", WiFi.SSID(), ip.get_address());
    Screen.print(wifiBuff);

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