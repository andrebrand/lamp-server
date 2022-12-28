#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>


#define STASSID "ardu"
#define STAPSK  "123456789"
const char* ap_ssid = STASSID;
const char* ap_password = STAPSK;

ESP8266WebServer server(80);

ESP8266WiFiMulti wifiMulti;

const int led = 13;


char ssid[32] = "";
char password[70] = "";

int PinRED = 0;
int PinGREEN = 2;
int PinBLUE = 4;

int rValue = LOW;
int gValue = LOW;
int bValue = LOW;
boolean doFade = true;

// 0 = access point
// 1 = client
int wifiMode = 0;

//delay in loop
unsigned long previousMillis = 0;
const long interval = 10000;

const char* dns_name = "lamp";

//Inital start up
//-----------------------------------
void setup(void) {
  Serial.begin(9600);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect();
  pinMode(PinRED, OUTPUT);
  pinMode(PinGREEN, OUTPUT);
  pinMode(PinBLUE, OUTPUT);
  WiFi.hostname(dns_name);
  int n = WiFi.scanNetworks();
  delay(500);
  startAccessPoint();
  loadColor();
  startWebServer();
}


boolean skip = false;

//Main function
//-----------------------------
void loop(void) {
  server.handleClient();
  if (doFade == true) {
    fadeColors();
  }
  skip = doSkip();
  if (skip == true) {
    Serial.println("[AB] still running...");
    checkForClients();
  }
  
}

//----------------------------------------
//----------------------------------------
//----------------------------------------

boolean doSkip() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  } else {
    return false;
  }
}

String scanWiFi(){
  WiFi.scanNetworks();
    Serial.println("[AB] Scan start");

    String wifiJson = "{\"wifi\": [";

    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    Serial.println("[AB] Scan done");
    if (n == 0) {
        Serial.println("[AB] no networks found");
    } else {
        Serial.print("[AB] networks found:");
        Serial.println(n);
        for (int i = 0; i < n; ++i) {
            if(i != 0){
              wifiJson = wifiJson + ", ";
            }
            wifiJson = wifiJson + "{\"name\": \"" + WiFi.SSID(i).c_str() + "\"}";
        }
    }

    wifiJson = wifiJson + "]}";
    // Delete the scan result to free memory.
    //WiFi.scanDelete();
    return wifiJson;
}


//Start up functions
//-----------------------------
void startAccessPoint() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  //Show ip for debugging
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("[AB] AP IP address: ");
  Serial.println(myIP);
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/settingsPost", handleSettingsPost);
  server.on("/change", handleChangeRequest);
  server.on("/script.js", handleScript);
  server.on("/style.css", handleStyle);
  server.on("/bootstrap.css", handleBootstrap);
  server.on("/favicon.svg", handleIcon);
  server.on("/error.svg", handleErrorIcon);
  server.on("/info.svg", handleInfoIcon);
  server.on("/fade", handleFadeRequest);
  server.on("/getMode", handleGetMode);
  server.on("/getWifiMode", handleGetWifiMode);
  server.on("/scanWifi", handlescanWifi);
  server.begin();
}



//-----------------------------
void checkForClients() {

  if (wifiMode == 1 && WiFi.softAPgetStationNum() == 0) {
    WiFi.softAPdisconnect();
    WiFi.persistent(false);
    WiFi.setSleepMode(WIFI_NONE_SLEEP, 10);
    WiFi.mode(WIFI_STA);
    WiFi.scanDelete();
  }
}



//Web pages
//-----------------------------


void handleSettingsPost() {
 
  boolean success = false; 

  String ssid_param = getParameter(server.arg("plain"), 1);
  //char ssid_char[32] =  ssid_param.c_str();
  String wifipw_param = getParameter(server.arg("plain"), 2);
  //const char* wifipw_char = wifipw_param.c_str();
  wifiMulti.cleanAPlist();
  wifiMulti.addAP(ssid_param.c_str(), wifipw_param.c_str());
  delay(2000);
  if (wifiMulti.run() == WL_CONNECTED) {
    ssid_param.toCharArray(ssid, ssid_param.length()+1);
    wifipw_param.toCharArray(password, wifipw_param.length()+1);
    Serial.println("[AB] connected"); 

    Serial.print("[AB] IP"); 
    Serial.println(WiFi.localIP().toString()); 
    
    
    success = true;
    wifiMode = 1;
    server.send(200, "application/json", "{\"success\": true, \"wifi\":\"" + ssid_param + "\", \"hostname\": \"" + dns_name + "\", \"ip\": \"" + WiFi.localIP().toString() +  "\"}");
  }else{
    Serial.println("[AB] disconnected");
    success = false;
    server.send(404, "application/json", "{\"success\": false, \"message\":\"Cloud not connect to specified Wifi.\"}");
  }
}

