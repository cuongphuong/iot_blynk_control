

#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPLT6qd46Ps"
#define BLYNK_TEMPLATE_NAME "NodeMCU"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>

// Using for check config finished
bool settingFlg = false;
bool bkynkSettingFlg = false;
bool lastConnectInternetStatus = true;
bool hasConnectedInternet = false;
bool hasNotifiResetTime = false;

// Config button state
const int buttonPin = D3;      // GPIO0 - D3
int setupButtonState = 0;      // Current state of the button
int setUpLastButtonState = 0;  // Previous state of the button
unsigned long buttonPressTime; // Time the button was pressed

// Config auto reset
unsigned long previousMillis = 0;               // Biến lưu trữ thời điểm trước đó
long interval = 2 * 60 * 60 * 1000;             // Default khoản thời gian tính bằng miliseconds (2 giờ)
long intervalNotify = (2 * 60 - 1) * 60 * 1000; // Thời gian cảnh báo

// Config check intenet
unsigned long prevInternetCheckMillis = 0;
const long intervalInternetCheck = 30000;

// Device PIN
int device1 = D1;
int device2 = D2;
int device3 = D8;

// Device state
bool lastDevice1State = LOW;
bool lastDevice2State = LOW;
bool lastDevice3State = LOW;

// Button PIN
int button1 = D5;
int button2 = D6;
int button3 = D7;

// Button state
int lastButton1State = HIGH;
int lastButton2State = HIGH;
int lastButton3State = HIGH;

// Service management
ESP8266WebServer server(80);
WiFiManager wifiManager;
Preferences preferences;
HTTPClient http;
WiFiClient wifiClient;
DNSServer dnsServer;

/**
 * Check hasConnected to internet
 */
bool hasConnectedToInternet()
{
  // Gửi yêu cầu GET đến máy chủ Blynk Cloud
  if (http.begin(wifiClient, "http://www.google.com"))
  {
    int httpCode = http.GET();

    // Kiểm tra mã trạng thái HTTP
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.println("[HTTP] Connected to internet");
      http.end();

      hasConnectedInternet = true;
      return true;
    }
    else
    {
      hasConnectedInternet = false;
      Serial.printf("[HTTP] Failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.println("[HTTP] Unable to connect");
  }

  return false;
}

/**
 * API GET "/"
 * Display setting page
 */
void webHome()
{
  String html = "<!DOCTYPE html>\r\n<html>\r\n\r\n<head>\r\n    <meta charset=\"UTF-8\">\r\n    <title>Điều khiển thiết bị thông qua Blynk</title>\r\n    <style>\r\n        .form {\r\n            display: flex;\r\n            flex-direction: column;\r\n            align-items: center;\r\n            margin-top: 50px;\r\n        }\r\n\r\n        .form-group {\r\n            display: flex;\r\n            flex-direction: row;\r\n            align-items: center;\r\n            margin-bottom: 20px;\r\n        }\r\n\r\n        label {\r\n            text-align: left;\r\n            width: 100px;\r\n            /* Điều chỉnh kích thước label để các label có chiều rộng như nhau */\r\n            margin-right: 10px;\r\n        }\r\n\r\n        input[type=\"text\"],\r\n        input[type=\"number\"],\r\n        input[type=\"password\"] {\r\n            padding: 10px;\r\n            border-radius: 5px;\r\n            border: none;\r\n            background-color: #f2f2f2;\r\n            width: 250px;\r\n        }\r\n\r\n        input[type=\"submit\"] {\r\n            background-color: #4CAF50;\r\n            color: white;\r\n            border-radius: 5px;\r\n            border: none;\r\n            padding: 10px;\r\n            width: 100px;\r\n            cursor: pointer;\r\n        }\r\n    </style>\r\n</head>\r\n\r\n<body>\r\n    <div class=\"form\">\r\n        <div class=\"form-group\">\r\n            <label for=\"user\">User</label>\r\n            <input type=\"text\" id=\"user\">\r\n        </div>\r\n        <div class=\"form-group\">\r\n            <label for=\"password\">Password</label>\r\n            <input type=\"password\" id=\"password\">\r\n        </div>\r\n        <div class=\"form-group\">\r\n            <label for=\"token\">Token</label>\r\n            <input type=\"text\" id=\"token\">\r\n        </div>\r\n        <div class=\"form-group\">\r\n            <label for=\"token\">Re-set time(h)</label>\r\n            <input type=\"number\" value=\"24\" id=\"resettime\">\r\n        </div>\r\n        <input onclick=\"save()\" type=\"submit\" value=\"Kết nối\">\r\n    </div>\r\n    <script>\r\n        document.addEventListener(\"DOMContentLoaded\", function (event) {\r\n            var xhr = new XMLHttpRequest();\r\n            xhr.open(\'GET\', \'/info\');\r\n            xhr.onload = function () {\r\n                if (xhr.status === 200) {\r\n                    let res = JSON.parse(xhr.responseText);\r\n                    document.getElementById(\'token\').value = res.token;\r\n                    document.getElementById(\'user\').value = res.username;\r\n                    document.getElementById(\'password\').value = res.pass;\r\n                    document.getElementById(\'resettime\').value = res.resettime;\r\n                }\r\n            };\r\n            // Send\r\n            xhr.send();\r\n        });\r\n        function save() {\r\n            var xhr = new XMLHttpRequest();\r\n            xhr.open(\'POST\', \'/save\');\r\n\r\n            // Set callback action\r\n            xhr.onload = function () {\r\n                if (xhr.status === 200) {\r\n                    alert(\'Đã lưu thông tin!\');\r\n                } else {\r\n                    alert(\'Lỗi, thử lại!\');\r\n                }\r\n            };\r\n\r\n            // Send\r\n            const formData = new FormData();\r\n            formData.append(\"token\", document.getElementById(\'token\').value);\r\n            formData.append(\"username\", document.getElementById(\'user\').value);\r\n            formData.append(\"pass\", document.getElementById(\'password\').value);\r\n            formData.append(\"resettime\", document.getElementById(\'resettime\').value);\r\n            xhr.send(formData);\r\n        }\r\n    </script>\r\n</body>\r\n\r\n</html>";
  server.send(200, "text/html", html);
}

/**
 * API GET "/info"
 * Get setting info
 */
void getConfigInfo()
{
  String myToken = preferences.getString("token", "undefined");
  String username = preferences.getString("username", "undefined");
  String pass = preferences.getString("pass", "undefined");
  String resettime = preferences.getString("resettime", "undefined");
  server.send(200, "application/json", "{\"token\": \"" + myToken + "\", \"username\": \"" + username + "\", \"pass\": \"" + pass + "\", \"resettime\": \"" + resettime + "\"}");
}

/**
 * API GET "/save"
 * Get setting info
 */
void saveConfigInfo()
{
  String token = server.arg("token");
  String username = server.arg("username");
  String pass = server.arg("pass");
  String resettime = server.arg("resettime");

  if (token.length() > 0 && username.length() && pass.length())
  {
    preferences.putString("token", token);
    preferences.putString("username", username);
    preferences.putString("pass", pass);
    preferences.putString("resettime", resettime);

    // Setting finish
    settingFlg = true;
    server.send(200, "application/json", "{\"message\": \"success\"}");

    // Delay 1s before reset
    delay(1000);

    // Reset
    ESP.reset();
  }
  else
  {
    server.send(400, "application/json", "{\"key\": \"faill\"}");
  }
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V1, V2, V3);
}

