#include "ESP8266WiFi.h"
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <DNSServer.h>

MDNSResponder mdns;
WiFiServer server(80);

const byte DNS_PORT = 53;
DNSServer dnsServer;

IPAddress ap_local_IP(8,8,8,8);
IPAddress ap_gateway(8,8,8,8);
IPAddress ap_subnet(255,255,255,0);

bool smartWiFi(String AP_name){
  EEPROM.begin(512);
  delay(500);

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }

  if ( esid.length() > 1 ) {
    WiFi.begin(esid.c_str(), epass.c_str());
    if ( testWifi() == 1 ) {
      Serial.println(WiFi.localIP());
      return false;
    }
  }

  setupAP(AP_name); 
  return true;
}

int testWifi(void) {
  int count = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( count < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("WiFi Connected");
      return(1);
    }
    delay(500);
    Serial.print(".");
    count++;
  }

  Serial.println(".");
  Serial.println("Time out, opening AP");
  Serial.println();
  return(0);
}

void setupAP(String AP_name) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  }

  delay(100);
  WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);

  char ap_ssid[50];
  AP_name.toCharArray(ap_ssid,50);
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", ap_local_IP);

  launchWeb(0);
}

void launchWeb(int webtype) {
  if(webtype == 1) {  
    if (!mdns.begin("esp8266", WiFi.localIP())) {
      Serial.println("Error setting up MDNS responder!");
      while(1) {
        delay(1000);
      }
    }
    
    server.begin();
  }
  else{
    Serial.println(WiFi.softAPIP());
    
    if (!mdns.begin("esp8266", WiFi.localIP())) {
      Serial.println("Error setting up MDNS responder!");
      while(1) {
        delay(1000);
      }
    }
    
    server.begin();
    int b = 1;
  
    while(b == 1) {
      b = mdns_setup();
      dnsServer.processNextRequest();
    }
  }  
}