void handleRoot() {
  server.send(200, "text/html", F("<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Lamp</title> <link rel='stylesheet' href='bootstrap.css'> <link rel='stylesheet' href='style.css'> <link rel='icon' href='favicon.svg'></head><body> <div class='container mt-5'> <div class='card mb-4'> <div class='card-header'> <h4 class='my-0 font-weight-normal'>Change lamp colour</h4> </div><div class='card-body'> <ul class='nav nav-tabs' id='myTab'> <li class='nav-item'> <a class='nav-link active' id='home-tab' href='/'>Colour picker</a> </li><li class='nav-item' id='settingsTabNavItem'> <a class='nav-link' id='settings-tab' href='/settings'>Network</a> </li></ul> <div class='tab-content'> <div class='tab-pane fade active show' id='home'> <div class='container p-0 mb-3'> <div class='form-check'> <input class='form-check-input' type='radio' name='gridRadios' id='gridRadios1' value='fade' checked='checked'> <label class='form-check-label' for='gridRadios1'> Fading Colours </label> </div><div class='form-check mt-2'> <input class='form-check-input' type='radio' name='gridRadios' id='gridRadios2' value='static' checked=''> <label class='form-check-label' for='gridRadios2'> Static colour </label> </div></div><div id='color-wrapper'> <div> <div class='container p-0' id='color-table'> <div class='card box red pointer colorButton' id='red'></div><div class='card box pink pointer colorButton' id='pink'></div><div class='card box yellow pointer colorButton' id='yellow'></div><div class='card box green pointer colorButton' id='green'></div><div class='card box cyan pointer colorButton' id='cyan'></div><div class='card box blue pointer colorButton' id='blue'></div><div class='card box white pointer colorButton' id='white'></div><div class='card box black pointer colorButton' id='black'></div></div></div><div> <div id='preview-wrapper' class='card inverted p-4'> <div class='card box inverted red preview fade' id='colorPreview'></div></div></div></div></div><div class='tab-pane fade' id='settings'> <form action='#' id='networkForm'> <div> <label for='inputSSID'>Wifi name</label> <div class='input-and-button'> <input type='text' class='form-95' id='inputSSID'> <button type='button' id='wifiValueHelp' class='btn ml-2 btn-primary'>?</button> </div></div><div class='mt-1'> <label for='inputPW' >Wifi password</label> <input type='password' class='form-95' id='inputPW'> </div><div class='text-right'> <button type='submit' id='submitWifi' class='btn btn-primary mt-2'>Submit</button> </div></form> </div></div></div></div></div><div class='modal fade' id='modal' tabindex='-1'> <div class='modal-dialog' id='modal-dialog'> <div class='modal-content'> <div class='modal-header'> <h5 class='modal-title' id='modalLabel'></h5> <button type='button' class='btn' id='closeModalX'> <span id='x'>×</span> </button> </div><div class='modal-body'> <div class='modal-icon-wrapper'> <img class='modal-icon' id='error-icon' src='error.svg' width='50px'/> <img class='modal-icon' id='info-icon' src='info.svg' width='50px'/> </div><div id='modalContent' class='ml-3'></div></div><div class='modal-footer'> <button type='button' class='btn btn-primary' id='closeModal'>Close</button> </div></div></div></div><div id='backdrop' class='modal-backdrop fade'></div><script src='script.js'></script></body></html>"));
}

void handleScript() {
  server.send(200, "text/javascript",  F("class LampConfig{doc;colorPickerDisabled=!1;pickedColor='';defaultColor='red';activeTab='home-tab';elements={};colors={red:{r:1,g:0,b:0},pink:{r:1,g:0,b:1},yellow:{r:1,g:1,b:0},green:{r:0,g:1,b:0},cyan:{r:0,g:1,b:1},blue:{r:0,g:0,b:1},white:{r:1,g:1,b:1},black:{r:0,g:0,b:0}};constructor(e){this.doc=e,this.elements.ssid=this.doc.getElementById('inputSSID'),this.elements.wifipw=this.doc.getElementById('inputPW'),this.elements.wifiValueHelp=this.doc.getElementById('wifiValueHelp'),this.elements.modal=this.doc.getElementById('modal'),this.elements.colorPreview=this.doc.getElementById('colorPreview'),this.elements.backdrop=this.doc.getElementById('backdrop'),this.elements.networkForm=this.doc.getElementById('networkForm'),this.start()}start(){this.addModalListener(),this.setInitalConfig(),this.addEventListenerToList(this.doc.getElementsByClassName('nav-link'),e=>{e.preventDefault(),this.toggleTabs(e.target)}),this.addEventListenerToList(this.doc.getElementsByName('gridRadios'),e=>{this.toggleRadioButtons('fade'==e.target.value)}),this.addEventListenerToList(this.doc.getElementsByClassName('colorButton'),e=>{this.changeColor(e.target.id)}),this.elements.networkForm.onsubmit=e=>{e.preventDefault(),this.submitWifi()},this.addEventListenerToList([this.elements.wifiValueHelp],e=>{this.openWifiValueHelp()})}sendHttpRequest(e,t,s,i,o){var l=new XMLHttpRequest;l.open(t,e,!0),l.timeout=5e3,l.setRequestHeader('Content-Type','application/json'),l.ontimeout=e=>{o()},l.onload=i,s?l.send(JSON.stringify(s)):l.send()}toggleColorPicker(){this.doc.getElementById('color-wrapper').classList.toggle('disabled')}addEventListenerToList(e,t){for(let s=0;s<e.length;s++)e[s].onclick=t}clearColorPreview(){this.elements.colorPreview.classList.remove('red','pink','yellow','green','cyan','blue','white','black')}changeColor(e){if(!this.colorPickerDisabled){let t=this.colors[e];e!==this.pickedColor&&(this.clearColorPreview(),this.sendHttpRequest('change?r='+t.r+'&g='+t.g+'&b='+t.b,'GET',null,t=>{this.elements.colorPreview.classList.add(e),this.pickedColor=e}))}}changeColorMode(e){this.sendHttpRequest('/fade?fade='+e,'GET',null,e=>{})}isValidSSID(e){return e.length>=2&&e.length<=32}isValidWifiPW(e){return e.length>=8&&e.length<=70}showModal(e,t,s){switch(this.doc.getElementById('modalLabel').innerHTML=e,this.doc.getElementById('modalContent').innerHTML=t,this.elements.modal.classList.add('show'),this.elements.backdrop.classList.add('show'),s){case'e':this.doc.getElementById('error-icon').classList.add('show');break;case'i':this.doc.getElementById('info-icon').classList.add('show')}}hideModal(){this.elements.modal.classList.remove('show'),this.elements.backdrop.classList.remove('show');let e=this.doc.getElementsByClassName('modal-icon');for(let t of e)t.classList.remove('show')}addModalListener(){let e=[];e.push(this.elements.modal),e.push(this.doc.getElementById('closeModalX')),e.push(this.doc.getElementById('closeModal')),this.addEventListenerToList(e,()=>this.hideModal()),this.doc.getElementById('modal-dialog').onclick=e=>{e.stopPropagation()}}toggleRadioButtons(e){this.colorPickerDisabled==e||(this.toggleColorPicker(),this.colorPickerDisabled=!this.colorPickerDisabled,this.changeColorMode(e?'1':'0'),e||this.changeColor(this.defaultColor))}showProgressModal(e){this.showModal(e,'<div class=\"progress mr-3\"><div class=\"progress-bar progress-bar-blocks\"></div></div>')}openWifiValueHelp(){this.showProgressModal('Scanning for WiFi networks...'),this.sendHttpRequest('scanWifi','GET',null,e=>{try{this.hideModal();let t=JSON.parse(e.target.response);if(t.wifi.length>0){let s='<div class=\"button-list\">';for(let i of t.wifi)s+='<button class=\"btn ml-2 btn-primary network-buttons\" type=\"button\" value=\"'+i.name+'\">'+i.name+'</button>';s+='</div>',this.showModal('Available WiFi networks',s,'i'),this.addEventListenerToList(this.doc.getElementsByClassName('network-buttons'),e=>{this.preselectSSID(e.target.value),this.hideModal()})}else this.showModal('Available WiFi networks','Can not find any WiFi networks','i')}catch(o){this.hideModal(),this.showModal('An Error occurred!','Can not scan for WiFi networks.','e'),console.error(o)}})}preselectSSID(e){this.elements.ssid.value=e}submitWifi(){let e=this.elements.ssid.value,t=this.elements.wifipw.value;this.showProgressModal('Connecting...'),this.isValidSSID(e)&&this.isValidWifiPW(t)&&this.sendHttpRequest('/settingsPost','POST',e+',.,'+t,e=>{if(this.hideModal(),200!=e.target.status)this.showModal('An Error occurred!','WiFi credentials are not correct.','e');else{this.elements.networkForm.reset();let t='Please connect your device to the same WiFi in order to continue.<br>';try{let s=JSON.parse(e.target.response);t='Please connect your device to the WiFi: '+s.wifi+' in order to continue.<br>',s.hostname&&s.ip&&(t+='Use hostname: '+s.hostname+' or IP: '+s.ip+' to open up this site again in the new network.')}catch(i){console.error(i)}this.showModal('WiFi connected!',t,'i'),this.doc.getElementById('settingsTabNavItem').style.display='none',this.toggleTabs(this.doc.getElementById('home-tab'))}},()=>{this.showModal('Error occurred!','WiFi credentials are not correct.','e')})}setInitalConfig(){this.sendHttpRequest('getMode','GET',null,e=>{let t={doFade:!1,isAccessPoint:!0};try{let s=JSON.parse(e.target.response);t.doFade=!!s.doFade,t.isAccessPoint=!s.wifiMode}catch(i){console.error(i)}t.doFade?(this.toggleColorPicker(),this.colorPickerDisabled=!0,this.doc.getElementById('gridRadios1').checked=!0):this.doc.getElementById('gridRadios2').checked=!0,t.isAccessPoint||(this.doc.getElementById('settingsTabNavItem').style.display='none')})}toggleTabs(e){if(e!=this.activeTab){this.activeTab=e;let t=this.doc.getElementsByClassName('nav-link'),s=this.doc.getElementsByClassName('tab-pane');for(let i of t)i.classList.remove('active');for(let o of(e.classList.add('active'),s))o.id==e.id.replace('-tab','')?(o.classList.add('active'),o.classList.add('show')):(o.classList.remove('active'),o.classList.remove('show'))}}}document.addEventListener('DOMContentLoaded',function(){new LampConfig(document)});"));
}