BLYNK_WRITE(V1)
{
  if (param.asInt())
  {
    digitalWrite(device1, HIGH);
    lastDevice1State = true;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 1 (D1) bật");
  }
  else
  {
    digitalWrite(device1, LOW);
    lastDevice1State = false;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 1 (D1) tắt");
  }
}

BLYNK_WRITE(V2)
{
  if (param.asInt())
  {
    digitalWrite(device2, HIGH);
    lastDevice2State = true;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 2 (D2) bật");
  }
  else
  {
    digitalWrite(device2, LOW);
    lastDevice2State = false;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 2 (D2) tắt");
  }
}

BLYNK_WRITE(V3)
{
  if (param.asInt())
  {
    digitalWrite(device3, HIGH);
    lastDevice3State = true;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 3 (D8) bật");
  }
  else
  {
    digitalWrite(device3, LOW);
    lastDevice3State = false;

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Thiết bị 3 (D8) tắt");
  }
}

/**
 * Start a config app
 */
void setupInfomation()
{
  // Set flg false
  settingFlg = false;

  // Blink light
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  // Create a sharepoint for config
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Connect to setting infomation.");

  // Start the server
  server.on("/", HTTP_GET, webHome);
  server.on("/info", HTTP_GET, getConfigInfo);
  server.on("/save", HTTP_POST, saveConfigInfo);
  server.begin();

  int startTime = millis();
  while (true)
  {
    server.handleClient();

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    if (settingFlg)
    {
      break;
    }

    // Tự thoát mode config sau 2p
    if (millis() - startTime > 120000)
    {

      if (hasConnectedInternet)
        Blynk.virtualWrite(V6, "Auto exit setup mode...");

      ESP.reset(); // Thực hiện reset
    }
  }
}

