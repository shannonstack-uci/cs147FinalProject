#include <DFRobot_mmWave_Radar.h>

#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "string.h"

#define PHOTO_RESISTOR 33
#define LED_PIN 25

HardwareSerial mySerial(1);
DFRobot_mmWave_Radar sensor(&mySerial);

int minBrightness = 1023; // minimum brightness
int maxBrightness = 0;    // maximum value of brightness
int currBrightness;       // current brightness reading
int adjustedBrightness;   // current brightness after adjustment

char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "18.116.40.144";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/?var=10";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("Done\n");
      // Serial.printf("SSID = %s\n", ssid);
      // Serial.printf("PASSWD = %s\n", pass);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}


void setup() {
  Serial.begin(9600);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);  //RX,TX

  delay(1000);
  // Retrieve SSID/PASSWD from flash before anything else
  nvs_access();
  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());


  while (!Serial)
    delay(10);


  sensor.factoryReset();     //Restore to the factory settings
  sensor.DetRangeCfg(0, 1);  //The detection range is as far as 9m
  sensor.OutputLatency(0, 10);

  Serial.println("Finished radar setup");

  // calibrate photo sensor 
  pinMode(LED_PIN, OUTPUT);
  pinMode(PHOTO_RESISTOR, INPUT);
  for (int i = 0; i < 5; i++)
  {
    
    currBrightness = analogRead(PHOTO_RESISTOR);

    if (currBrightness < minBrightness)
    {
      minBrightness = currBrightness;
    }
    if (currBrightness > maxBrightness)
    {
      maxBrightness = currBrightness;
    }

    delay(500);
    if (currBrightness < minBrightness)
    {
      minBrightness = currBrightness;
    }
    if (currBrightness > maxBrightness)
    {
      maxBrightness = currBrightness;
    }
    delay(500);
  }
}

void loop() {
  int val = sensor.readPresenceDetection();
  Serial.println(val);
  std::string path = "/?cmd=";
  

  currBrightness = analogRead(PHOTO_RESISTOR);
  adjustedBrightness = map(currBrightness, minBrightness, maxBrightness, 0, 100);
  if (adjustedBrightness < 20 && val) {
    // lights not on, turn it on 
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Yes motion no light");
    // turn light on
    path.append("on");
  } else if( adjustedBrightness >= 20 && val==0) {
    Serial.println("No motion yes light");
    // turn light off
    path.append("off");
  } else if(adjustedBrightness < 20 && val==0){
    Serial.println("No motion no light");
    // do nothing 
  } else if (adjustedBrightness >= 20 && val==1){
    Serial.println("Yes motion yes light");
    // do nothing
  }

  int err;
  WiFiClient c;
  HttpClient http(c);
  err = http.get(kHostname, 5000, path.c_str());
}