void handleStyle() {
  server.send(200, "text/css",  F(".btn,.pointer{cursor:pointer}.modal-dialog,body>div:first-child{max-width:600px}.progress-bar{-webkit-box-orient:vertical;-webkit-box-direction:normal}.card-header,.progress,.progress-bar,header{overflow:hidden}header video,html{min-height:100%}.list-group-item,header,header .container,html{position:relative}body{font-family:Tahoma,'MS Sans Serif',Verdana,Segoe,sans-serif;background:teal;color:#fff;padding-bottom:28px}a{color:#2d49eb}.input-and-button{display:flex;align-items:center}.input-and-button button{margin-top:4px}header{background-color:#000;height:100vh;min-height:25rem;width:100%}header video{position:absolute;top:50%;left:50%;min-width:100%;width:auto;height:auto;z-index:0;-ms-transform:translateX(-50%) translateY(-50%);-moz-transform:translateX(-50%) translateY(-50%);-webkit-transform:translateX(-50%) translateY(-50%);transform:translateX(-50%) translateY(-50%)}header .container{z-index:2}::-webkit-scrollbar{width:12px}::-webkit-scrollbar-button:end:increment,::-webkit-scrollbar-button:start:decrement{display:block}::-webkit-scrollbar-button:vertical:end:decrement,::-webkit-scrollbar-button:vertical:start:increment{display:none}::-webkit-scrollbar-thumb:vertical{border:1px outset #fff;border-shadow:1px 1px #000;height:40px;background-color:silver}::-webkit-scrollbar-corner:vertical{background-color:#000}.btn,.btn:hover,.card,.list-group{background:silver}:-webkit-scrollbar-button:start:decrement,::-webkit-scrollbar-button:end:increment{display:block}::-webkit-scrollbar-button:horizontal:end:decrement,::-webkit-scrollbar-button:horizontal:start:increment{display:none}::-webkit-scrollbar-button:horizontal:decrement,::-webkit-scrollbar-button:horizontal:increment{width:18px;height:18px;margin:1px 1px 1px 10px;border:1px outset #fff;border-shadow:1px 1px #000}::-webkit-scrollbar-track-piece{margin:1px}::-webkit-scrollbar-thumb:horizontal{border:1px outset #fff;border-shadow:1px 1px #000;height:40px;background-color:silver}.btn{border-width:2px;border-style:outset;border-color:buttonface #424242 #424242 buttonface;color:#000;padding:0 0 4px;border-radius:1px}.btn:hover{border:2px inset #fff;color:#424242;box-shadow:-1px -1px #000}.btn:active{border:2px inset #fff!important;color:#424242;box-shadow:-1px -1px #000!important;outline:0!important}.btn-primary{padding-left:8px;padding-right:8px}button:focus{outline:dotted 1px}.btn.disabled,.btn:disabled{cursor:default;background-color:silver;border-style:outset;border-color:buttonface #424242 #424242 buttonface;color:grey;text-shadow:1px 1px #fff}.card,.list-group{border:2px solid;border-color:#fff #424242 #424242 #fff;color:#212529}.card.inverted{border-color:#424242 #fff #fff #424242}.card.box{width:40px;height:40px;display:flex;justify-content:center;align-items:center}.card.box.preview{width:25vw;height:25vw;max-width:150px;max-height:150px}#color-wrapper{display:grid;column-gap:20px;grid-template-columns:80px 1fr}#color-wrapper.disabled{opacity:30%}#color-table{display:grid;row-gap:3px;grid-template-columns:50% 50%;min-width:85px}#preview-wrapper{display:flex;justify-content:center;align-items:center}#x{font-family:'Microsoft Sans Serif'}.card.box.red{background-color:#ff0e00}.card.box.pink{background-color:#ff00fe}.card.box.yellow{background-color:#faff08}.card.box.green{background-color:#00ff7b}.card.box.cyan{background-color:#00feff}.card.box.blue{background-color:#3400fe}.card.box.white{background-color:#fff}.card.box.black{background-color:#000}.card.square,.card.square .card-header{border-radius:0}.card.w95 .card-header{background:#08216b}.card-header{background:-webkit-linear-gradient(left,#08216b,#a5cef7);color:#fff;display:block;white-space:nowrap;text-overflow:ellipsis;padding-top:2px;padding-bottom:1px;text-align:left}.card-header.icon{padding-left:4px}.header-inactive{background:gray!important}.list-group-item{display:block;background:0 0;color:#212529}.list-group-item-primary{color:#fff;background:-webkit-linear-gradient(left,#08216b,#a5cef7)}.dropdown-item:hover,.list-group-item-action:hover{color:#08216b}.navbar-95{background:silver;margin:0;border:1px outset #fff;min-height:40px;padding:0 6px;color:#212529}.navbar-brand{color:#212529;padding:0 6px}.nav-link{text-decoration:none;display:inline-block;padding:0 9px;color:#212529}.dropdown-menu{display:none;min-width:119px;padding:0 0 2px;margin-left:12px;font-size:12px;list-style-type:none;background:silver;border:1.8px outset #fff;border-radius:0;-webkit-box-shadow:1.5px 1.5px #000;box-shadow:1.5px 1.5px #000}.dropdown-item{padding:1px 0 1px 2px}.navbar-toggler{width:auto}#modalContent,.form-95,.taskbar{width:100%}.taskbar{cursor:default;background-color:silver;margin:16px 0 0;padding:0 8px;border-top:2px outset #fff;z-index:228;position:absolute;bottom:0}.taskbar .start-button{cursor:default;display:inline-block;border:1px outset #fff;padding:0 0 0 2px;box-shadow:1px 1px #000;margin-bottom:2px;font-size:14px}.form-95,.form-check-label::before{background:#fff;box-shadow:-1px -1px 0 0 #828282}.taskbar .time{color:#212529;margin-top:2px;text-align:right;font-size:14px;margin-right:0}#page-content{flex:1 0 auto}.icon-16{margin-bottom:2px;max-height:16px}.icon-16-4{margin-bottom:4px;max-height:16px}.form-95{border:2px inset #d5d5d5;color:#424242;margin-top:4px;padding-left:2px}.bootstrap-select,[contenteditable].form-control:focus,[type=email].form-control:focus,[type=password].form-control:focus,[type=tel].form-control:focus,[type=text].form-control:focus,input.form-control:focus,input[type=email]:focus,input[type=number]:focus,input[type=password]:focus,input[type=text]:focus,textarea.form-control:focus,textarea:focus{outline:0!important}input[type=checkbox],input[type=radio]{position:absolute;left:-9999px}.form-check-label::after,.form-check-label::before{content:'';position:absolute;top:0;left:0}.form-check-label{margin-left:6px}input[type=radio]+.form-check-label::after,input[type=radio]+.form-check-label::before{border-radius:50%}.form-check-label::before{height:20px;width:20px;top:3px;padding-right:2px;border:2px inset #d5d5d5}input[type=radio]+.form-check-label::after{display:none;width:8px;height:8px;margin:6px;top:3px;background:#000}input[type=checkbox]+.form-check-label::after{display:none;background-image:url('data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 8 8'%3e%3cpath fill='%23000' d='M6.564.75l-3.59 3.612-1.538-1.55L0 4.26 2.974 7.25 8 2.193z'/%3e%3c/svg%3e');width:12px;height:12px;margin:4px;top:3px}.modal-icon-wrapper .modal-icon.show,input:checked+.form-check-label::after{display:block}.progress-bar{display:-webkit-box;display:-ms-flexbox;display:flex;-ms-flex-direction:column;flex-direction:column;-webkit-box-pack:center;-ms-flex-pack:center;justify-content:center}.tab-content{background:silver;border:2px solid;border-color:silver #424242 #424242 #fff;padding:1rem 1.4rem}.nav-tabs{border-bottom:2px solid #fff}.nav-tabs .nav-item{position:relative;margin-bottom:0;background:silver;color:#000;border-top-left-radius:4px;border-top-right-radius:4px;border-right:2px solid #5a5a5a;border-top:2px solid #fff;box-sizing:border-box}.nav-tabs .nav-item .nav-link{color:#000;padding:.2rem 1.8rem;box-sizing:border-box;transform:none}.nav-tabs .nav-item .nav-link.active{position:relative;background:silver}.nav-tabs .nav-item:not(:first-child) .nav-link.active{border-left:1px solid #5a5a5a}.nav-tabs .nav-item:first-child{border-left:2px solid #fff}.nav-tabs .nav-item .nav-link.active:after{content:'';display:block;width:100%;height:2px;position:absolute;left:0;bottom:-2px;background:silver}.nav-tabs .nav-link{border:0;padding:1rem}.modal-content{box-shadow:1px 1px 0 0 #424242;border:1px solid #fff;border-right-color:#848484;border-bottom-color:#848484;background:silver;padding:2px}.modal-body{color:#000;display:flex}.button-list{display:flex;flex-direction:column;width:90%;row-gap:5px}.modal-dialog{width:calc(100% - 50px);margin:1.75rem auto}.modal-icon-wrapper .modal-icon{display:none}.modal-header{height:32px;background:-webkit-linear-gradient(left,#08216b,#a5cef7)!important;color:#fff;padding:0 6px}.modal-header .btn{margin-top:5px;padding-bottom:10px;height:22px;width:22px}.modal-header .btn span{position:absolute;top:6px;right:14px}.modal-title{padding-top:2px;padding-bottom:1px}.modal-footer{padding:6px}.table{background:#fff;border-color:silver}.table-bordered,.table-bordered td,.table-bordered th{border:1px solid silver}.table td,.table th{padding:.75rem;vertical-align:top;border-top:1px solid silver}.progress{display:flex;height:1.2rem;font-size:.75rem;background-color:silver;border-radius:0;border:1px inset #d5d5d5;color:#424242;position:relative}.progress-bar{display:-webkit-box;display:-ms-flexbox;display:flex;-ms-flex-direction:column;flex-direction:column;-webkit-box-pack:center;-ms-flex-pack:center;justify-content:center;white-space:nowrap;color:#fff;text-align:center;background-color:#1a0094;transition:left .6s}.progress-bar-blocks{background-image:linear-gradient(90deg,transparent 75%,#d5d5d5 25%);background-size:1rem 1rem;width:40px;-webkit-animation:4s linear infinite progress;animation:4s linear infinite progress;height:100%;position:absolute;top:0;left:0}@-webkit-keyframes progress{0%{left:-40px}100%{left:calc(100% + 40px)}}@keyframes progress{0%{left:-40px}100%{left:calc(100% + 40px)}}"));
}

