#include <WiFi.h>
#include <string.h>

#define DEBUG

#define RXD2 16
#define TXD2 17

#define MAX_CLIENTS 5
#define buffSize 256
#define numBytes 6

long COM2_BAUD = 19200;

char receiveBuffer[buffSize];
uint8_t receiveBufferSize = 0;

WiFiServer wifiServerC(8888, MAX_CLIENTS); // TCP Port for Config

WiFiClient wifiClientC[MAX_CLIENTS];
bool wifiClientCConnected[MAX_CLIENTS];

const char *letters = "1234567890";

IPAddress IP = {192, 168, 1, 1};
IPAddress NMask = {255, 255, 255, 0};

unsigned long oldTime;
int delayTime = 100;
int lastTask = 0;

bool ConnectionEstablished; // Flag for successfully handled connection

void setup()
{

    Serial.begin(115200);

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP);

    WiFi.softAP("Bytronic_Connect", NULL, 1, 0, 10);

    delay(1000);
    WiFi.softAPConfig(IP, IP, NMask);

    Serial2.begin(COM2_BAUD, SERIAL_8N1, RXD2, TXD2);

#ifdef DEBUG

    Serial2.println("Serial2 Txd is on pin: " + String(TXD2));
    Serial2.println("Serial2 Rxd is on pin: " + String(RXD2));

    IPAddress IP = WiFi.softAPIP();
    Serial.println(IP);

    wifiServerC.begin(); // Start the TCP Server for Configuring the Module
    wifiServerC.setNoDelay(true);
#endif // DEBUG
}
void loop()
{
    if (oldTime + delayTime <= millis())
    {
        oldTime = millis();
        if (lastTask > 2)
            lastTask = 0;

        if (lastTask == 0)
        {
            if (Serial2.available())
            {
                receiveBufferSize = 0;
                while (Serial2.available() > 0)
                {
                    receiveBuffer[receiveBufferSize] = char(Serial2.read());
                    if (receiveBufferSize < buffSize - 1)
                        receiveBufferSize++;
                }
                Serial.write(receiveBuffer, receiveBufferSize);
            }
        }
        else if (lastTask == 1)
        {
            handleClient();
        }
        else if (lastTask == 2)
        {
            handleConnection();
        }
        lastTask++;
    }
}
void handleConnection()
{
    for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
        if (!wifiClientC[clientNum].connected())
        {
            if (wifiClientCConnected[clientNum])
            {
                wifiClientCConnected[clientNum] = false;
                Serial.println("Client disconnected ... terminate session ");
                wifiClientC[clientNum].stop();
            }
        }
    }
    if (wifiServerC.hasClient())
    {
        Serial.println("Server hasClient!");
        ConnectionEstablished = false; // Set to false
        for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
        {
            if (!wifiClientC[clientNum])
            {
                wifiClientC[clientNum] = wifiServerC.available();
                wifiClientCConnected[clientNum] = true;
                Serial.print("New client connected to session ");
                Serial.println(clientNum + 1);

                wifiClientC[clientNum].flush(); // clear input buffer, else you get strange characters
                wifiClientC[clientNum].println("Welcome!");
                ConnectionEstablished = true;
                break;
            }
        }
        if (ConnectionEstablished == false)
        {
            Serial.println("No free sessions ... drop connection");
            wifiServerC.available().stop();
        }
    }
}
void handleClient()
{
    for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
        if (wifiClientC[clientNum] && wifiClientC[clientNum].connected())
        {

            if (wifiClientC[clientNum].available() > 0)
            {
#ifdef DEBUG
                Serial.println("Config Data Available!");
#endif // DEBUG
                receiveBufferSize = 0;
                while (wifiClientC[clientNum].available() > 0)
                {
                    receiveBuffer[receiveBufferSize] = char(wifiClientC[clientNum].read());
                    if (receiveBufferSize < buffSize - 1)
                        receiveBufferSize++;
                }
                receiveBuffer[receiveBufferSize] = 0;
                Serial.println(receiveBuffer);
                if (receiveBuffer[0] == '!')
                {
#ifdef DEBUG
                    Serial.println("Config Data Valid!");
#endif // DEBUG
                    char *command = strtok(receiveBuffer, "?");
                    Serial.println(command);
                    if ((strcmp(command, "!GETCFG\r\n") == 0) || (strcmp(command, "!GETCFG") == 0))
                    {
                        Serial.println("GET CONFIG");
                        String configData = "!CFGCOM?" + (String) "test"; // comConf + "!CFGSID?" + ssidConf + "!CFGCOM1?" + com1Conf + "!CFGCOM2?" + com2Conf + "!CFGCOM3?" + com3Conf;
                        sendWifi(configData);
                        Serial.print(configData);
                    }
                    else if (strcmp(command, "!SETSID") == 0)
                    {
                        Serial.println("SET SSID CONFIG");
                        char *command2 = strtok(0, "?");
                        Serial.println(command2);
                        Serial.println(strlen(command2));
                        if (strlen(command2) >= 4)
                        {
                            // writeString(SSIDCONFIG, command2);
                            sendWifi("!OK");
                        }
                        else
                        {
                            sendWifi("!ERR?SSIDLEN");
                        }
                    }
                    else if (strcmp(command, "!SETCOM") == 0)
                    {
                        Serial.println("SET COM CONFIG");
                        char *command2 = strtok(0, "?");
                        Serial.println(command2);
                        sendWifi("!OK");
                    }
                    else if (strcmp(command, "!SETCOM1") == 0)
                    {
                        Serial.println("SET COM CONFIG");
                        char *command2 = strtok(0, "?");
                        Serial.println(command2);
                        int commad2Int = String(command2).toInt();
                        sendWifi("!OK");
                    }
                    else if (strcmp(command, "!SETCOM2") == 0)
                    {
                        Serial.println("SET COM CONFIG");
                        char *command2 = strtok(0, "?");
                        Serial.println(command2);
                        int commad2Int = String(command2).toInt();
                        sendWifi("!OK");
                    }
                    else if (strcmp(command, "!SETCOM3") == 0)
                    {
                        Serial.println("SET COM CONFIG");
                        char *command2 = strtok(0, "?");
                        Serial.println(command2);
                        int commad2Int = String(command2).toInt();
                        sendWifi("!OK");
                    }
                    else if ((strcmp(command, "!REBOOT\r\n") == 0) || (strcmp(command, "!REBOOT") == 0))
                    {
                        Serial.println("Esp will Reboot");
                        ESP.restart();
                    }
                    else
                    {
                        Serial.println("Wrong Command");
                        sendWifi("!ERR?COMMAND");
                    }
                }
            }
        }
    }
}
void sendWifi(char *c, uint8_t l)
{
    for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
        if (wifiClientC[clientNum].connected())
        {
            wifiClientC[clientNum].write(c, l);
        }
    }
}
void sendWifi(char *c)
{
    for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
        if (wifiClientC[clientNum].connected())
        {
            wifiClientC[clientNum].print(c);
        }
    }
}
void sendWifi(String c)
{
    for (int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
        if (wifiClientC[clientNum].connected())
        {
            wifiClientC[clientNum].print(c);
        }
    }
}
char *generateRandom()
{
    int i = 0;
    char *randString = "";
    for (i = 0; i < numBytes; i++)
    {
        randString = appendCharToCharArray(randString, letters[random(0, 9)]);
    }
    return randString;
}
char *appendCharToCharArray(char *array, char a)
{
    size_t len = strlen(array);

    char *ret = new char[len + 2];

    strcpy(ret, array);
    ret[len] = a;
    ret[len + 1] = '\0';

    return ret;
}