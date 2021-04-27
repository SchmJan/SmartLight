#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266SSDP.h>
#include "LightMode.h"
#include <Scheduler.h>
#include <espnow.h>
#include "ArduinoJson.h"

#define LED 2
#define NUMBER 10

void clearEeprom();

void handleDispatcher();

void handleDispatcherAp();

void handleBrightness();

void handleColor();

void handleEffect();

void handleSpeed();

void handleDataRequest();

void setUrlsStaMode();

void setUrlsApMode();

void Get_Req();

void setSSDP();

void handleDataUpdate();

void searchNetworks();

String getPasswordFromEEprom();

String getNetworkFromEEprom();

void setPasswordForEEprom(String sssid, String passs);

void OnDataSent();


String Essid = "";                                          //EEPROM Network SSID
String Epass = "";                                          //EEPROM Network Password
String sssid = "";                                          //Read SSID From Web Page
String passs = "";                                          //Read Password From Web Page

ESP8266WebServer server(80);                           //Specify port

long timestamp;
LightMode ledTask;

const char website[]
        PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Page Title</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
<h1>Control for the Light</h1>
<p>Choose a color <input id="color" type="color" value="#ffff00"/></p>
<p> Brightness:
    <input id="bright" type="range" min=20 max=250 value=127 oninput="sendData(-1);">
<p> Speed:
    <input id="speed" type="range" min=20 max=200 value=50 oninput="sendData(-1);">
<hr/>
<h2>Choose an effect</h2>

    <p>
        <button name="action" type="submit" value="1">Normal</button>
    </p>
    <p>
        <button name="action" type="submit" value="2">Rainbow</button>
    </p>
    <p>
        <button name="action" type="submit" value="3">Rainbow Wheel</button>
    </p>
    <p>
        <button name="action" type="submit" value="4">Theater Chase Rainbow</button>
    </p>
    <p>
        <button name="action" type="submit" value="5">Theater Chase Rainbow 2</button>
    </p>
    <p>
        <button name="action" type="submit" value="6">Theather Chase</button>
    </p>
    <p>
        <button name="action" type="submit" value="7">Color Wipe</button>
    </p>
    <p>
        <button name="action" type="submit" value="8">Fire</button>
    </p>
    <p>
        <button name="action" type="submit" value="9">Running Lights</button>
    </p>
    <p>
        <button name="action" type="submit" value="10">Sparkle</button>
    </p>
    <p>
        <button name="action" type="submit" value="11">Sparkle Random</button>
    </p>
    <p>
        <button name="action" type="submit" value="12">Twinkle</button>
    </p>
    <p>
        <button name="action" type="submit" value="13">Twinkle Random</button>
    </p>
    <p>
        <button name="action" type="submit" value="14">Cylon Bounce</button>
    </p>
    <p>
        <button name="action" type="submit" value="15">Blinking</button>
    </p>


<script>
    function sendData( mode) {
        console.log('called');
        let xhr = new XMLHttpRequest();
        xhr.open('POST', '/update');

        // prepare form data
        let obj = { mode: mode.toString(),
            delay: document.getElementById("speed").value,
            brightness :document.getElementById("bright").value,
            color : document.getElementById("color").value};

        let data =JSON.stringify(obj);
        console.log(data);
        // set headers
        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');

        // send request
        xhr.send(data);
    }

    var buttons = document.querySelectorAll('button');
    for (var i = 0; i < buttons.length; i++) {
        var self = buttons[i];

        self.addEventListener('click', function (event) {
            // prevent browser's default action
            event.preventDefault();

            // call your awesome function here
            sendData(this.value); // 'this' refers to the current button on for loop
        }, false);
    }



    function sendBrightness(slider, value) {
        console.log(slider + ": " + value);
        var xhr = new XMLHttpRequest();
        var url = "brightness?bright=" + value;
        xhr.open("GET", url, true);
        xhr.send();
    }

    function sendColor(value) {
        console.log("Color : " + value);
        var xhr = new XMLHttpRequest();
        var url = "color?col=" + value.substr(1);
        xhr.open("GET", url, true);
        xhr.send();
    }

    function sendSpeed(value) {
        console.log("Speed : " + value);
        var xhr = new XMLHttpRequest();
        var url = "speed?delay=" + value;
        xhr.open("GET", url, true);
        xhr.send();
    }

    function sendModus(value) {
        var json = {
            mode: '',
            delay: '',
            color: '',
            mode: '$value'
        };
        console.log("Modus : " + value);
        xhr = new XMLHttpRequest();
        xhr.open('POST', '/update');

        // prepare form data
        let data = new FormData(form);

        // set headers
        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');

        // send request
        xhr.send(data);

    }


