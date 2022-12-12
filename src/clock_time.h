/**
 * This file contains functions for connecting to the internet over WiFi,
 * retrieving the current time from an api,
 * and creating a string representing the current time that updates as time passes
 * without further wifi connections being necessary.
 */
#ifndef CLOCK_TIME_H
#define CLOCK_TIME_H
#include <Arduino.h>
#include <WiFi101.h>

WiFiClient client;

char buffer[5000];
char startTimeBuf[20];
char timeBuf[40];
int timeSinceStart;

char ssid[] = "router"; // network SSID (name)
char pass[] = "password"; // for networks that require a password
int status = WL_IDLE_STATUS; // the WiFi radio's status

/**
 * @brief  attempts to connect to the worldtimeapi.org website
 * @retval true if connection successful, false if unsuccessful
 */
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

/**
 * @brief  connects to a wifi network, call on startup
 */
void setup_wifi()
{
    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
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

/**
 * @brief
 * @retval true if string containing time has been stored
 */
bool read_webpage()
{
    int len = client.available();
    if (len == 0) {
        return false;
    }
    memset(buffer, 0x0, sizeof(buffer));
    unsigned int index = 0;
    while (client.available()) {
        char c = client.read(); // read data from webpage
        buffer[index] = c;
        if (index < sizeof(buffer) - 1) { // prevent buffer overflow
            index++;
        }
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
            while (i <= 29) { // read the part of the response containing the time into a smaller buffer
                startTimeBuf[k] = response[i];
                i++;
                k++;
            }
        } else {
            return false; // "datetime" not found in response
        }
        interrupts();
        delay(500); // don't spam the API too fast if webpage_read takes more than one attempt
    } else {
        return false; // no data read; index==0
    }
    return true;
}

/**
 * @brief converts a string representation of time to an integer value of seconds
 * @param  t: char* string representing time, must be 8 characters, ex: 16:31:02
 * @retval integer value of seconds since midnight.
 */
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

/**
 * @brief connects to wifi and saves a string representing the time.
 * @note blocks until wifi connects and webpage returns a good response
 * @retval pointer to global variable that contains the time represented as a string
 */
void getStartTime()
{
    setup_wifi();
    while (!read_webpage())
        ;
}

/**
 * @brief This function uses millis() to calculate the current time without any more connections to a time API.
 * @retval pointer to global variable string containing the current time in 24 hour time
 */
char* getCurrentTime()
{
    int curMillis = millis();
    int millisInDay = 24 * 60 * 60 * 1000;
    if (timeSinceStart > (millisInDay)) {
        timeSinceStart = timeSinceStart % millisInDay;
    }
    int curTime = stringToTimeInt(startTimeBuf) + ((curMillis)-timeSinceStart) / 1000;
    int hours = curTime / 60 / 60;
    int mins = (curTime / 60) - (hours * 60);
    int secs = curTime - (hours * 60 * 60) - (mins * 60);
    sprintf(timeBuf, "%02d:%02d:%02d", hours % 24, mins, secs);
    return timeBuf;
}
#endif