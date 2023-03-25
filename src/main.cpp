#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <BlynkSimpleEsp8266.h>

const int buttonPin = 0;       // GPIO0 - D3
int buttonState = 0;           // Current state of the button
int lastButtonState = 0;       // Previous state of the button
unsigned long buttonPressTime; // Time the button was pressed
bool leftConfigMode = true;
bool hasBkynkConfig = false;

ESP8266WebServer server(80);
WiFiManager wifiManager;
Preferences preferences;

void configBlynk()
{
  // Get blynk token
  String myToken = preferences.getString("token", "undefined");
  Serial.println("Your token: " + myToken);

  // Start Blynk
  Blynk.begin(myToken.c_str(), WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Default set of led
  digitalWrite(LED_BUILTIN, HIGH);

  // Change status
  hasBkynkConfig = true;
}

void setup()
{
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // Reset cấu hình wifi trước đó (nếu có)
  wifiManager.resetSettings();
  wifiManager.autoConnect("NodeMCU");

  // start cache config
  preferences.begin("NODEMCU", false);
}

void mainLoop()
{
  Blynk.run();
}

void handleRoot()
{
  String html = "<html lang=en><meta charset=UTF-8><meta content=\"IE=edge\"http-equiv=X-UA-Compatible><meta content=\"width=device-width,initial-scale=1\"name=viewport><title>Blynk Settings</title><body><style>@keyframes button-loading{20%{color:transparent;transform:scale(1,1)}40%{border-color:#5585ff;background-color:transparent;transform:scale(1,1)}60%{transform:scale(.7,1.1);margin-left:1.25rem;width:2.5rem;text-indent:-.6125rem;color:transparent;border-color:#5585ff;background-color:#5585ff}80%{transform:scale(1,1)}100%{margin-left:1.25rem;width:2.5rem;background-color:#5585ff;border-color:#5585ff;color:transparent}}@keyframes button-dot-intro{0%{opacity:0}60%{opacity:1;transform:scale(1,1)}100%{transform:scale(.75,.75)}}@keyframes button-dot-pulse{0%{opacity:1;transform:scale(.75,.75)}15%{transform:scale(.85,.85)}45%{transform:scale(.75,.75)}55%{transform:scale(.95,.95)}85%{transform:scale(.75,.75)}100%{opacity:1;transform:scale(.75,.75)}}@keyframes button-ready{0%{margin-left:1.25rem;width:2.5rem}10%{background-color:#5585ff;border-color:#5585ff}70%{margin:0;width:7.25rem;background-color:#fff;transform:scale(1.1,1.1)}100%{margin:0;width:7rem;border-color:#8cce1e;background-color:#fff}}@keyframes button-dot-outro{0%{opacity:1}100%{opacity:0;transform:scale(1,1)}}@keyframes button-ready-label{0%{opacity:0}100%{opacity:1}}:root{--input-color:#99a3ba;--input-border:#cdd9ed;--input-background:#fff;--input-placeholder:#cbd1dc;--input-border-focus:#275efe;--group-color:var(--input-color);--group-border:var(--input-border);--group-background:#eef4ff;--group-color-focus:#fff;--group-border-focus:var(--input-border-focus);--group-background-focus:#678efe}.form-field{display:block;width:100%;padding:8px 16px;line-height:25px;font-size:14px;font-weight:500;font-family:inherit;border-radius:6px;-webkit-appearance:none;color:var(--input-color);border:1px solid var(--input-border);background:var(--input-background);transition:border .3s ease}.form-field::-moz-placeholder{color:var(--input-placeholder)}.form-field:-ms-input-placeholder{color:var(--input-placeholder)}.form-field::placeholder{color:var(--input-placeholder)}.form-field:focus{outline:0;border-color:var(--input-border-focus)}.form-group{position:relative;display:flex;width:100%}.form-group .form-field,.form-group>span{white-space:nowrap;display:block}.form-group .form-field:not(:first-child):not(:last-child),.form-group>span:not(:first-child):not(:last-child){border-radius:0}.form-group .form-field:first-child,.form-group>span:first-child{border-radius:6px 0 0 6px}.form-group .form-field:last-child,.form-group>span:last-child{border-radius:0 6px 6px 0}.form-group .form-field:not(:first-child),.form-group>span:not(:first-child){margin-left:-1px}.form-group .form-field{position:relative;z-index:1;flex:1 1 auto;width:1%;margin-top:0;margin-bottom:0}.form-group>span{text-align:center;padding:8px 12px;font-size:14px;line-height:25px;color:var(--group-color);background:var(--group-background);border:1px solid var(--group-border);transition:background .3s ease,border .3s ease,color .3s ease}.form-group:focus-within>span{color:var(--group-color-focus);background:var(--group-background-focus);border-color:var(--group-border-focus)}html{box-sizing:border-box;-webkit-font-smoothing:antialiased}*{box-sizing:inherit}:after,:before{box-sizing:inherit}body{min-height:100vh;font-family:\"Mukta Malar\",Arial;display:flex;padding-top:50px;align-items:center;flex-direction:column;background:#f5f9ff}@media (min-width:600px){body{min-height:100vh;font-family:\"Mukta Malar\",Arial;display:flex;justify-content:center;align-items:center;flex-direction:column;background:#f5f9ff}}body .form-group{max-width:500px}body .form-group:not(:last-child){margin-bottom:32px}button{position:relative;overflow:hidden;width:7rem;color:#5585ff;border:2px solid #5585ff;background-color:transparent;cursor:pointer;line-height:2;margin:0;padding:0;border-radius:6px;font-size:1.125rem;text-transform:lowercase;outline:0;transition:transform 125ms}button:active{transform:scale(.9,.9)}button:after,button:before{position:absolute;opacity:0;border-radius:50%;background-color:#fff;top:50%;left:50%;margin-top:-1.125rem;margin-left:-1.125rem;width:2.25rem;height:2.25rem;content:\"\";z-index:1}button.loading{animation:button-loading .5s forwards}button.loading:before{opacity:1;animation:button-dot-intro .5s forwards}button.loading:after{opacity:0;animation:button-dot-pulse 1.5s infinite .5s}button.ready{text-indent:0;color:transparent;background-color:#5585ff;animation:button-ready 333ms forwards}button.ready:before{position:absolute;left:0;right:0;top:auto;margin:0;width:auto;height:auto;border-radius:0;background-color:transparent;color:#8cce1e;content:\"success\";opacity:0;z-index:2;animation:button-ready-label .5s forwards 275ms}button.ready:after{opacity:1;animation:button-dot-outro 333ms}</style><div class=form-group><span>Blynk token</span> <input class=form-field id=token name=token placeholder=\"type here...\"style=margin-right:5px;min-width:250px></div><button onclick=submit_F(this) type=submit>Save</button><script>document.addEventListener(\"DOMContentLoaded\", function (event) {\r\n            var xhr = new XMLHttpRequest();\r\n            xhr.open('GET', '/token');\r\n            xhr.onload = function () {\r\n                if (xhr.status === 200) {\r\n                    let res = JSON.parse(xhr.responseText);\r\n                    document.getElementById('token').value = res.token;\r\n                }\r\n            };\r\n\r\n            // Send\r\n            xhr.send();\r\n        });\r\n\r\n        function submit_F(obj) {\r\n            obj.className = 'loading';\r\n            setTimeout(function () {\r\n                save();\r\n            }, 200);\r\n        }\r\n\r\n        function save() {\r\n            var xhr = new XMLHttpRequest();\r\n            xhr.open('POST', '/login');\r\n            // Set callback action\r\n            xhr.onload = function () {\r\n                if (xhr.status === 200) {\r\n                    document.querySelector('button').className = 'ready';\r\n                } else {\r\n                    document.querySelector('button').className = '';\r\n                    alert('Save failed!');\r\n                }\r\n            };\r\n\r\n            // Send\r\n            const formData = new FormData();\r\n            formData.append(\"token\", document.getElementById('token').value);\r\n            xhr.send(formData);\r\n        }</script>";
  server.send(200, "text/html", html);
}

void handleLogin()
{
  String token = server.arg("token");
  Serial.print("Input token: ");
  Serial.println(token);

  if (token.length() > 0)
  {
    preferences.putString("token", token);
    leftConfigMode = true;

    server.send(200, "application/json", "{\"message\": \"success\"}");
  }
  else
  {
    server.send(400, "application/json", "{\"key\": \"faill\"}");
  }
}

void handleGetToken()
{
  // Get blynk token
  String myToken = preferences.getString("token", "undefined");
  server.send(200, "application/json", "{\"token\": \"" + myToken + "\"}");
}

void setupInfomation()
{

  // Phát ra 1 mạng wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP("NodeMCU Access Point");

  // Start the server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/token", HTTP_GET, handleGetToken);
  server.begin();

  Serial.print("Setting at IP: ");
  Serial.println(WiFi.localIP());
  leftConfigMode = false;
  hasBkynkConfig = false;

  while (true)
  {
    server.handleClient();

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    // Back to main loop
    if (leftConfigMode)
      break;
  }
}

void loop()
{
  buttonState = digitalRead(buttonPin);

  // Button was just pressed
  if (buttonState == LOW && lastButtonState == HIGH)
  {
    buttonPressTime = millis();
  }
  else if (buttonState == LOW && lastButtonState == LOW)
  {
    // Button is being held down
    if (millis() - buttonPressTime > 1500)
    {
      // Button has been held for more than 3 seconds
      Serial.println("Flash button held down for more than 3 seconds.");
      // Do something here
      setupInfomation();
    }
  }
  else if (buttonState == HIGH && lastButtonState == LOW)
  {
    // Button was just released
    // Do something here if necessary
    Serial.println("end setting.");
  }
  else
  {
    if (!hasBkynkConfig)
    {
      configBlynk();
      Serial.println("Config complete.");
    }

    mainLoop();
  }

  // Remember the current button state for the next loop
  lastButtonState = buttonState;
}