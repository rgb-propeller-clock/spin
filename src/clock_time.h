#ifndef CLOCK_TIME_H
#define CLOCK_TIME_H
#include <Arduino.h>
#include <WiFi101.h>

WiFiClient client;

char buffer[5000];
char startTimeBuf[20];
char timeBuf[40];
int timeSinceStart;

char ssid[] = "RLAB"; // network SSID (name)
char pass[] = "metropolis"; // for networks that require a password
int status = WL_IDLE_STATUS; // the WiFi radio's status


bool connect_to_webpage()
{
    Serial.println("Attempting to connect to webpage");
    if (client.connect("worldtimeapi.org", 80)) {
        Serial.println("Connected to server");
        client.println("GET /api/timezone/America/New_York HTTP/1.1");
        client.println("Host: worldtimeapi.org");
        client.println("Connection: close");
        client.println();
        return true;
    } else {
        Serial.println("Failed to fetch webpage");
        return false;
    }
}


void setup_wifi()
{
    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass); // WiFi.begin(ssid, pass) for password
        delay(10000);
    }
    Serial.println("Connected!");
    bool connected = false;
    while (!connected) {
        if (connect_to_webpage()) {
            Serial.println("fetched webpage");
            connected = true;
        } else {
            Serial.println("failed to fetch webpage");
        }
        Serial.println();
    }
}

bool read_webpage()
{
    int len = client.available();
    if (len == 0) {
        return false;
    }
    memset(buffer, 0x0, sizeof(buffer));
    int index = 0;
    while (client.available()) {
        char c = client.read();
        buffer[index] = c;
        index++;
    }
    timeSinceStart = millis();
    Serial.println("Content received");
    if (index > 0) {
        Serial.println();
        noInterrupts();
        char* response = strstr(buffer, "datetime");
        if (response) {
            memset(startTimeBuf, 0x0, sizeof(startTimeBuf));
            int i = 22;
            int k = 0;
            while (i <= 29) {
                startTimeBuf[k] = response[i];
                i++;
                k++;
            }
        }
        interrupts();
        delay(500);
    }
    return true;
}

int stringToTimeInt(char* t)
{
    char hours[2];
    char mins[2];
    char secs[2];
    hours[0] = t[0];
    hours[1] = t[1];
    mins[0] = t[3];
    mins[1] = t[4];
    secs[0] = t[6];
    secs[1] = t[7];
    int hoursInt = atoi(hours);
    int minsInt = atoi(mins);
    int secsInt = atoi(secs);
    return (hoursInt * 60 * 60) + (minsInt * 60) + secsInt;
}

char* getStartTime()
{
    setup_wifi();
    while (!read_webpage())
        ;
    return startTimeBuf;
}

char* getCurrentTime()
{
    // TODO: Consider midnight case
    int curMillis = millis();
    int millisInDay = 24 * 60 * 60 * 1000;
    if (timeSinceStart > (millisInDay)) {
        timeSinceStart = timeSinceStart % millisInDay;
    }
    int curTime = stringToTimeInt(startTimeBuf) + ((curMillis)-timeSinceStart) / 1000;
    int hours = curTime / 60 / 60;
    int mins = (curTime / 60) - (hours * 60);
    int secs = curTime - (hours * 60 * 60) - (mins * 60);
    sprintf(timeBuf, "%02d:%02d:%02d", hours, mins, secs);
    return timeBuf;
}

// void setup() {
//   Serial.begin(9600);
//   while(!Serial);
//   Serial.println(getStartTime());
// }
// void loop() {
//   Serial.println(getCurrentTime());
//   delay(1000);
// }
#endif