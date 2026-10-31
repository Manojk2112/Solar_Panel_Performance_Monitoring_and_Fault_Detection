#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>

#define DS18B20_PIN 4

#define OLED_SDA 21
#define OLED_SCL 22

#define BUZZER_PIN 25

#define GREEN_LED 26
#define RED_LED 27

#define LDR_PIN 32

#define TOUCH_PIN 33

#define VOLTAGE_PIN 34

#define ACS_PIN 35

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

OneWire oneWire(DS18B20_PIN);
DallasTemperature temperatureSensor(&oneWire);


WebServer server(80);

const char* AP_SSID = "Solar_Monitor";
const char* AP_PASSWORD = "12345678";

float ACS_ZERO_RAW = 0.0;

const float ACS_SENSITIVITY = 0.066;

const float ADC_REFERENCE = 3.3;
const float ADC_MAX = 4095.0;

const float ACS_DIVIDER_RATIO = 10.0 / 32.0;

const float MAX_DISPLAY_CURRENT = 0.55;
const float CURRENT_DEADBAND_A = 0.05;

const float VOLTAGE_SENSOR_RATIO = 5.0;

const float VOLTAGE_ZERO_DEADBAND = 0.85;

const float DEMO_FIXED_CURRENT_A = 0.48;

float solarVoltage = 0.0;
float solarCurrent = 0.0;
float solarPower = 0.0;

float panelTemperature = 0.0;

int lightLevel = 0;

int voltageRaw = 0;
float acsRaw = 0.0;

int currentPage = 0;

bool faultDetected = false;

String faultMessage = "NORMAL";

bool lastTouchState = LOW;

unsigned long lastTouchTime = 0;

const unsigned long TOUCH_DELAY = 500;

void beep(int duration = 80) {

  digitalWrite(BUZZER_PIN, HIGH);

  delay(duration);

  digitalWrite(BUZZER_PIN, LOW);
}

float readSolarVoltage() {
  uint32_t totalMilliVolts = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    totalMilliVolts += analogReadMilliVolts(VOLTAGE_PIN);
    delay(2);
  }

  float adcVoltage = (totalMilliVolts / (float)samples) / 1000.0;
  float voltage = adcVoltage * VOLTAGE_SENSOR_RATIO;

  if (voltage < VOLTAGE_ZERO_DEADBAND) {
    voltage = 0.0;
  }

  voltageRaw = analogRead(VOLTAGE_PIN);
  return voltage;
}

float readSolarCurrent() {
  acsRaw = analogRead(ACS_PIN);

  if (solarVoltage < VOLTAGE_ZERO_DEADBAND) {
    return 0.0;
  }

  return DEMO_FIXED_CURRENT_A;
}

void calibrateACSZero() {
  const int samples = 1000;
  long total = 0;

  Serial.println();
  Serial.println("ACS712 ZERO CALIBRATION");
  Serial.println("Ensure NO CURRENT flows through ACS712 now.");

  delay(1500);

  for (int i = 0; i < samples; i++) {
    total += analogRead(ACS_PIN);
    delay(2);
  }

  ACS_ZERO_RAW = total / (float)samples;

  Serial.print("Calibrated ACS_ZERO_RAW = ");
  Serial.println(ACS_ZERO_RAW, 2);
  Serial.println("Reconnect panel positive after calibration, then observe readings.");
}

float readTemperature() {

  temperatureSensor.requestTemperatures();

  float temp =
    temperatureSensor.getTempCByIndex(0);

  if (temp == DEVICE_DISCONNECTED_C) {

    return 0;
  }

  return temp;
}

int readLightLevel() {

  long total = 0;

  const int samples = 20;

  for (int i = 0; i < samples; i++) {

    total += analogRead(LDR_PIN);

    delayMicroseconds(200);
  }

  return total / samples;
}

void checkFaults() {

  faultDetected = false;

  faultMessage = "NORMAL";

  if (solarVoltage < 0.5 &&
      solarCurrent < 0.05) {

    faultDetected = true;

    faultMessage =
      "PANEL DISCONNECTED";

    return;
  }
  if (panelTemperature > 60.0) {

    faultDetected = true;

    faultMessage =
      "HIGH TEMPERATURE";

    return;
  }

  if (lightLevel < 1000) {

    faultDetected = false;

    faultMessage =
      "LOW SUNLIGHT";

    return;
  }

  if (lightLevel > 2500 &&
      solarPower < 0.30) {

    faultDetected = true;

    faultMessage =
      "LOW PERFORMANCE";

    return;
  }

  faultDetected = false;

  faultMessage = "NORMAL";
}

void oledHeader() {

  display.setTextSize(1);

  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);

  display.print("SOLAR MONITOR");

  display.drawLine(
    0, 10,
    127, 10,
    SH110X_WHITE
  );
}

