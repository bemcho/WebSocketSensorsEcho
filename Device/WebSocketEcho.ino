#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "http_client.h"
#include "IoT_DevKit_HW.h"
#include "SystemVersion.h"
#include "Sensor.h"
#include "parson.h"
#include "WebSocketClient.h"
DevI2C *ext_i2c;
RGB_LED rgbLed;

static bool hasWifi = false;
static int userLEDState = 0;
static int rgbLEDState = 0;
static int rgbLEDR = 0;
static int rgbLEDG = 0;
static int rgbLEDB = 0;
static bool isWsConnected;
static int buttonAState = 0;
static int buttonBState = 0;
static char buffInfo[128];

static char webSocketServerUrl[] = "ws://192.168.100.8:2001/"; // or use ws://demos.kaazing.com/echo
static WebSocketClient* wsClient;
char wsBuffer[1024];
char wifiBuff[128];
int msgCount;
const static long RESET_TIMEOUT=5000;
long lastTimeSuccess=0L;

int calibratedGyro[3]={0,0,0};
int calibratedAccelero[3]={0,0,0};

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
    Screen.print(1, "Connect WS -> OK.");
    Screen.print(1, "A to calibrate.");
  }
  else
  {
    Screen.print(1, "Connect WS -> failed.");
    Screen.print(1, "B to reset WS.");
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

 if (getButtonAState())
  {
    // Button B is pushed down
    buttonAState = 1;
  }
  else
  {
    // Button B is released
    if (buttonAState)
    {
      calibrate();
    }
  }

  if (getButtonBState())
  {
    // Button B is pushed down
    buttonBState = 1;
  }
  else
  {
    // Button B is released
    if (buttonBState)
    {
      resetNet();
    }
  }

  if (hasWifi)
  {
    if (!isWsConnected)
    {
      connectWebSocket();
    }

    if (isWsConnected)
    {
      readAndSendData();
    }else{
      Screen.print("Sleeping ...");
      delay(1000);
    }

    if(SystemTickCounterRead()-lastTimeSuccess > RESET_TIMEOUT){
        resetNet();
        lastTimeSuccess=SystemTickCounterRead();
      }
  }

  delay(100);
}
/**
 * Takes current values of sensors (gyro,accelero) and threats them as zero
 */
void calibrate(){
  Screen.print("Calibrating ...");

  int x, y, z;
  getDevKitGyroscopeValue(&x, &y, &z);
  calibratedGyro[0] = x;
  calibratedGyro[1] = y;
  calibratedGyro[2] = z;

getDevKitAcceleratorValue(&x, &y, &z);
  calibratedAccelero[0] = x;
  calibratedAccelero[1] = y;
  calibratedAccelero[2] = z;
}

void resetNet()
{
  calibratedGyro[0]=0;
  calibratedGyro[1]=0;
  calibratedGyro[2]=0;

  calibratedAccelero[0]=0;
  calibratedAccelero[1]=0;
  calibratedAccelero[2]=0;

  delete wsClient;
  wsClient = NULL;
  connectWebSocket();
  buttonBState = 0;
  Serial.print("WS reseting ...");
}

void readAndSendData()
{
  char state[2024]={0};
  readSensors(state);
  // Send message to WebSocket server
  int res = wsClient->send(state,strlen(state));
  if (res > 0)
  {
    Screen.print(0, "WSend -> OK");
    lastTimeSuccess=SystemTickCounterRead() ;
  }
  else
  {
    Screen.print(0, "WSend -> failed:");
    Screen.print(1, "B to reset WS.");
  }

// Receive message from WebSofcket Server
  bool isEndOfMessage = false;
  WebSocketReceiveResult *recvResult = NULL;

  recvResult = wsClient->receive(wsBuffer, sizeof(wsBuffer));

  if (recvResult && recvResult->length > 0)
  {
    int len = recvResult->length;
    isEndOfMessage = recvResult->isEndOfMessage;
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

void showIpAddress(char resultJson[])
{
  sprintf(resultJson,"\"ipAddress\":\"%s\"",WiFi.localIP().get_address());
}

void showMotionGyroSensor(char resultJson[])
{
  int x, y, z;
  getDevKitGyroscopeValue(&x, &y, &z);
  x-=calibratedGyro[0];
  y-=calibratedGyro[1];
  z-=calibratedGyro[2];
  sprintf(resultJson, "\"gyroscope\":[ %d, %d, %d ]", x, y, z);
}

void showMotionAccelSensor(char resultJson[])
{
  int x, y, z;
  getDevKitAcceleratorValue(&x, &y, &z);
  x-=calibratedAccelero[0];
  y-=calibratedAccelero[1];
  z-=calibratedAccelero[2];
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