</script>
</body>
</html>
)rawliteral";

class clientUpdate : public Task {
protected:
    void loop() {
        server.handleClient();

        if (digitalRead(0) == LOW) {
            clearEeprom();
            delay(100);
            ESP.restart();
        }

        yield();
    }

} clientUpdate_task;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    char macStr[18];
    Serial.print("Packet to:");
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print(macStr);
    Serial.print(" send status: ");
    if (sendStatus == 0) {
        Serial.println("Delivery success");
    } else {
        Serial.println("Delivery fail");
    }
}


void setup() {
    delay(200);                            //Stable Wifi
    Serial.begin(115200);             //Set Baud Rate
    delay(100);
    Serial.println("Started");
    EEPROM.begin(512);

    pinMode(0, INPUT);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    String password = getPasswordFromEEprom();
    String network = getNetworkFromEEprom();

    Serial.println(network);
    Serial.println(password);

    if (network.charAt(0) != '#') {

        WiFi.mode(WIFI_STA);
        Serial.println("STA-Mode");

        WiFi.begin(network, password);

        bool lock = false;
        long time = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (digitalRead(0) == LOW) {
                clearEeprom();
                delay(100);
                ESP.restart();
            }

            digitalWrite(LED, HIGH);
            delay(200);
            digitalWrite(LED, LOW);
            delay(200);
            Serial.print(".");

        }

        digitalWrite(LED, HIGH);

        setUrlsStaMode();

        setSSDP();

        Serial.println(WiFi.localIP());

        ledTask.begin();

        Scheduler.start(&clientUpdate_task);
        Scheduler.start(&ledTask);
        Scheduler.begin();
    } else {
        WiFi.mode(WIFI_AP_STA);
        Serial.println("Wifi mode: AP");

        WiFi.softAP("Smart Light AP", "");  //Access Point Mode SSID and Password-Provided hard coded in the code
        delay(10);
        IPAddress myIP = WiFi.softAPIP(); //Get IP address
        Serial.print("HotSpt IP:");
        Serial.println(myIP);

        setUrlsApMode();
        timestamp = millis();
    }
}

void loop() {
    server.handleClient();

    if (millis() - timestamp > 1500) {
        timestamp = millis();
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
    }


    if (digitalRead(0) == LOW) {
        clearEeprom();
        delay(100);
        ESP.restart();
    }
}

void setUrlsStaMode() {
    server.on("/", handleDispatcher);
    server.on("/update", HTTP_POST, handleDataUpdate);
    server.on("/getData", HTTP_GET, handleDataRequest);
    Serial.println("HTTP server started");
    Serial.println("TCP server started");

    server.on("/description.xml", HTTP_GET, []() {
        SSDP.schema(server.client());
    });
    server.begin();
}

void setUrlsApMode() {
    server.on("/", handleDispatcherAp);
    server.on("/a", Get_Req);
    server.begin();
}

String getPasswordFromEEprom() {
    String Epass = "";
    for (int i = 32; i < 96; ++i) {
        Epass += char(EEPROM.read(i));
    }

    return Epass;
}

String getNetworkFromEEprom() {
    String Essid = "";
    for (int i = 0; i < 32; ++i) {
        Essid += char(EEPROM.read(i));
    }

    return Essid;
}

void setPasswordForEEprom(String sssid, String passs) {
    clearEeprom();
    for (int i = 0; i < 32; ++i) {
        if (i < sssid.length())
            EEPROM.write(i, sssid[i]);
        else
            EEPROM.write(i, 0);
    }

    for (int i = 32; i < 96; ++i) {
        if (i - 32 < passs.length())
            EEPROM.write(i, passs[i - 32]);
        else
            EEPROM.write(i, 0);
    }
    if (!EEPROM.commit()) Serial.println("Error set new password");
    else Serial.println("new password set");
}

