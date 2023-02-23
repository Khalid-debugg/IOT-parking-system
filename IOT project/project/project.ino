#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>                               
#include <WebSocketsServer.h>                        
#include <ArduinoJson.h> 
#include <fstream>

#define enterServo_PIN 25
#define exitServo_PIN 26
#define enterIR_PIN 33
#define exitIR_PIN 14
#define parking1_PIN 15
#define parking2_PIN 2
#define parking3_PIN 4
#define LED1_PIN 5
#define LED2_PIN 18 
#define LED3_PIN 19

using namespace std;

int lastStateEnterIR = HIGH;  
int currentStateEnterIR;
int lastStateExitIR = HIGH;  
int currentStateExitIR;
int lastStateP1IR = HIGH;  
int currentStateP1IR;
int lastStateP2IR = HIGH;  
int currentStateP2IR;
int lastStateP3IR = HIGH;  
int currentStateP3IR;
int currentStateLED1;
int currentStateLED2;
int currentStateLED3;

Servo enterServoMotor;
Servo exitServoMotor;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); 

const char* ssid = "Goda";
const char* password = "jgwi7000";
ifstream myfile;
myfile.open("index.html");
String webpage = "<!DOCTYPE html><html><head> <title>Parking</title> <link rel='icon' type='image/png' href='https://cdn-icons-png.flaticon.com/512/3774/3774278.png'> <style> *{ margin: 0; padding: 0; box-sizing: border-box; } body{ background-image: url('https://blog.getmyparking.com/wp-content/uploads/2018/07/underground-parking.jpg'); background-size:auto; background-position-y: center; background-position-x: center; background-repeat: no-repeat; display: flex; flex-direction: column; justify-content: center; height: 100vh; } #content{ background-color: rgba(255, 255, 255, 0.75); border-radius: 1.75em; display: flex; flex-direction: column; align-items: center; justify-content: space-around; padding: 2em; margin: 10px auto; } #parking{ display: flex; } #parking *{ margin: 0.25em; } p{ text-align: center; font-size: 1.25em; padding-bottom: 0.5em; font-family:'Franklin Gothic Medium', 'Arial Narrow', Arial, sans-serif; } h1{ padding: 0.75em; } .parkingSpot{ display: flex; flex-direction: column; margin: auto 0.25em; } .disabled{ pointer-events: none; background: #011b2b; border-radius: 12px; font-family: Arial; color: #ffffff; font-size: 20px; padding: 10px 20px 10px 20px; text-decoration: none } .btn{ border-radius: 12px; font-family: Arial; color: #ffffff; font-size: 20px; background: #236794; padding: 10px 20px 10px 20px; text-decoration: none } .btn:hover { background: #011b2b; text-decoration: none; cursor: pointer; } .history{ background-color: rgba(255, 255, 255, 0.75); border-radius: 1em; padding: 1.5em; margin: 1em 5em; } h2{ margin: 0.25em; } ul *{ margin-bottom: 0.25em; font-family:'Franklin Gothic Medium', 'Arial Narrow', Arial, sans-serif; } .empty{ width: 200px; height: 300px; background-color: white; border: 0.5em solid black; border-radius: 1.5em; } .full{ background-image: url('https://media.istockphoto.com/id/1256522275/vector/red-sedan-semi-flat-rgb-color-vector-illustration.jpg?s=612x612&w=0&k=20&c=K-Ud7A-4CAo-ZlKWsXg-5qupVLDXfaD90s6tVpGFTM4='); background-size: cover; background-repeat: no-repeat; background-position-x: center; background-position-y: center; } </style></head><body><div id='content'> <h1>IOT Parking System</h1> <div id='parking'> <div class='parkingSpot'><p>P1: <span id='p1'>Empty</span></p> <div class='empty'> </div> <button type='button' class='btn' id='reserve1'>Reserve parking 1</button></div> <div class='parkingSpot'><p>P2: <span id='p2'>Empty</span></p> <div class='empty'> </div> <button type='button' class='btn' id='reserve2'>Reserve parking 2</button></div> <div class='parkingSpot'><p>P3: <span id='p3'>Empty</span></p> <div class='empty'> </div> <button type='button' class='btn' id='reserve3'>Reserve parking 3</button></div> </div> </div><div class='history'> <h2>Parking history</h2> <ul id='parkingList'> </ul></div></body><script> var Socket; document.getElementsByClassName('btn')[0].addEventListener('click', btn1); document.getElementsByClassName('btn')[1].addEventListener('click', btn2); document.getElementsByClassName('btn')[2].addEventListener('click', btn3); var timer1= 0; var timer2= 0; var timer3= 0; var refershTimer1; var refershTimer2; var refershTimer3; var parkingHistory = []; function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function (event) { processCommand(event); }; } function btn1() { var response = { P1state: 'Occupied', P2state: document.getElementById('p2').innerHTML, P3state: document.getElementById('p3').innerHTML }; Socket.send(JSON.stringify(response)); } function btn2() { var response = { P1state: document.getElementById('p1').innerHTML, P2state: 'Occupied', P3state: document.getElementById('p3').innerHTML }; Socket.send(JSON.stringify(response)); } function btn3() { var response = { P1state: document.getElementById('p1').innerHTML, P2state: document.getElementById('p2').innerHTML, P3state: 'Occupied' }; Socket.send(JSON.stringify(response)); } function startTimer(p){ if(p == 'p1') refershTimer1 = setInterval(()=>{timer1++;},1000); else if(p == 'p2') refershTimer2 = setInterval(()=>{timer2++;},1000); else refershTimer3 = setInterval(()=>{timer3++;},1000); } function finishTimer(x){ if(x=='p1'){ clearInterval(refershTimer1); var parkingEntry ={}; parkingEntry.ID = 'Parking 1'; parkingEntry.hours = Math.floor(timer1/3600); timer1 = timer1 % 3600; parkingEntry.minutes = Math.floor(timer1/60); timer1 = timer1 % 60; parkingEntry.seconds = timer1; timer1 =0; parkingHistory.unshift(parkingEntry); } else if(x=='p2'){ clearInterval(refershTimer2); var time = timer3; var parkingEntry ={}; parkingEntry.ID = 'Parking 2'; parkingEntry.hours = Math.floor(timer2/3600); timer2 = timer2 % 3600; parkingEntry.minutes = Math.floor(timer2/60); timer2 = timer2 % 60; parkingEntry.seconds = timer2; timer2 =0; parkingHistory.unshift(parkingEntry); } else if(x=='p3'){ clearInterval(refershTimer3); var time = timer3; var parkingEntry ={}; parkingEntry.ID = 'Parking 3'; parkingEntry.hours = Math.floor(timer3/3600); timer3 = timer3 % 3600; parkingEntry.minutes = Math.floor(timer3/60); timer3 = timer3 % 60; parkingEntry.seconds = timer3; timer3 =0; parkingHistory.unshift(parkingEntry); } viewHistory(); } function viewHistory(){ var list= document.getElementById('parkingList'); list.innerHTML=''; for(let i=0;i<5;i++){ if(parkingHistory[i]!==undefined){ var listItem = document.createElement('li'); listItem.innerHTML = `${parkingHistory[i].ID} => Hours: ${parkingHistory[i].hours}, Minutes: ${parkingHistory[i].minutes}, Seconds: ${parkingHistory[i].seconds}`; list.appendChild(listItem); } } } function processCommand(event) { var obj = JSON.parse(event.data); var b1 = document.getElementById('reserve1'); var b2 = document.getElementById('reserve2'); var b3 = document.getElementById('reserve3'); if(obj.P1state == 'Occupied' && document.getElementById('p1').innerHTML=='Empty'){ b1.classList.replace('btn','disabled'); b1.innerHTML = 'Reserved'; document.getElementsByClassName('empty')[0].classList.add('full'); startTimer('p1'); } else if(obj.P1state == 'Empty' && document.getElementById('p1').innerHTML == 'Occupied'){ b1.classList.replace('disabled','btn'); b1.innerHTML = 'Reserve parking 1'; document.getElementsByClassName('empty')[0].classList.remove('full'); finishTimer('p1'); } document.getElementById('p1').innerHTML = obj.P1state; if(obj.P2state == 'Occupied' && document.getElementById('p2').innerHTML == 'Empty'){ b2.classList.replace('btn','disabled'); b2.innerHTML = 'Reserved'; document.getElementsByClassName('empty')[1].classList.add('full'); startTimer('p2'); } else if(obj.P2state == 'Empty' && document.getElementById('p2').innerHTML == 'Occupied'){ b2.classList.replace('disabled','btn'); b2.innerHTML = 'Reserve parking 2'; document.getElementsByClassName('empty')[1].classList.remove('full'); finishTimer('p2'); } document.getElementById('p2').innerHTML = obj.P2state; if(obj.P3state == 'Occupied' && document.getElementById('p3').innerHTML == 'Empty'){ b3.classList.replace('btn','disabled'); b3.innerHTML = 'Reserved'; document.getElementsByClassName('empty')[2].classList.add('full'); startTimer('p3'); } else if(obj.P3state == 'Empty' && document.getElementById('p3').innerHTML == 'Occupied'){ b3.classList.replace('disabled','btn'); b3.innerHTML = 'Reserve parking 3'; document.getElementsByClassName('empty')[2].classList.remove('full'); finishTimer('p3'); } document.getElementById('p3').innerHTML = obj.P3state; } window.onload = function (event) { init(); }</script></html>";
int interval = 1000;                                 
unsigned long previousMillis = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(enterIR_PIN, INPUT);
  pinMode(exitIR_PIN, INPUT);
  pinMode(parking1_PIN, INPUT);
  pinMode(parking2_PIN, INPUT);
  pinMode(parking3_PIN, INPUT);
  enterServoMotor.attach(enterServo_PIN);
  exitServoMotor.attach(exitServo_PIN);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED3_PIN, HIGH);

  WiFi.begin(ssid, password);                         
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));     
 
  while (WiFi.status() != WL_CONNECTED) {            
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());                     
  
  server.on("/", []() {                               
    server.send(200, "text/html", webpage);           
  });
  server.begin();                                     
  webSocket.begin();                                  
  webSocket.onEvent(webSocketRecieveEvent); 
}
void loop() {

  currentStateEnterIR = digitalRead(enterIR_PIN);
  currentStateExitIR = digitalRead(exitIR_PIN);
  currentStateP1IR = digitalRead(parking1_PIN);
  currentStateP2IR = digitalRead(parking2_PIN);
  currentStateP3IR = digitalRead(parking3_PIN);
  currentStateLED1 = digitalRead(LED1_PIN);
  currentStateLED2 = digitalRead(LED2_PIN);
  currentStateLED3 = digitalRead(LED3_PIN);

  
//Enter gate
  //open
  bool isEmpty = currentStateLED1 == HIGH || currentStateLED2 == HIGH ||currentStateLED3 == HIGH;
  if ((lastStateEnterIR == HIGH && currentStateEnterIR == LOW) && isEmpty){
     for(int pos=0; pos<=90; pos++){
        enterServoMotor.write(pos);
        delay(15);
      }      
  }
  //close
  else if(lastStateEnterIR == LOW && currentStateEnterIR == HIGH && isEmpty){
      for(int pos=90; pos>=0; pos--){
        enterServoMotor.write(pos);
        delay(15);
      }
  }

  lastStateEnterIR = currentStateEnterIR;
//Exit gate
  //open
  if (lastStateExitIR == HIGH && currentStateExitIR == LOW){
     for(int pos=0; pos<=90; pos++){
        exitServoMotor.write(pos);
        delay(15);
      }      
  }
  //close
  else if(lastStateExitIR == LOW && currentStateExitIR == HIGH){
    for(int pos=90; pos>=0; pos--){
        exitServoMotor.write(pos);
        delay(15);
      }
  }
  lastStateExitIR = currentStateExitIR;
//Parking 1 area
  //parked
    if (lastStateP1IR == HIGH && currentStateP1IR == LOW){
      digitalWrite(LED1_PIN, LOW);
    }
   //empty
   else if(lastStateP1IR == LOW && currentStateP1IR == HIGH){
       digitalWrite(LED1_PIN, HIGH);
    }
  lastStateP1IR = currentStateP1IR;
//Parking 2 area
  //parked
    if (lastStateP2IR == HIGH && currentStateP2IR == LOW){
      digitalWrite(LED2_PIN, LOW);
    }
   //empty
   else if(lastStateP2IR == LOW && currentStateP2IR == HIGH){
       digitalWrite(LED2_PIN, HIGH);
    }
  lastStateP2IR = currentStateP2IR;
//Parking 3 area
  //parked
    if (lastStateP3IR == HIGH && currentStateP3IR == LOW){
      digitalWrite(LED3_PIN, LOW);
    }
   //empty
   else if(lastStateP3IR == LOW && currentStateP3IR == HIGH){
       digitalWrite(LED3_PIN, HIGH);
    }
  lastStateP3IR = currentStateP3IR;

  server.handleClient();                              
  webSocket.loop();                                   
  
  unsigned long now = millis();                       
  if ((unsigned long)(now - previousMillis) > interval) { 
    
    String Json = "";                           
    StaticJsonDocument<200> doc;                      
    JsonObject jsonObject = doc.to<JsonObject>();      

    if(currentStateLED1 == HIGH)
      jsonObject["P1state"] = "Empty";
    else
      jsonObject["P1state"] = "Occupied";

    if(currentStateLED2 == HIGH)
      jsonObject["P2state"] = "Empty";
    else
      jsonObject["P2state"] = "Occupied";

    if(currentStateLED3 == HIGH)
      jsonObject["P3state"] = "Empty";
    else
      jsonObject["P3state"] = "Occupied"; 
      
    serializeJson(doc, Json);                   
    Serial.println(Json);                       
    webSocket.broadcastTXT(Json);               
    
    previousMillis = now;                             
  }
}
void webSocketRecieveEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {     

      if(type == WStype_TEXT){

        StaticJsonDocument<200> clientJson;                    
        DeserializationError error = deserializeJson(clientJson, payload);
        String p1state = clientJson["P1state"];
        String p2state = clientJson["P2state"];
        String p3state = clientJson["P3state"];

        if(p1state == "Empty")
            digitalWrite(LED1_PIN, HIGH);
        else if(p1state == "Occupied")
            digitalWrite(LED1_PIN, LOW);

        if(p2state == "Empty")
            digitalWrite(LED2_PIN, HIGH);
        else if(p2state == "Occupied")
            digitalWrite(LED2_PIN, LOW);

        if(p3state == "Empty")
            digitalWrite(LED3_PIN, HIGH);
        else if(p3state == "Occupied")
            digitalWrite(LED3_PIN, LOW);
      }
      else{
        Serial.println("data recieved is not text");
      }

      
}