bool checkConnectWifi()
{
  String myToken = preferences.getString("token", "undefined");
  String username = preferences.getString("username", "undefined");
  String pass = preferences.getString("pass", "undefined");

  Serial.println("Cache info: " + username + "/" + pass + "/" + myToken);

  WiFi.begin(username, pass);

  int tryNum = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Try connect...");
    tryNum++;

    if (tryNum == 10)
    {
      return false;
    }
  }

  return true;
}

/**
 * Try to connect Blynk
 */
void reconnectBlynk()
{
  // Disconect
  Blynk.disconnect();

  // Get blynk token
  String myToken = preferences.getString("token", "undefined");
  String username = preferences.getString("username", "undefined");
  String pass = preferences.getString("pass", "undefined");

  // Re connect
  Blynk.begin(myToken.c_str(), WiFi.SSID().c_str(), WiFi.psk().c_str());
}

/**
 * Config blynk infomation
 */
bool configBlynk()
{

  // Check connnect wifi
  if (!checkConnectWifi())
  {
    return false;
  }

  // Get blynk token
  String myToken = preferences.getString("token", "undefined");
  String username = preferences.getString("username", "undefined");
  String pass = preferences.getString("pass", "undefined");
  String resettime = preferences.getString("resettime", "undefined");

  // Set reset time
  interval = resettime.toInt() * 60 * 60 * 1000;             // Default khoản thời gian tính bằng miliseconds (2 giờ)
  intervalNotify = (resettime.toInt() * 60 - 1) * 60 * 1000; // Thời gian cảnh báo

  // Show info
  Serial.println(WiFi.localIP());

  // Check connect wifi
  hasConnectedToInternet();

  // Start Blynk
  if (hasConnectedInternet)
    Blynk.begin(myToken.c_str(), WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Default set of led
  digitalWrite(LED_BUILTIN, HIGH);

  // Change state of 3 PIN to off
  if (hasConnectedInternet)
  {
    // Blynk.virtualWrite(V3, LOW);
    // Blynk.virtualWrite(V1, LOW);
    // Blynk.virtualWrite(V2, LOW);
    int minute = resettime.toInt() * 60;
    String msg = "Has connected (Next " + String(minute) + " minute).";
    Blynk.virtualWrite(V6, msg);
  }

  // Change status
  bkynkSettingFlg = true;

  return true;
}

void handleButton1()
{
  // Button was just pressed
  int button1Input = digitalRead(button1);
  if (button1Input == LOW && lastButton1State == HIGH)
  {
    lastDevice1State = !lastDevice1State;
    if (lastDevice1State)
    {
      digitalWrite(device1, HIGH);

      if (hasConnectedInternet && Blynk.connected())
      {
        Blynk.virtualWrite(V1, HIGH);
        Blynk.virtualWrite(V6, "Thiết bị 1 (D1) bật");
      }
    }
    else
    {
      digitalWrite(device1, LOW);

      if (hasConnectedInternet && Blynk.connected())
      {
        Blynk.virtualWrite(V1, LOW);
        Blynk.virtualWrite(V6, "Thiết bị 1 (D1) tắt");
      }
    }
  }
  lastButton1State = button1Input;
}

void handleButton2()
{
  // Button was just pressed
  int button2Input = digitalRead(button2);
  if (button2Input == LOW && lastButton2State == HIGH)
  {
    lastDevice2State = !lastDevice2State;
    if (lastDevice2State)
    {
      digitalWrite(device2, HIGH);

      if (hasConnectedInternet && Blynk.connected())
      {
        Blynk.virtualWrite(V2, HIGH);
        Blynk.virtualWrite(V6, "Thiết bị 2 (D2) bật");
      }
    }
    else
    {
      digitalWrite(device2, LOW);

      if (hasConnectedInternet && Blynk.connected())
      {
        Blynk.virtualWrite(V2, LOW);
        Blynk.virtualWrite(V6, "Thiết bị 2 (D2) tắt");
      }
    }
  }
  lastButton2State = button2Input;
}

void handleButton3()
{
  // Button was just pressed
  int button3Input = digitalRead(button3);
  if (button3Input == LOW && lastButton3State == HIGH)
  {
    lastDevice3State = !lastDevice3State;
    if (lastDevice3State)
    {
      digitalWrite(device3, HIGH);

      if (hasConnectedInternet)
      {
        Blynk.virtualWrite(V3, HIGH);
        Blynk.virtualWrite(V6, "Thiết bị 3 (D8) bật");
      }
    }
    else
    {
      digitalWrite(device3, LOW);

      if (hasConnectedInternet)
      {
        Blynk.virtualWrite(V3, LOW);
        Blynk.virtualWrite(V6, "Thiết bị 3 (D8) tắt");
      }
    }
  }
  lastButton3State = button3Input;
}

void handlePressButton()
{
  handleButton1();
  handleButton2();
  handleButton3();
}

void mainProcess()
{
  if (hasConnectedInternet)
  {
    Blynk.run();
  }

  handlePressButton();
}

void run()
{
  setupButtonState = digitalRead(buttonPin);

  // Button was just pressed
  if (setupButtonState == LOW && setUpLastButtonState == HIGH)
  {
    buttonPressTime = millis();
    Serial.println("Presss button...");

    if (hasConnectedInternet)
      Blynk.virtualWrite(V6, "Presss button...");
  }
  else if ((setupButtonState == LOW && setUpLastButtonState == LOW) || !settingFlg)
  {
    // Button is being held down
    if (millis() - buttonPressTime > 1500 || !settingFlg)
    {
      // Config user infomation
      if (hasConnectedInternet)
        Blynk.virtualWrite(V6, "Setup mode...");

      Serial.println("Setup mode...");
      setupInfomation();
    }
  }
  else if (setupButtonState == HIGH && setUpLastButtonState == LOW)
  {
    // Button was just released
  }
  else
  {
    if (!bkynkSettingFlg)
    {
      if (configBlynk())
      {
        settingFlg = true;
        Serial.println("Config successfully.");
      }
      else
      {
        settingFlg = false;
        delay(10000);
        return;
      }
    }

    mainProcess();
  }

  // Remember the current button state for the next loop
  setUpLastButtonState = setupButtonState;
}

void setup()
{
  Serial.begin(115200);

  // Start cache object
  preferences.begin("NODEMCU", false);

  // Config PIN mode
  pinMode(LED_BUILTIN, OUTPUT);

  // Confing device mode
  pinMode(device1, OUTPUT);
  pinMode(device2, OUTPUT);
  pinMode(device3, OUTPUT);

  // Confing device mode
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);

  // Check cache config infomation
  String myToken = preferences.getString("token", "undefined");
  String username = preferences.getString("username", "undefined");
  String pass = preferences.getString("pass", "undefined");

  // Pass setting info
  if (myToken != "undefined" && username != "undefined" && pass != "undefined")
  {
    settingFlg = true;
  }
}