void handleBootstrap() {
  server.send(200, "text/css", F(".card,.form-check{position:relative}body{font-size:1rem;font-weight:400;line-height:1.5;color:#212529}body,caption{text-align:left}.card{word-wrap:break-word;display:flex;-ms-flex-direction:column;flex-direction:column;min-width:0;background-color:#fff;background-clip:border-box;border:1px solid rgba(0,0,0,.125);border-radius:.25rem}.card-footer,.card-header{padding:.75rem 1.25rem;background-color:rgba(0,0,0,.03)}.card-header{margin-bottom:0;border-bottom:1px solid rgba(0,0,0,.125)}.card-header:first-child{border-radius:calc(.25rem - 1px) calc(.25rem - 1px) 0 0}.card-body{-ms-flex:1 1 auto;flex:1 1 auto;min-height:1px;padding:1.25rem}.container,.container-fluid,.container-lg,.container-md,.container-sm,.container-xl{width:100%;padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}a{color:#007bff;text-decoration:none;background-color:transparent}.nav-link,.navbar{padding:.5rem 1rem}.carousel-item-next,.carousel-item-prev,.carousel-item.active,.dropdown-menu.show,.nav-link,.nav-tabs .nav-link{border:1px solid transparent;border-top-left-radius:.25rem;border-top-right-radius:.25rem}.nav-tabs .nav-item.show .nav-link,.nav-tabs .nav-link.active{color:#495057;background-color:#fff;border-color:#dee2e6 #dee2e6 #fff}address,dl,ol,p,pre,ul{margin-bottom:1rem}.nav-tabs .nav-item{margin-bottom:-1px}.btn-group-toggle>.btn,.btn-group-toggle>.btn-group>.btn,.card-text:last-child,.form-check-label,.modal-title,.nav,ol ol,ol ul,ul ol,ul ul{margin-bottom:0}.nav-tabs{border-bottom:1px solid #dee2e6}.nav,.navbar{-ms-flex-wrap:wrap}.nav{display:-ms-flexbox;display:flex;flex-wrap:wrap;padding-left:0}.input-group-text input[type=checkbox],.input-group-text input[type=radio],.list-group-horizontal .list-group-item.active,dl,h1,h2,h3,h4,h5,h6,ol,p,ul{margin-top:0}.dropdown-menu,.nav,.navbar-nav{list-style:none}.fade{transition:opacity .15s linear}.badge:empty,.navbar-expand .navbar-toggler,.popover-header:empty,.tab-content>.tab-pane,.toast.hide{display:none}.form-check{display:block;padding-left:1.25rem}dd,label{margin-bottom:.5rem}label{display:inline-block}button,hr,input{overflow:visible}button,input,optgroup,select,textarea{margin:0;font-family:inherit;font-size:inherit;line-height:inherit}.form-check-input{position:absolute;margin-top:.3rem;margin-left:-1.25rem}input[type=checkbox],input[type=radio]{box-sizing:border-box;padding:0}.h1,.h2,.h3,.h4,.h5,.h6,h1,h2,h3,h4,h5,h6{margin-bottom:.5rem;font-weight:500;line-height:1.2}.h4,h4{font-size:1.5rem}*,::after,::before{box-sizing:border-box}.font-weight-normal{font-weight:400!important}.m-0{margin:0!important}.mt-0,.my-0{margin-top:0!important}.mr-0,.mx-0{margin-right:0!important}.mb-0,.my-0{margin-bottom:0!important}.ml-0,.mx-0{margin-left:0!important}.m-1{margin:.25rem!important}.mt-1,.my-1{margin-top:.25rem!important}.mr-1,.mx-1{margin-right:.25rem!important}.mb-1,.my-1{margin-bottom:.25rem!important}.ml-1,.mx-1{margin-left:.25rem!important}.m-2{margin:.5rem!important}.mt-2,.my-2{margin-top:.5rem!important}.mr-2,.mx-2{margin-right:.5rem!important}.mb-2,.my-2{margin-bottom:.5rem!important}.ml-2,.mx-2{margin-left:.5rem!important}.m-3{margin:1rem!important}.mt-3,.my-3{margin-top:1rem!important}.mr-3,.mx-3{margin-right:1rem!important}.mb-3,.my-3{margin-bottom:1rem!important}.ml-3,.mx-3{margin-left:1rem!important}.m-4{margin:1.5rem!important}.mt-4,.my-4{margin-top:1.5rem!important}.mr-4,.mx-4{margin-right:1.5rem!important}.mb-4,.my-4{margin-bottom:1.5rem!important}.ml-4,.mx-4{margin-left:1.5rem!important}.m-5{margin:3rem!important}.mt-5,.my-5{margin-top:3rem!important}.mr-5,.mx-5{margin-right:3rem!important}.mb-5,.my-5{margin-bottom:3rem!important}.ml-5,.mx-5{margin-left:3rem!important}.p-0{padding:0!important}.pt-0,.py-0{padding-top:0!important}.pr-0,.px-0{padding-right:0!important}.pb-0,.py-0{padding-bottom:0!important}.pl-0,.px-0{padding-left:0!important}.p-1{padding:.25rem!important}.pt-1,.py-1{padding-top:.25rem!important}.pr-1,.px-1{padding-right:.25rem!important}.pb-1,.py-1{padding-bottom:.25rem!important}.pl-1,.px-1{padding-left:.25rem!important}.p-2{padding:.5rem!important}.pt-2,.py-2{padding-top:.5rem!important}.pr-2,.px-2{padding-right:.5rem!important}.pb-2,.py-2{padding-bottom:.5rem!important}.pl-2,.px-2{padding-left:.5rem!important}.p-3{padding:1rem!important}.pt-3,.py-3{padding-top:1rem!important}.pr-3,.px-3{padding-right:1rem!important}.pb-3,.py-3{padding-bottom:1rem!important}.pl-3,.px-3{padding-left:1rem!important}.p-4{padding:1.5rem!important}.pt-4,.py-4{padding-top:1.5rem!important}.pr-4,.px-4{padding-right:1.5rem!important}.pb-4,.py-4{padding-bottom:1.5rem!important}.pl-4,.px-4{padding-left:1.5rem!important}.p-5{padding:3rem!important}.pt-5,.py-5{padding-top:3rem!important}.pr-5,.px-5{padding-right:3rem!important}.pb-5,.py-5{padding-bottom:3rem!important}.pl-5,.px-5{padding-left:3rem!important}.m-n1{margin:-.25rem!important}.mt-n1,.my-n1{margin-top:-.25rem!important}.mr-n1,.mx-n1{margin-right:-.25rem!important}.mb-n1,.my-n1{margin-bottom:-.25rem!important}.ml-n1,.mx-n1{margin-left:-.25rem!important}.m-n2{margin:-.5rem!important}.mt-n2,.my-n2{margin-top:-.5rem!important}.mr-n2,.mx-n2{margin-right:-.5rem!important}.mb-n2,.my-n2{margin-bottom:-.5rem!important}.ml-n2,.mx-n2{margin-left:-.5rem!important}.m-n3{margin:-1rem!important}.mt-n3,.my-n3{margin-top:-1rem!important}.mr-n3,.mx-n3{margin-right:-1rem!important}.mb-n3,.my-n3{margin-bottom:-1rem!important}.ml-n3,.mx-n3{margin-left:-1rem!important}.m-n4{margin:-1.5rem!important}.mt-n4,.my-n4{margin-top:-1.5rem!important}.mr-n4,.mx-n4{margin-right:-1.5rem!important}.mb-n4,.my-n4{margin-bottom:-1.5rem!important}.ml-n4,.mx-n4{margin-left:-1.5rem!important}.m-n5{margin:-3rem!important}.mt-n5,.my-n5{margin-top:-3rem!important}.mr-n5,.mx-n5{margin-right:-3rem!important}.mb-n5,.my-n5{margin-bottom:-3rem!important}.ml-n5,.mx-n5{margin-left:-3rem!important}.m-auto{margin:auto!important}.mt-auto,.my-auto{margin-top:auto!important}.mr-auto,.mx-auto{margin-right:auto!important}.mb-auto,.my-auto{margin-bottom:auto!important}.ml-auto,.mx-auto{margin-left:auto!important}.text-right{text-align:right!important}.modal.show{display:block!important}.tab-content>.active{display:block}.modal{position:fixed;top:0;left:0;z-index:1050;display:none;width:100%;height:100%;overflow:hidden;outline:0}.modal-dialog{position:relative;width:auto;margin:.5rem;pointer-events:none}.modal-content{position:relative;display:-ms-flexbox;display:flex;-ms-flex-direction:column;flex-direction:column;width:100%;pointer-events:auto;background-color:#fff;background-clip:padding-box;border:1px solid rgba(0,0,0,.2);border-radius:.3rem;outline:0}.modal-header{display:-ms-flexbox;display:flex;-ms-flex-align:start;align-items:flex-start;-ms-flex-pack:justify;justify-content:space-between;padding:1rem;border-bottom:1px solid #dee2e6;border-top-left-radius:calc(.3rem - 1px);border-top-right-radius:calc(.3rem - 1px)}.h5,h5{font-size:1.25rem}.modal-title,.popover,.tooltip{line-height:1.5}.modal-body{position:relative;-ms-flex:1 1 auto;flex:1 1 auto;padding:1rem}.modal-footer{display:-ms-flexbox;display:flex;-ms-flex-wrap:wrap;flex-wrap:wrap;-ms-flex-align:center;align-items:center;-ms-flex-pack:end;justify-content:flex-end;padding:.75rem;border-top:1px solid #dee2e6;border-bottom-right-radius:calc(.3rem - 1px);border-bottom-left-radius:calc(.3rem - 1px)}.modal-backdrop{position:fixed;top:0;left:0;z-index:1040;width:100vw;height:100vh;background-color:#000;display:none}.modal-backdrop.show{display:block;opacity:.5}"));
}

