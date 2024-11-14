#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Servo.h>

const char* ssid = "Redmi"; //"Galaxy A03 Core8529";
const char* password = "buntutMuIren9"; //"MilikAsidar";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Set your Static IP address
//IPAddress local_IP(192, 168, 48, 220);
// Set your Gateway IP address
//IPAddress gateway(192, 168, 48, 160);

//IPAddress subnet(255, 255, 255, 0);
//IPAddress primaryDNS(8, 8, 8, 8);   //optional
//IPAddress secondaryDNS(8, 8, 4, 4); //optional

// Assign output variables to GPIO pins
const int rainSensorPower=D5;
const int rainSensor=A0;
const int engineRelay=D3;
const int redLed=D0;
const int greenLed=D1;
const int blueLed=D2;
const int trig = D8;
const int echo = D7;
String duration="5";
String engineStatusResponse;
String servoDegree;
String rainSensStat;
String wetTreshold = "500";
String dryTreshold="1000";
bool rainSensorStatus=true;
bool engineStatus = false;
bool toggleEngine = false;
unsigned long HCduration, distance, distancePrev;
int rainDropValue;
int rainDropValuePrev=1024;
Servo arm1;


void setup() {
  Serial.begin(115200);
  
  pinMode(rainSensor, INPUT_PULLUP);
  pinMode(rainSensorPower, OUTPUT);
  pinMode(engineRelay,OUTPUT);
  pinMode(redLed,OUTPUT);
  pinMode(greenLed,OUTPUT);
  pinMode(blueLed,OUTPUT);
  pinMode(trig,OUTPUT);
  pinMode(echo, INPUT);
  
  arm1.writeMicroseconds(600); //544 microseconds = 0 degree
  arm1.attach(D4, 544, 2400);

  digitalWrite(engineRelay, HIGH);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  /*if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }*/
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Connecting to WiFi..");
    delay(5000);
    checkRainSensor();
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to load water-tap.svg file
  server.on("/water-tap.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/water-tap.svg", "image/svg");
  });

  server.on("/engine", HTTP_GET, [](AsyncWebServerRequest *request){
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    
    if (request->hasParam("engineStatus") && request->hasParam("duration")) {
      engineStatusResponse = request->getParam("engineStatus")->value();
      duration = request->getParam("duration")->value();
      rainSensStat = request->getParam("rainSensStat")->value();
      rainSensorStatus= (rainSensStat=="on")?true:false;
    }else{
      engineStatusResponse = "on";
      duration = "3";
    }

    toggleEngine = true;
    engineStatus = (engineStatusResponse=="on")?true:false;
    
    Serial.println(engineStatusResponse);
    request->send(200, "text/text", engineStatusResponse+"-"+duration+"-"+servoDegree+"-"+rainDropValue+"-"+rainSensStat);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){

    if(digitalRead(engineRelay)){
      engineStatusResponse="on";
      servoDegree="0";
    }else{
      engineStatusResponse="off";
      servoDegree="90";
    }

    if (request->hasParam("duration") && request->hasParam("rainSensStat")) {
      duration = request->getParam("duration")->value();
      rainSensStat = request->getParam("rainSensStat")->value();
      wetTreshold = request->getParam("wetTreshold")->value();
      dryTreshold = request->getParam("dryTreshold")->value();
      rainSensorStatus= (rainSensStat=="on")?true:false;
    }

    digitalWrite(trig, LOW); 
    delayMicroseconds(2);
    digitalWrite(trig,  HIGH); 
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
       
    HCduration  = pulseIn(echo, HIGH);
    distance = (HCduration/2)/29.1;
      
    

    Serial.println(engineStatusResponse+"-"+distance+"-"+servoDegree+"-"+rainDropValue+"-"+rainSensStat+"-"+dryTreshold);
    
    request->send(200, "text/text", engineStatusResponse+"-"+distance+"-"+servoDegree+"-"+rainDropValue+"-"+rainSensStat);
  });

  server.begin();

}

void loop() {
  if(toggleEngine){
    Serial.println("toggleEngine true");
      if(!engineStatus){
        Serial.println("engineStatus false");
        turnOnOffEngine(0);
        
        int dd = duration.toInt() * 60 * 1000;
        delay(dd);
  
        changeServoState(600);

        Serial.println("Mesin OFF");
        
      }else{
        Serial.println("engineStatus true");
        changeServoState(1500);
  
        int dd = duration.toInt() * 1000;
        delay(dd);
        
        turnOnOffEngine(1);

        Serial.println("Mesin ON");
      }
  
      toggleEngine=false;
    }
    
  if(WiFi.status() == WL_CONNECTED){
    
    checkRainSensor();
    
    if(digitalRead(engineRelay)){
      setRGBcolor(255, 0, 0); // Red Color
      delay(1000);
            
      setRGBcolor(0, 255, 0); // Green Color
      delay(1000);
            
      setRGBcolor(0, 0, 255); // Blue Color
      delay(1000);
      
      setRGBcolor(255, 255, 0); // Yellow Color
      delay(1000);
          
      setRGBcolor(0, 255, 255); // Cyan Color
      delay(1000);
            
      setRGBcolor(255, 0, 255); // Magenta Color
      delay(1000);
            
      setRGBcolor(255, 165, 0); // Orange Color
      delay(1000);
            
      setRGBcolor(128, 0, 128); // Purple Color
      delay(1000);
            
      setRGBcolor(255, 255, 255); // White Color
      delay(1000);
    }else{
      setRGBcolor(255, 0, 0); // Red Color
      delay(5000);
    }
  }else{
    delay(5000);
    checkRainSensor();
    if(digitalRead(engineRelay)){
      setRGBcolor(128, 0, 128); // Purple Color
    }else{
      setRGBcolor(255, 0, 0); // Red Color
    }
  }
  

}

void setRGBcolor(int red, int green, int blue){
  analogWrite(redLed, 255 - red);
  analogWrite(greenLed, 255 - green);
  analogWrite(blueLed, 255 - blue);
}

void turnOnOffEngine(int com){
  if(com == 1){
    //if(digitalRead(engineRelay)){
      digitalWrite(engineRelay, HIGH);
      engineStatusResponse="off";
    //}
  }else{
    //if(!digitalRead(engineRelay)){
      digitalWrite(engineRelay, LOW);
      engineStatusResponse="on";
    //}
  }
}

void changeServoState(int degree){
  arm1.writeMicroseconds(degree);
  servoDegree=(degree==600)?"0":"90";
}

void checkRainSensor(){
  if(!digitalRead(rainSensorPower)){
    digitalWrite(rainSensorPower, HIGH);
  }
    delay(1000);
    rainDropValue=analogRead(rainSensor);
    
    if(rainSensorStatus){
      if(rainDropValue < wetTreshold.toInt()){
        toggleEngine=true;
        engineStatus=false;
      }else if(rainDropValue > rainDropValuePrev && rainDropValue > dryTreshold.toInt()){
        if(!engineStatus){
          toggleEngine=true;
          engineStatus=true;
        }
      }
    }

    Serial.println(engineStatusResponse+"-"+distance+"-"+servoDegree+"-"+rainDropValue+"-"+rainDropValuePrev+"-"+rainSensStat);
    
    rainDropValuePrev=rainDropValue;
    digitalWrite(rainSensorPower, LOW);
    
}