void syncStatusDevice()
{
  int statusDevice1 = digitalRead(device1);
  int statusDevice2 = digitalRead(device2);
  int statusDevice3 = digitalRead(device3);

  // Send device status to BLynk cloud
  Blynk.virtualWrite(V1, statusDevice1);
  Blynk.virtualWrite(V2, statusDevice2);
  Blynk.virtualWrite(V3, statusDevice3);
}

void loop()
{
  // Serial.print("Running.../");
  // Serial.print(hasConnectedInternet);
  // Serial.print("/");
  // Serial.print(lastConnectInternetStatus);
  // Serial.print("/");
  // Serial.println(Blynk.connected());

  run();

  // Get time running
  unsigned long currentInternetCheckMillis = millis();

  // Check connect intenet (If than intervalInternetCheck (30s))
  if (currentInternetCheckMillis - prevInternetCheckMillis > intervalInternetCheck)
  {
    // Turn on led
    digitalWrite(LED_BUILTIN, LOW);
    prevInternetCheckMillis = currentInternetCheckMillis;

    // Get status connect
    Serial.println("Check connect intenet...");
    hasConnectedToInternet();

    // Re-connect if fail
    if (hasConnectedInternet && (!lastConnectInternetStatus || !Blynk.connected()))
    {
      Serial.println("Re-connect to Blynk cloud...");
      reconnectBlynk();
      // syncStatusDevice();
      Blynk.syncVirtual(V1, V2, V3);
    }

    // Set status last check connect
    lastConnectInternetStatus = hasConnectedInternet;

    // Turn off led
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // Send system data
  int freeHeapKb = ESP.getFreeHeap() / 1024;

  if (hasConnectedInternet)
  {
    Blynk.virtualWrite(V5, currentInternetCheckMillis / 1000);
    Blynk.virtualWrite(V7, freeHeapKb);
  }

  // Check reset
  if (currentInternetCheckMillis - previousMillis >= intervalNotify &&
      !hasNotifiResetTime)
  {
    hasNotifiResetTime = true;
    Blynk.virtualWrite(V6, "Tự khởi động lại sau 1p nữa.");
  }

  // Check reset
  if ((currentInternetCheckMillis - previousMillis >= interval) || freeHeapKb <= 5)
  {
    Serial.println("Reset.");
    Blynk.virtualWrite(V6, "Reseting...");
    ESP.reset(); // Thực hiện reset
  }
}