void handleIcon() {
  server.send(200, "image/svg+xml", F("<?xml version='1.0' encoding='UTF-8' standalone='no'?><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg width='100%' height='100%' viewBox='0 0 512 512' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' xml:space='preserve' xmlns:serif='http://www.serif.com/' style='fill-rule:evenodd;clip-rule:evenodd;stroke-linejoin:round;stroke-miterlimit:2;'><clipPath id='_clip1'><rect id='icon' x='-0' y='-0' width='512' height='512'/></clipPath><g clip-path='url(#_clip1)'><path d='M322.35,462.233l-0,9.063c-0,11.393 0.109,20.628 -11.284,20.628l-26.656,5.607c-9.96,10.006 -18.106,14.469 -28.353,14.469c-10.247,0 -18.507,-4.463 -28.467,-14.469l-26.656,-5.607c-11.393,0 -11.284,-9.235 -11.284,-20.628l0,-0.057l132.7,-9.006Zm-132.7,-6.845l0,-14.581c0,-3.191 -0.19,-6.605 -0.553,-10.164l133.806,-0c-0.363,3.559 -0.553,6.973 -0.553,10.164l-0,9.008l-132.7,5.573Zm34.801,-44.822l-38.878,0c-4.878,-20.092 -13.714,-40.502 -24.201,-50.463c-22.812,-21.669 -39.468,-68.533 -39.468,-96.784c-0,-67.819 34.379,-138.301 133.977,-138.301c99.599,-0 134.215,70.482 134.215,138.301c0,28.251 -16.656,75.115 -39.468,96.784c-3.692,3.507 -7.179,8.308 -10.361,13.935c-5.856,10.357 -10.679,23.509 -13.84,36.528l-36.607,0c-0.547,-8.018 -3.79,-56.046 -5.522,-93.202c19.566,-7.04 31.859,-23.845 37.043,-39.436c5.606,-16.863 -5.301,-35.243 -21.483,-35.57c-6.917,-0.139 -12.02,1.267 -15.83,3.576c-8.499,5.15 -10.722,17.572 -10.993,28.208c-0.176,6.894 1.31,23.397 1.929,37.621c-4.16,1.162 -13.336,2.126 -17.696,2.126c-4.788,-0 -13.394,-0.85 -17.961,-2.126c0.619,-14.224 -0.166,-23.061 -0.342,-29.955c-0.271,-10.636 -2.494,-20.868 -10.993,-26.018c-3.81,-2.309 -8.913,-3.715 -15.83,-3.576c-16.182,0.327 -27.089,16.517 -21.483,33.38c5.184,15.59 19.748,24.73 39.314,31.77c-1.732,37.156 -4.975,85.184 -5.522,93.202Zm56.268,0l-47.166,0c0.619,-9.124 3.644,-54.241 5.373,-90.471c4.598,1.169 12.121,1.92 18.338,1.92c6.217,-0 14.094,-0.906 18.081,-1.92c1.73,36.23 4.754,81.347 5.374,90.471Zm-50.363,-101.847c-15.149,-6.161 -26.804,-12.842 -30.985,-25.417c-3.752,-11.286 2.146,-22.791 12.976,-23.009c4.57,-0.093 7.993,0.649 10.51,2.175c2.817,1.707 4.387,4.35 5.38,7.387c1.198,3.665 1.538,7.882 1.646,12.136c0.162,6.36 1.009,13.906 0.473,26.728Zm53.559,0c-0.535,-12.822 -1.96,-28.034 -1.798,-34.394c0.108,-4.254 0.448,-8.471 1.646,-12.136c0.993,-3.037 1.614,-7.206 5.38,-9.577c2.489,-1.568 5.94,-2.268 10.51,-2.175c10.83,0.218 16.728,13.913 12.976,25.199c-4.181,12.575 -13.565,26.922 -28.714,33.083Zm-15.318,-303.953l-2.871,70.956c0,2.632 -2.133,4.765 -4.766,4.765l-9.602,-0.638c-2.632,0 -4.765,-2.133 -4.765,-4.765l-3.19,-70.318c0,-2.632 2.134,-4.766 4.766,-4.766l15.662,0c2.632,0 4.766,2.134 4.766,4.766Zm207.03,131.616l-62.17,31.803c-2.303,1.274 -5.203,0.439 -6.477,-1.864l-3.753,-5.591c-1.274,-2.304 -0.44,-5.203 1.864,-6.477l58.342,-39.917c2.303,-1.274 5.203,-0.44 6.477,1.863l7.581,13.706c1.274,2.303 0.44,5.203 -1.864,6.477Zm-439.254,-0c-2.304,-1.274 -3.138,-4.174 -1.864,-6.477l7.581,-13.706c1.274,-2.303 4.174,-3.137 6.477,-1.863l59.746,36.842c2.303,1.274 3.137,4.174 1.863,6.477l-3.753,8.092c-1.274,2.303 -4.174,3.137 -6.477,1.863l-63.573,-31.228Z' style='fill:#fff;'/></g></svg>"));
}