int mdns_setup(){
  mdns.update();

  WiFiClient client = server.available();
  if (!client) {
    return(1);
  }

  while(client.connected() && !client.available()) {
    delay(1);
  }

  String req = client.readStringUntil('\r');

  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return(1);
  }
  req = req.substring(addr_start + 1, addr_end);
  client.flush();

  String s;

  if (req == "/") {
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
    s += "<html>";
    s += "<head>";
    s += "<meta http-equiv='X-UA-Compatible' content='IE=edge,chrome=1'>";
    s += "<meta name=”apple-mobile-web-app-capable” content=”yes”>";
    s += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
    s += "<meta name='viewport' content='width=device-width, initial-scale=1.0, minimum-scale=1.0'>";
    s += "<title>ESP8266 AP Setup</title>";
    s += "</head>";
    s += "<body>";
    s += "<div style = 'padding:30px;'>";
    s += "<div style = 'text-align:center; padding:30px; color: #252e31;'><b>ESP8266 WiFi Setup</b></div>";
    s += "<form method='get' action='a' style = 'max-width:460px; margin-left:auto; margin-right:auto; background-color:#F7F9FA; border-radius:5px; padding:60px 20px;'>";
    s += "<div class='content' style = 'margin:0 10%; padding: 0 0 10px;'>";
    s += "<form method='get' action='a'>";
    s += "<input name='ssid' autocomplete='off' placeholder='SSID' style = 'text-align:center; height: 40px; width: 100%; margin-bottom:10px; border: 1px solid #9da3a6; background: #fff; border-radius: 5px;'>";
    s += "<input name='pass' autocomplete='off' placeholder='Password' style = 'text-align:center; height: 40px; width: 100%; margin-bottom:10px; border: 1px solid #9da3a6; background: #fff; border-radius: 5px;'>";
    s += "<input type='submit' style = 'width: 100%; height: 44px; padding:0 10px; border:0; background-color: #394950; border-radius:5px; color:#fff; margin-top:10px;'>";
    s += "</div>";
    s += "</form>";
    s += "</div>";
    s += "<div style = 'bottom: 0; height: 30px; text-align:center; padding:30px; font-size: small;'>ID KAIST <br> CIDR Lab</div>";
    s += "</body>";
    s += "</html>\r\n\r\n";
  }
  else if ( req.startsWith("/generate_204") ) {
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
    s += "<html>";
    s += "<head>";
    s += "<meta http-equiv='X-UA-Compatible' content='IE=edge,chrome=1'>";
    s += "<meta name=”apple-mobile-web-app-capable” content=”yes”>";
    s += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
    s += "<meta name='viewport' content='width=device-width, initial-scale=1.0, minimum-scale=1.0'>";
    s += "<title>ESP8266 AP Setup</title>";
    s += "</head>";
    s += "<body>";
    s += "<div style = 'padding:30px;'>";
    s += "<div style = 'text-align:center; padding:30px; color: #252e31;'><b>ESP8266 WiFi Setup</b></div>";
    s += "<form method='get' action='a' style = 'max-width:460px; margin-left:auto; margin-right:auto; background-color:#F7F9FA; border-radius:5px; padding:60px 20px;'>";
    s += "<div class='content' style = 'margin:0 10%; padding: 0 0 10px;'>";
    s += "<form method='get' action='a'>";
    s += "<input name='ssid' autocomplete='off' placeholder='SSID' style = 'text-align:center; height: 40px; width: 100%; margin-bottom:10px; border: 1px solid #9da3a6; background: #fff; border-radius: 5px;'>";
    s += "<input name='pass' autocomplete='off' placeholder='Password' style = 'text-align:center; height: 40px; width: 100%; margin-bottom:10px; border: 1px solid #9da3a6; background: #fff; border-radius: 5px;'>";
    s += "<input type='submit' style = 'width: 100%; height: 44px; padding:0 10px; border:0; background-color: #394950; border-radius:5px; color:#fff; margin-top:10px;'>";
    s += "</div>";
    s += "</form>";
    s += "</div>";
    s += "<div style = 'bottom: 0; height: 30px; text-align:center; padding:30px; font-size: small;'>ID KAIST <br> CIDR Lab</div>";
    s += "</body>";
    s += "</html>\r\n\r\n";
  }
  else if ( req.startsWith("/a?ssid=") ) {
    Serial.println("clearing eeprom");
    
    for (int i = 0; i < 96; ++i){ 
      EEPROM.write(i, 0);
    }
    
    String qsid;
    qsid = req.substring(8,req.indexOf('&'));
    Serial.println(qsid);
    Serial.println("");
    
    String qpass;
    qpass = req.substring(req.lastIndexOf('=')+1);
    Serial.println(qpass);
    Serial.println("");

    Serial.println("writing eeprom ssid:");
    for (int i = 0; i < qsid.length(); ++i)
    {
      EEPROM.write(i, qsid[i]);
      Serial.print(qsid[i]);
    }
    Serial.println();

    Serial.println("writing eeprom pass:");
    for (int i = 0; i < qpass.length(); ++i)
    {
      EEPROM.write(32+i, qpass[i]);
      Serial.print(qpass[i]);
    }
    Serial.println();

    EEPROM.commit();
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
    s += "<html>";
    s += "<head>";
    s += "<meta http-equiv='X-UA-Compatible' content='IE=edge,chrome=1'>";
    s += "<meta name=”apple-mobile-web-app-capable” content=”yes”>";
    s += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
    s += "<meta name='viewport' content='width=device-width, initial-scale=1.0, minimum-scale=1.0'>";
    s += "<title>ESP8266 AP Setup</title>";
    s += "</head>";
    s += "<body>";
    s += "<div style = 'padding:30px;'>";
    s += "<div style = 'text-align:center; padding:30px; color: #252e31;'><b>ESP8266 WiFi Setup</b></div>";
    s += "<form method='get' action='a' style = 'max-width:460px; margin-left:auto; margin-right:auto; background-color:#F7F9FA; border-radius:5px; padding:60px 20px;'>";
    s += "<div class='content' style = 'margin: 0 15%; padding: 0 0 10px; text-align:center'>";
    s += "Information successfully <br> saved to EEPROM <br><br> ... reset to boot into <br> new WiFi";
    s += "</div>";
    s += "<div style = 'bottom: 0; height: 30px; text-align:center; padding:30px; font-size: small;'>ID KAIST <br> CIDR Lab</div>";
    s += "</body>";
    s += "</html>\r\n\r\n";
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
  }

  client.print(s);
  return(1);
}