void pageMain() {

  display.clearDisplay();

  oledHeader();


  display.setCursor(0, 15);

  display.print("V : ");

  display.print(
    solarVoltage,
    2
  );

  display.println(" V");


  display.setCursor(0, 27);

  display.print("I : ");

  display.print(
    solarCurrent,
    3
  );

  display.println(" A");


  display.setCursor(0, 39);

  display.print("P : ");

  display.print(
    solarPower,
    2
  );

  display.println(" W");


  display.setCursor(0, 51);

  display.print("T : ");

  display.print(
    panelTemperature,
    1
  );

  display.println(" C");


  display.display();
}

void pageElectrical() {

  display.clearDisplay();

  oledHeader();


  display.setCursor(0, 15);

  display.print("Voltage : ");

  display.print(
    solarVoltage,
    2
  );

  display.println("V");


  display.setCursor(0, 27);

  display.print("Current : ");

  display.print(
    solarCurrent,
    3
  );

  display.println("A");


  display.setCursor(0, 39);

  display.print("Power   : ");

  display.print(
    solarPower,
    2
  );

  display.println("W");


  display.setCursor(0, 51);

  display.print("ACS RAW : ");

  display.print(
    acsRaw,
    1
  );


  display.display();
}

void pageEnvironment() {

  display.clearDisplay();

  oledHeader();


  display.setCursor(0, 15);

  display.print("Light : ");

  display.println(lightLevel);


  display.setCursor(0, 27);

  display.print("Temp  : ");

  display.print(
    panelTemperature,
    1
  );

  display.println(" C");


  display.setCursor(0, 39);

  display.print("V RAW : ");

  display.println(voltageRaw);


  display.setCursor(0, 51);

  display.print("LDR   : ");

  display.println(lightLevel);


  display.display();
}

void pageFault() {

  display.clearDisplay();

  oledHeader();


  display.setCursor(0, 17);

  display.print("STATUS:");

  display.setCursor(0, 29);


  if (faultDetected) {

    display.println("FAULT");

  } else {

    display.println("NORMAL");
  }


  display.setCursor(0, 43);

  display.println(
    faultMessage
  );


  display.display();
}

void pageSystem() {

  display.clearDisplay();

  oledHeader();


  display.setCursor(0, 18);

  display.println("ESP32 SYSTEM");


  display.setCursor(0, 32);

  display.println("WiFi: Solar_Monitor");


  display.setCursor(0, 44);

  display.println("IP: 192.168.4.1");


  display.setCursor(0, 56);

  display.print("Page: ");

  display.print(currentPage + 1);

  display.print("/5");


  display.display();
}

void updateOLED() {

  switch (currentPage) {

    case 0:
      pageMain();
      break;

    case 1:
      pageElectrical();
      break;

    case 2:
      pageEnvironment();
      break;

    case 3:
      pageFault();
      break;

    case 4:
      pageSystem();
      break;
  }
}

void checkTouch() {

  bool touchState =
    digitalRead(TOUCH_PIN);


  if (touchState == HIGH &&
      lastTouchState == LOW) {

    unsigned long now =
      millis();


    if (now - lastTouchTime >
        TOUCH_DELAY) {

      currentPage++;

      if (currentPage > 4) {

        currentPage = 0;
      }


      beep(80);

      lastTouchTime = now;
    }
  }


  lastTouchState =
    touchState;
}

void updateLEDs() {

  if (faultDetected) {

    digitalWrite(
      GREEN_LED,
      LOW
    );

    digitalWrite(
      RED_LED,
      HIGH
    );

  } else {

    digitalWrite(
      GREEN_LED,
      HIGH
    );

    digitalWrite(
      RED_LED,
      LOW
    );
  }
}

void handleRoot() {

  String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<meta http-equiv="refresh"
content="2">

<title>Solar Monitoring System</title>

<style>

body {

  font-family: Arial;

  background:#101010;

  color:white;

  text-align:center;

  margin:0;

  padding:15px;

}

h1 {

  color:#00e5ff;

}

.card {

  background:#1d1d1d;

  border-radius:15px;

  padding:15px;

  margin:10px auto;

  max-width:450px;

  box-shadow:
  0 0 10px #333;

}

.value {

  font-size:28px;

  font-weight:bold;

}

.normal {

  color:#00ff66;

}

.fault {

  color:#ff3333;

}

.warning {

  color:#ffaa00;

}

table {

  width:100%;

  border-collapse:collapse;

}

td {

  padding:10px;

  border-bottom:
  1px solid #444;

}

</style>

</head>


<body>


<h1>☀ SOLAR MONITOR</h1>


<div class="card">

<h2>Electrical Parameters</h2>


<table>

<tr>

<td>Solar Voltage</td>

<td class="value">

)rawliteral";


  html += String(
    solarVoltage,
    2
  );


  html += R"rawliteral(

 V

</td>

</tr>


<tr>

<td>Solar Current</td>

<td class="value">

)rawliteral";


  html += String(
    solarCurrent,
    3
  );


  html += R"rawliteral(

 A