void searchNetworks() {
    int networksAvailable = 0, i = 0, len = 0;
    String st = "", s = "";                    //String array to store the SSID's of available networks
    networksAvailable = WiFi.scanNetworks();       //Scan for total networks available
    st = "<ul>";
    for (int i = 0; i < networksAvailable; ++i) {
        // Print SSID and RSSI for each network found
        st += "<li>";
        st += i + 1;
        st += ": ";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);
        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</li>";
    }
    st += "</ul>";
    IPAddress ip = WiFi.softAPIP();                  //Get ESP8266 IP Adress
    //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "\n\r\n<!DOCTYPE HTML>\r\n<html><h1> Available Networks</h1> ";
    //s += ipStr;
    s += "<p>";
    s += st;
    s += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><label>Password: </label><input name='pass' length=64><input type='submit'></form>";
    s += "</html>\r\n\r\n";

    server.send(200, "text/html", s);
}

void Get_Req() {

    if (server.hasArg("ssid") && server.hasArg("pass")) {
        sssid = server.arg("ssid");//Get SSID
        passs = server.arg("pass");//Get Password
    }


    if (sssid.length() > 1 && passs.length() > 1) {
        clearEeprom();
        delay(10);

        String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>Smart Light</h1> ";
        s += "<p>Password Saved... Reset to boot into new wifi. You can reconnect to your normal (Smart light connected) network</html>\r\n\r\n";
        server.send(200, "text/html", s);
        setPasswordForEEprom(sssid, passs);
        delay(100);
        ESP.restart();
    }

}

void clearEeprom() {
    Serial.println("Clearing Eeprom");
    for (int i = 0; i < 96; ++i) { EEPROM.write(i, '#'); }
    EEPROM.commit();
}

void handleDispatcherAp() {
    int Tnetwork = 0, i = 0, len = 0;
    String st = "", s = "";                    //String array to store the SSID's of available networks
    Tnetwork = WiFi.scanNetworks();       //Scan for total networks available
    st = "<ul>";
    for (int i = 0; i < Tnetwork; ++i) {
        // Print SSID and RSSI for each network found
        st += "<li>";
        st += i + 1;
        st += ": ";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);
        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</li>";
    }
    st += "</ul>";
    IPAddress ip = WiFi.softAPIP();                  //Get ESP8266 IP Adress
    //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "\n\r\n<!DOCTYPE HTML>\r\n<html><h1> Metro Store</h1> ";
    //s += ipStr;
    s += "<p>";
    s += st;
    s += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><label>Paswoord: </label><input name='pass' length=64><input type='submit'></form>";
    s += "</html>\r\n\r\n";

    server.send(200, "text/html", s);
}

void handleDispatcher() {
    server.send(200, "text/html", website); //Send web page
}

void handleDataUpdate() {

    if (!server.hasArg("plain")) { //Check if body received

        server.send(200, "text/plain", "Body not received");
        return;

    }

    String body = server.arg("plain");
    Serial.println("Received new data:\n" + body);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    JsonObject obj = doc.as<JsonObject>();

    long mode = obj["mode"];
    long brightness = obj["brightness"];
    long delay = obj["delay"];
    String data = obj["color"];

    if (mode != -1) {
        ledTask.setMode(mode);
    }

    ledTask.setBrightness(brightness);
    ledTask.setSpeed(delay);

    Serial.println("Color :" + data);
    long long number = strtoll(&data[1], NULL, 16);

    // Split them up into r, g, b values
    int red = (int) number >> 16;
    int green = (int) number >> 8 & 0xFF;
    int blue = (int) number & 0xFF;

    ledTask.setColor(red, green, blue);

    server.send(200, "text/plain", "");

}

void handleDataRequest() {
    String data = ledTask.toString();
    server.send(200, "text/plain", data);
}

void setSSDP() {

    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("Philips hue clone");
    SSDP.setSerialNumber("001788102201");
    SSDP.setURL("index.html");
    SSDP.setModelName("Philips hue bridge 2012");
    SSDP.setModelNumber("929000226503");
    SSDP.setModelURL("http://www.meethue.com");
    SSDP.setManufacturer("Royal Philips Electronics");
    SSDP.setManufacturerURL("http://www.philips.com");
    SSDP.begin();


    Serial.printf("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("Smart Light");
    SSDP.setSerialNumber("0000001");
    SSDP.setURL("/");
    SSDP.setModelName("Smart Light ESP8266");
    SSDP.setModelNumber("0001");
    SSDP.setModelURL(""); //TODO set github link
    SSDP.setManufacturer("Jan Schmidt");
    SSDP.setManufacturerURL("http://www.philips.com");
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.begin();


}
