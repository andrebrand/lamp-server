#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>


#ifndef STASSID
#define STASSID "TP-LINK_OG"
#define STAPSK  "097226437"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

int PinRED = 0;
int PinGREEN = 2;
int PinBLUE = 4;

int rValue = LOW;
int gValue = LOW;
int bValue = LOW;
boolean doFade = true;

void handleRoot() {
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Lamp</title> <style>body{--body-padding: 20px; --ButtonFace: rgb(192, 192, 192); --ButtonHilight: rgb(255, 255, 255); --ButtonLight: rgb(223, 223, 223); --ButtonShadow: rgb(128, 128, 128); --windowBackground: #c0c0c0; --ButtonDkShadow: rgb(0, 0, 0); --windowHeadline: 'Change lamp color';}body{background-color: #5872c1; height:calc(100vh - (2 * var(--body-padding))); width: calc(100vw - (2 * var(--body-padding))); margin: var(--body-padding); padding:0; display: grid; grid-template-columns: calc(100%); font-family:'MS Sans Serif'; font-size:12px;}.body-text{padding-top:20px; padding-bottom: 20px;}.color-picker{display: grid; column-gap: 5px; grid-template-columns: 80px 1fr;}.color-table{display: grid; grid-template-columns: 50% 50%; border-bottom: 1px solid #838383; border-right: 1px solid #838383; padding: 1px;}.selected-color-wrapper{display: flex; padding: 20px; border: 1px solid var(--ButtonHilight); border-top-color: var(--ButtonShadow); border-left-color: var(--ButtonShadow);}.hide{user-select: none; opacity: 0.3;}.hide button{display: none !important;}.inset-color, .selected-color{border-top: 1px solid var(--ButtonShadow); border-left: 1px solid var(--ButtonShadow); border-right: 1px solid var(--ButtonFace); border-bottom: 1px solid var(--ButtonFace); height: 40px; max-width: 100%; position: relative;}.selected-color{height: 100px; width: 100px; margin: auto;}.inset-color::before{content: ''; position: absolute; left: 0; top: 0; right: 0; bottom: 0; border-left: 1px solid var(--ButtonDkShadow); border-top: 1px solid var(--ButtonDkShadow);}.inset-color::after{content: ''; position: absolute; left: -1px; top: -1px; right: -1px; bottom: -1px; border-right: 1px solid var(--ButtonHilight); border-bottom: 1px solid var(--ButtonHilight);}.inset-color.red, .selected-color.red{background-color: #ff0e00;}.inset-color.green, .selected-color.green{background-color: #00ff7b;}.inset-color.blue, .selected-color.blue{background-color: #3400fe;}.inset-color.pink, .selected-color.pink{background-color: #ff00fe;}.inset-color.black, .selected-color.black{background-color: #000;}.inset-color.yellow, .selected-color.yellow{background-color: #faff08;}.inset-color.cyan, .selected-color.cyan{background-color: #00feff;}.inset-color.white, .selected-color.white{background-color: #fff;}.window{width: 100%; max-width: 500px; display: flex; flex-direction: column; margin: auto; height: 400px; max-height: 700px; background-color: var(--windowBackground); padding:1px; border-top: 2px solid #fff; border-left: 2px solid #fff; border-bottom: 2px solid #000; border-right: 2px solid #000; position: relative;}.window::before{font-family: Tahoma; font-weight: bold; color: #fff; content: var(--windowHeadline); display: flex; align-items: center; padding-left: 2px; background: rgb(60,113,140); background: linear-gradient(90deg, rgba(60,113,140,1) 6%, rgba(84,143,173,1) 55%, rgba(132,200,221,1) 100%); height: 20px; width: calc(100% - 2px); margin: 0 auto; background-color: black;}.window::after{content: 'X'; position: absolute; font-family: Tahoma; font-weight: bold; text-align: center; font-size: 9px; right: 3px; top: 3px; width: 14px; height: 14px; border-top: 1px solid #fff; border-left: 1px solid #fff; border-bottom: 2px solid #808080; border-right: 2px solid #808080; background: #C0C0C0; display: flex; justify-content: center; align-items: center;}.window .body{padding: 20px;}.button-wrapper{display: flex; align-items: center; justify-content: flex-end; margin-top:20px;}button{border-top: 2px solid #fff; border-left: 2px solid #fff; border-right: 2px solid #000; border-bottom: 2px solid #000; background-color: #C0C0C0; padding-left: 20px; padding-right: 20px; padding-top:7px; padding-bottom: 7px; line-height: 11px; display: flex; align-items: center; position: relative; margin-left: 10px;}#cancelColor, #applyColor{display: none;}#cancelColor.show, #applyColor.show{display: flex;}button:hover{text-decoration: underline;}button:focus::before, button:active::before{content: ''; display: block; position: absolute; height: calc(100% - 8px); width: calc(100% - 8px); border: 1px dotted #000; border-radius: 2px; top: 0; left: 0; bottom: 0; right: 0; margin: auto;}button:active, button.active{border-bottom: 2px solid #000; border-right: 2px solid #000; border-left: 2px solid #000; border-top: 2px solid #000;}</style></head><body> <div class='window'> <div class='body'> <div class='button-wrapper'> <button type='button' id='modeFade' class='active'> fading colors </button> <button type='button' id='modeStatic'> static color </button> </div><div id='static-colors' class='hide'> <div class='body-text'> Select static light color: </div><div class='color-picker'> <div class='color-table'> <div class='inset-color red' data-name='red' data-r='1' data-g='0' data-b='0'></div><div class='inset-color pink' data-name='pink' data-r='1' data-g='0' data-b='1'></div><div class='inset-color yellow' data-name='yellow' data-r='1' data-g='1' data-b='0'></div><div class='inset-color green' data-name='green' data-r='0' data-g='1' data-b='0'></div><div class='inset-color cyan' data-name='cyan' data-r='0' data-g='1' data-b='1'></div><div class='inset-color blue' data-name='blue' data-r='0' data-g='0' data-b='1'></div><div class='inset-color white' data-name='white' data-r='1' data-g='1' data-b='1'></div><div class='inset-color black' data-name='black' data-r='0' data-g='0' data-b='0'></div></div><div class='selected-color-wrapper'> <div class='selected-color white' data-color='white'></div></div></div><div class='button-wrapper'> <button type='button' id='applyColor'> apply color </button> <button type='button' id='cancelColor'> cancel selection </button> </div></div></div></div><script src='https://code.jquery.com/jquery-3.5.1.min.js' integrity='sha256-9/aliU8dGd2tb6OSsuzixeV4y/faTqgFtohetphbbj0=' crossorigin='anonymous'></script> <script>$(document).ready( ()=>{let applyedColor='white'; let selectedColor='white'; let mode='fade'; for(var i=0; i < 255; i++){console.log(i);}$('.inset-color').click((e)=>{if(mode==='static'){t=$(e.target); const newColor=t.data('name'); if(selectedColor !==newColor){selectedColor=newColor; const currentColor=$('.selected-color').data('color'); $('.selected-color').data('color', newColor); $('.selected-color').addClass(newColor); $('.selected-color').removeClass(currentColor); $('#cancelColor').addClass('show'); $('#applyColor').addClass('show');}}}); $('#cancelColor').click(()=>{if(mode==='static'){selectedColor=applyedColor; const newColor=selectedColor; const currentColor=$('.selected-color').data('color'); $('.selected-color').data('color', newColor); $('.selected-color').addClass(newColor); $('.selected-color').removeClass(currentColor); $('#cancelColor').removeClass('show'); $('#applyColor').removeClass('show');}}); $('#applyColor').click(()=>{if(mode==='static'){applyedColor=selectedColor; $('#cancelColor').removeClass('show'); $('#applyColor').removeClass('show'); const color$=$('.inset-color.'+ applyedColor); const r=color$.data('r'); const g=color$.data('g'); const b=color$.data('b'); console.log('r', r, 'g', g, 'b', b); $.get('/change?r='+ r + '&g=' + g + '&b=' + b, function(data, status){console.log('data', data); console.log('status', status)});}}); $('#modeFade').click(()=>{if(!$('#modeFade').hasClass('active')){$('#static-colors').addClass('hide'); mode='fade'; $.get('/fade?fade=1', function(data, status){console.log('data', data); console.log('status', status)}); $('#modeStatic').removeClass('active'); $('#modeFade').addClass('active');}}); $('#modeStatic').click(()=>{if(!$('#modeStatic').hasClass('active')){$('#static-colors').removeClass('hide'); mode='static'; $.get('/fade?fade=0', function(data, status){console.log('data', data); console.log('status', status);}); $('#modeFade').removeClass('active'); $('#modeStatic').addClass('active'); $('#applyColor').addClass('show');}})}) </script></body></html>");
  return;
}


void handleChangeRequest() { //Handler

  String templateData = "";
  String rArg = server.arg(0);
  String gArg = server.arg(1);
  String bArg = server.arg(2);
  if(rArg == "1" || rArg ==  "0"){
    rValue = rArg.toInt();
  }
  if(gArg == "1" || gArg ==  "0"){
    gValue = gArg.toInt();
  }
  if(bArg == "1" || bArg ==  "0"){
    bValue = bArg.toInt();
  }
  updateLight();
  doFade = false;
  String result = "{doFade:" + String(doFade) + "}";
  server.send(200, "text/json", result);       //Response to the HTTP request
  return;
}

void handleFadeRequest() { //Handler
  
  String fArg = server.arg(0);
  if(fArg == "1" || fArg ==  "0"){
    doFade = fArg == "1" ? true : false;
  }
  String result = "{doFade:" + String(doFade) + "}";
  server.send(200, "text/json", result);       //Response to the HTTP request
  return;
}

void applyRGB(int red, int green, int blue)
 {
  analogWrite(PinRED, red);
  analogWrite(PinGREEN, green);
  analogWrite(PinBLUE, blue);
  return;
}

void updateLight(){
  digitalWrite(PinRED, rValue);
  digitalWrite(PinGREEN, gValue);
  digitalWrite(PinBLUE, bValue);
  return;
}

void loadColor(){
  rValue = LOW;
  gValue = LOW;
  bValue = HIGH;
  updateLight();
  return;
}

void warnColor(){
  while(true){
    rValue = HIGH;
    gValue = LOW;
    bValue = LOW;
    updateLight();
    delay(100);
    rValue = LOW;
    gValue = LOW;
    bValue = LOW;
    updateLight();
    delay(100);
  }
  
  return;
}


void defaultColor(){
  rValue = HIGH;
  gValue = HIGH;
  bValue = HIGH;
  updateLight();
  return;
}




int rgbColor[] = {1023, 0, 0};
int decColor = 0;
int incColor = 1;

void fadeColors(){
  // cross-fade the two colours.
  static unsigned long int waitSince = 0;
  static unsigned long int waitSpan = 10;
  if (millis() - waitSince >= waitSpan){
    waitSince = millis();
    rgbColor[decColor] -= 1;
    rgbColor[incColor] += 1;
    if(rgbColor[incColor] > 1010 || rgbColor[decColor] > 1010){
      waitSpan = 100;
    }else{
      waitSpan = 5;
    }
    if(rgbColor[incColor] == 1023){
      decColor = decColor == 2 ? 0 : decColor + 1;
      incColor = incColor == 2 ? 0 : incColor + 1;
    }
    applyRGB(rgbColor[0], rgbColor[1], rgbColor[2]);
  }
  return;
}


void setup(void) {
  pinMode(PinRED, OUTPUT);
  pinMode(PinGREEN, OUTPUT);
  pinMode(PinBLUE, OUTPUT);
  // WiFi.persistent(false);
  //WiFi.setSleepMode(WIFI_NONE_SLEEP, 10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  loadColor();
  Serial.begin(9600);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  if (MDNS.begin("lamp")) {
    defaultColor();
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/change", handleChangeRequest);
  server.on("/fade", handleFadeRequest);
  
  MDNS.addService("http", "tcp", 80);
  server.begin();
}

void loop(void) {
  server.handleClient();
  if(doFade == true){
    fadeColors();
  }
  if(WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED){
    warnColor();
  }
  //update();
  Serial.println(MDNS.update());
}