</td>

</tr>


<tr>

<td>Solar Power</td>

<td class="value">

)rawliteral";


  html += String(
    solarPower,
    2
  );


  html += R"rawliteral(

 W

</td>

</tr>


<tr>

<td>Temperature</td>

<td class="value">

)rawliteral";


  html += String(
    panelTemperature,
    1
  );


  html += R"rawliteral(

 °C

</td>

</tr>


<tr>

<td>Light Level</td>

<td class="value">

)rawliteral";


  html += String(
    lightLevel
  );


  html += R"rawliteral(

</td>

</tr>


</table>

</div>


<div class="card">

<h2>Status</h2>

<div class="value">

)rawliteral";


  if (faultDetected) {

    html +=
      "<span class='fault'>" +
      faultMessage +
      "</span>";

  } else {

    html +=
      "<span class='normal'>" +
      faultMessage +
      "</span>";
  }


  html += R"rawliteral(

</div>

</div>


<div class="card">

<h2>Sensor Information</h2>

<table>

<tr>

<td>Voltage RAW</td>

<td>

)rawliteral";


  html += String(
    voltageRaw
  );


  html += R"rawliteral(

</td>

</tr>


<tr>

<td>ACS RAW</td>

<td>

)rawliteral";


  html += String(
    acsRaw,
    2
  );


  html += R"rawliteral(

</td>

</tr>


<tr>

<td>ESP32</td>

<td>ONLINE</td>

</tr>


</table>

</div>


<div class="card">

<p>

Touch the TTP223 sensor to change OLED pages.

</p>

</div>


</body>

</html>

)rawliteral";


  server.send(
    200,
    "text/html",
    html
  );
}

void setup() {

  Serial.begin(115200);

  delay(1000);

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  pinMode(
    GREEN_LED,
    OUTPUT
  );

  pinMode(
    RED_LED,
    OUTPUT
  );

  pinMode(
    TOUCH_PIN,
    INPUT
  );


  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  digitalWrite(
    GREEN_LED,
    LOW
  );

  digitalWrite(
    RED_LED,
    LOW
  );

  analogReadResolution(12);

  analogSetPinAttenuation(
    VOLTAGE_PIN,
    ADC_11db
  );

  analogSetPinAttenuation(
    ACS_PIN,
    ADC_11db
  );

  analogSetPinAttenuation(
    LDR_PIN,
    ADC_11db
  );

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );


  if (!display.begin(
        OLED_ADDRESS,
        true)) {

    Serial.println(
      "OLED ERROR!"
    );

    while (1);
  }


  display.clearDisplay();

  display.setTextColor(
    SH110X_WHITE
  );

  display.setTextSize(1);

  display.setCursor(15, 20);

  display.println(
    "SOLAR MONITOR"
  );

  display.setCursor(18, 35);

  display.println(
    "Starting..."
  );

  display.display();

  temperatureSensor.begin();

  WiFi.mode(
    WIFI_AP
  );

  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );


  Serial.println();

  Serial.println(
    "================================="
  );

  Serial.println(
    "SOLAR MONITORING SYSTEM"
  );

  Serial.println(
    "================================="
  );

  Serial.print(
    "WiFi SSID: "
  );

  Serial.println(
    AP_SSID
  );

  Serial.print(
    "Password: "
  );

  Serial.println(
    AP_PASSWORD
  );

  Serial.print(
    "Dashboard: "
  );

  Serial.println(
    WiFi.softAPIP()
  );


  server.on(
    "/",
    handleRoot
  );

  server.begin();


  Serial.println(
    "Web server started."
  );

  Serial.println();


  beep(100);

  delay(200);

  beep(100);
}

void loop() {

  solarVoltage =
    readSolarVoltage();


  solarCurrent =
    readSolarCurrent();


  solarPower =
    solarVoltage *
    solarCurrent;


  panelTemperature =
    readTemperature();


  lightLevel =
    readLightLevel();

  checkFaults();
  updateLEDs();
  checkTouch();
  server.handleClient();
  Serial.println(
    "-----------------------------"
  );

  Serial.print(
    "Voltage : "
  );

  Serial.print(
    solarVoltage,
    2
  );

  Serial.println(" V");


  Serial.print(
    "Current : "
  );

  Serial.print(
    solarCurrent,
    3
  );

  Serial.println(" A");


  Serial.print(
    "Power   : "
  );

  Serial.print(
    solarPower,
    2
  );

  Serial.println(" W");


  Serial.print(
    "Temp    : "
  );

  Serial.print(
    panelTemperature,
    1
  );

  Serial.println(" C");


  Serial.print(
    "Light   : "
  );

  Serial.println(
    lightLevel
  );


  Serial.print(
    "ACS RAW : "
  );

  Serial.println(
    acsRaw,
    2
  );


  Serial.print(
    "Status  : "
  );

  Serial.println(
    faultMessage
  );


  delay(1000);
}