void handleErrorIcon() {
  server.send(200, "image/svg+xml", F("<?xml version='1.0' encoding='UTF-8' standalone='no'?><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg width='100%' height='100%' viewBox='0 0 350 350' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' xml:space='preserve' xmlns:serif='http://www.serif.com/' style='fill-rule:evenodd;clip-rule:evenodd;stroke-linejoin:round;stroke-miterlimit:2;'><g id='error'><g id='error1' serif:id='error'><circle cx='189.991' cy='191.153' r='150' style='fill-opacity:0.3;'/><circle cx='175' cy='169.533' r='155'/><circle cx='175' cy='169.533' r='150' style='fill:#f00;'/><path d='M153.787,169.533l-77.782,-77.782l21.213,-21.213l77.782,77.782l77.782,-77.782l21.213,21.213l-77.782,77.782l77.782,77.782l-21.213,21.213l-77.782,-77.782l-77.782,77.782l-21.213,-21.213l77.782,-77.782Z' style='fill:#fff;'/></g></g></svg>"));
}

void handleInfoIcon() {
  server.send(200, "image/svg+xml", F("<?xml version='1.0' encoding='UTF-8' standalone='no'?><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg width='100%' height='100%' viewBox='0 0 350 350' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' xml:space='preserve' xmlns:serif='http://www.serif.com/' style='fill-rule:evenodd;clip-rule:evenodd;stroke-linejoin:round;stroke-miterlimit:2;'><g id='info'><g><path d='M160.269,291.512c-79.487,-7.266 -135.547,-61.995 -135.547,-127.794c0,-70.742 71.479,-128.176 159.52,-128.176c88.042,-0 159.521,57.434 159.521,128.176c-0,61.022 -49.185,111.696 -120.309,124.593c0.227,13.885 0.275,51.531 -1.135,57.083c-1.427,5.619 -42.438,-18.744 -51.718,-33.625c-3.587,-5.751 -9.029,-13.736 -10.332,-20.257Z' style='fill-opacity:0.24;'/><path d='M151.027,275.631c-79.487,-7.266 -135.547,-61.995 -135.547,-127.794c-0,-70.742 71.478,-128.176 159.52,-128.176c88.042,-0 159.52,57.434 159.52,128.176c0,61.022 -49.185,111.697 -120.308,124.593c0.227,13.885 0.275,51.531 -1.135,57.083c-1.427,5.62 -42.439,-18.743 -51.719,-33.625c-3.586,-5.751 -9.028,-13.736 -10.331,-20.257Z'/><path d='M156.909,269.396c-75.236,-6.877 -133.768,-58.26 -133.768,-120.54c0,-66.96 67.657,-121.323 150.991,-121.323c83.334,0 153.619,50.457 153.812,117.417c0.19,65.902 -53.162,110.05 -120.483,122.257c0.215,13.143 0.633,46.201 -0.443,53.611c-0.789,5.43 -34.617,-18.204 -43.401,-32.29c-3.394,-5.444 -5.475,-12.959 -6.708,-19.132Z' style='fill:#fff;'/><path d='M122.983,99.918c0,-9.99 4.534,-18.365 13.601,-25.127c9.068,-6.762 22.015,-10.143 38.843,-10.143c16.829,-0 29.815,3.227 38.959,9.682c9.144,6.454 13.716,15.983 13.716,28.584c0,8.299 -2.036,15.638 -6.109,22.015c-4.072,6.378 -10.271,14.045 -16.943,19.48c-7.742,6.305 -24.554,13.435 -28.894,21.566c-1.876,3.516 -2.484,27.294 -8.931,24.21c-6.795,-3.25 -7.944,-28.693 -6.974,-37.017c0.756,-6.482 3.253,-13.716 6.762,-19.363c3.509,-5.648 14.293,-14.523 14.293,-14.523c4.457,-4.457 7.53,-8.645 9.221,-12.564c1.69,-3.919 2.536,-9.029 2.536,-15.33c-0,-14.907 -6.916,-22.36 -20.748,-22.36c-6.608,-0 -11.449,1.767 -14.523,5.302c-3.073,3.534 -4.61,6.915 -4.61,10.143c-0,3.227 0.461,6.07 1.383,8.529c0,-0 4.362,-2.069 8.406,-0.386c3.358,1.398 5.163,5.994 5.033,10.157c-0.142,4.532 -2.324,9.89 -7.564,13.172c-4.295,2.689 -11.331,5.411 -17.632,5.411c-6.301,0 -11.18,-2.036 -14.638,-6.109c-3.458,-4.072 -5.187,-9.182 -5.187,-15.329Zm43.915,130.937c-5.763,0 -10.066,-1.652 -12.909,-4.956c-2.843,-3.304 -4.265,-7.454 -4.265,-12.449c0,-4.994 1.537,-9.144 4.611,-12.448c3.073,-3.304 7.3,-4.956 12.678,-4.956c5.379,-0 9.682,1.422 12.91,4.265c3.227,2.843 4.841,7.031 4.841,12.563c-0,5.533 -1.537,9.913 -4.611,13.14c-3.073,3.227 -7.492,4.841 -13.255,4.841Z' style='fill:#00f;fill-rule:nonzero;'/></g></g></svg>"));
}

