#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Camera.h"
#include "WebPost.h"

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//



const char* ssid = "FlyFly";
const char* password = "flyuntildie";

void startCameraServer();


void NetMaintain() {
  int connect_fail=0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connecting");
    WiFi.disconnect();
    WiFi.begin();
    int wait_cnt=60;
    while (wait_cnt-- > 0) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\r\nWifi connected");
        connect_fail=0;
        return;
      }
      delay(1000);
      Serial.print(".");
    }  
    connect_fail++;
    if (connect_fail >= 5) ESP.restart();
  } 
}


void TimeSync() {
  struct tm tmstruct ;
  byte daysavetime=0;
  long timezone=7;
  Serial.println("Contacting Time Server");
  //configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  configTime(3600 * timezone, daysavetime * 3600, "clock.nectec.or.th", "0.pool.ntp.org", "1.pool.ntp.org");              //Set configTime NTP
  vTaskDelay(2000);
  tmstruct.tm_year = 0;
  Serial.print("OK");
  while (!getLocalTime(&tmstruct, 1000))
  {
    Serial.print(".");
  }

  Serial.printf("\r\nNow is : %d-%02d-%02d %02d:%02d:%02d\r\n", (tmstruct.tm_year) + 1900, ( tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);

}

#define LED 33
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(LED,OUTPUT_OPEN_DRAIN);

  CameraSetup();

  WiFi.begin(ssid, password);
  delay(1000);

  NetMaintain();

  TimeSync();

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

}

#define CAMERA_ID "__agricam00__1000__100"


String GetCurrentTimeString() {
  struct tm t;  
  char str[20];
  while (!getLocalTime(&t, 1000))
  {
    Serial.print(".");
  }
  sprintf(str,"%04d%02d%02d%02d%02d%02d",(t.tm_year) + 1900, (t.tm_mon) + 1, t.tm_mday, t.tm_hour , t.tm_min, t.tm_sec);
  Serial.printf("\r\n\nNow is : %s\r\n", str );

  return String(str);
}




int pic_cnt=0;
void loop() {

  int capture_retry=0;

  NetMaintain();

  {
    struct tm t;
    static int cnt=0,led=0;
    cnt++;
    if (cnt==10) {
      cnt=0;
      led=led^1;
      digitalWrite(LED,led);
    }
    vTaskDelay(100);
    getLocalTime(&t, 5000);
    if (pic_cnt>0) {
      //if (((t.tm_min % 5 != 0) || (t.tm_sec != 0))) return;
      //if (((t.tm_min % 5 != 4) || (t.tm_sec != 55))) return;
      //if (((t.tm_min % 15 != 14) || (t.tm_sec != 55))) return;
      if (((t.tm_min  != 0) || (t.tm_sec != 0))) return;
    }
  }


  if (capture_retry>=3) {
    capture_retry = 0;
    //put code to init camera here
  }

  camera_fb_t *fb;
  String my_id_str = "__agricam00__1000__100";

  
  String time_str= GetCurrentTimeString();
  String img_name = time_str + my_id_str + ".jpg";

  CameraFlash(1);
  fb=CameraCapture();
  if (fb==NULL)  {
    capture_retry++;
    return;
  }
  Serial.println("pic_number: " + String(pic_cnt++));
  String res=WebPostSendImage(img_name,time_str,fb->buf,fb->len);
  Serial.println("Server Response:");
  Serial.print(res);
  CameraRelease(fb);
  CameraFlash(0);
}