void handleChangeRequest() {

  String templateData = "";
  String rArg = server.arg(0);
  String gArg = server.arg(1);
  String bArg = server.arg(2);
  if (rArg == "1" || rArg ==  "0") {
    rValue = rArg.toInt();
  }
  if (gArg == "1" || gArg ==  "0") {
    gValue = gArg.toInt();
  }
  if (bArg == "1" || bArg ==  "0") {
    bValue = bArg.toInt();
  }
  updateLight();
  doFade = false;
  String result = "{\"doFade\":" + String(doFade) + "}";
  server.send(200, "text/json", result);       //Response to the HTTP request
  return;
}

void handlescanWifi(){
  server.send(200, "application/json", scanWiFi());       //Response to the HTTP request
  return;
}

void handleFadeRequest() { //Handler

  String fArg = server.arg(0);
  if (fArg == "1" || fArg ==  "0") {
    doFade = fArg == "1" ? true : false;
  }
  String result = "{\"doFade\":" + String(doFade) + "}";
  server.send(200, "application/json", result);       //Response to the HTTP request
  return;
}

void handleGetMode() { //Handler
  String result = "{\"doFade\":" + String(doFade) + ", \"wifiMode\": " + String(wifiMode) + " }";
  server.send(200, "application/json", result);       //Response to the HTTP request
  return;
}

void handleGetWifiMode() { //Handler
  String result = "{\"wifiMode\":" + String(wifiMode) + "}";
  server.send(200, "application/json", result);       //Response to the HTTP request
  return;
}



String getParameter(String inputString, int pos) {

  String returnValue = inputString.substring(inputString.indexOf(",.,"), inputString.length());

  if (pos == 1) {
    String returnValue2 = inputString;
    returnValue2.replace(returnValue, "");
    returnValue2.replace(",.,", "");
    returnValue2.replace("\"", "");
    return returnValue2;
  } else {
    returnValue.replace(",.,", "");
    returnValue.replace("\"", "");
    return returnValue;
  }


}




//LED logic
//-----------------------------
void applyRGB(int red, int green, int blue)
{
  analogWrite(PinRED, red);
  analogWrite(PinGREEN, green);
  analogWrite(PinBLUE, blue);
  return;
}

void updateLight() {
  digitalWrite(PinRED, rValue);
  digitalWrite(PinGREEN, gValue);
  digitalWrite(PinBLUE, bValue);
  return;
}

void loadColor() {
  rValue = LOW;
  gValue = LOW;
  bValue = HIGH;
  updateLight();
  return;
}

void warnColor() {
  while (true) {
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


void defaultColor() {
  rValue = HIGH;
  gValue = HIGH;
  bValue = HIGH;
  updateLight();
  return;
}




int rgbColor[] = {1023, 0, 0};
int decColor = 0;
int incColor = 1;

void fadeColors() {
  // cross-fade the two colours.
  static unsigned long int waitSince = 0;
  static unsigned long int waitSpan = 10;
  if (millis() - waitSince >= waitSpan) {
    waitSince = millis();
    rgbColor[decColor] -= 1;
    rgbColor[incColor] += 1;
    if (rgbColor[incColor] > 1010 || rgbColor[decColor] > 1010) {
      waitSpan = 100;
    } else {
      waitSpan = 5;
    }
    if (rgbColor[incColor] == 1023) {
      decColor = decColor == 2 ? 0 : decColor + 1;
      incColor = incColor == 2 ? 0 : incColor + 1;
    }
    applyRGB(rgbColor[0], rgbColor[1], rgbColor[2]);
  }
  return;
}

void fadeOff(int red, int green, int blue) {
  int inRed = 0;
  int inGreen = 0;
  int inBlue = 0;
  applyRGB(0, 0, 0);
  boolean isFadeIn = true;
  while (isFadeIn) {
    if (inRed < red) {
      inRed++;
    }
    if (inGreen < green) {
      inGreen++;
    }
    if (inBlue < blue) {
      inBlue++;
    }
    applyRGB(inRed, inGreen, inBlue);
    if ( ((inRed + inGreen + inBlue) % 2) == 0) {
      delay(1);
    }
    if (inRed == red && inGreen == green && inBlue == blue) {
      isFadeIn = false;
    }
  }

  delay(2000);

  while (red > 0 || green > 0 || blue > 0) {
    if (red > 0) {
      red--;
    }
    if (green > 0) {
      green--;
    }
    if (blue > 0) {
      blue--;
    }

    applyRGB(red, green, blue);
    if ( ((red + green + blue) % 2) == 0) {
      delay(1);
    }

  }
}
