#include <arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define CURRENT_VERSION 0.1

// The pins that the relays are connected to.
#define RELAY_ONE_PIN 4
#define RELAY_TWO_PIN 7
#define RELAY_THREE_PIN 8
#define RELAY_FOUR_PIN 12
#define RELAY_FIVE_PIN 14
#define RELAY_SIX_PIN 15
#define RELAY_SEVEN_PIN 16
#define RELAY_EIGHT_PIN 17

// The pins that the fans are connected to. will be driven using PWM.
#define FAN_ONE 3
#define FAN_TWO 5
#define FAN_THREE 6
#define FAN_FOUR 9
#define FAN_FIVE 10
#define FAN_SIX 11

// pins used to connect to the server
#define CLIENT_DEVICE_RX 2
#define CLIENT_DEVICE_RX 3

// VARIABLES
byte relay[8] = { // the relay device
    RELAY_ONE_PIN,
    RELAY_TWO_PIN,
    RELAY_THREE_PIN,
    RELAY_FOUR_PIN,
    RELAY_FIVE_PIN,
    RELAY_SIX_PIN,
    RELAY_SEVEN_PIN,
    RELAY_EIGHT_PIN};
byte relayState[8]; // the state of the relay as defined by the server
byte fan[6] = {     // the fan device
    FAN_ONE,
    FAN_TWO,
    FAN_THREE,
    FAN_FOUR,
    FAN_FIVE,
    FAN_SIX};
byte fanSpeed[6]; // the speed of the fan as defined by the server

// ************************************************** JSON SETUP **************************************************
// Inside the brackets, 200 is the RAM allocated to this document.
// Use https://arduinojson.org/v6/assistant to compute the capacity.
// buffer to store incoming data from the client
#define JSON_LENGTH 30
char incomingData[JSON_LENGTH];
struct ServerData
{
    byte device;
    byte value;
};

// ************************************************** OBJECTS **************************************************
StaticJsonDocument<JSON_LENGTH> doc; // Allocate the JSON document
ServerData Data;
SoftwareSerial Server(18, 19);

void setup()
{
    Relay_Initialize();
    Fan_Initialize();
    Server_Initialize();
}

void loop()
{
    Server_ParseFrom();

    // RELAYS
    for (size_t i = 0; i < 9; i++)
    {
        Relay_SetHardwareState(Data.device, Data.value); // set the hardware state according to the software state.
    }

    // FANS
    for (size_t i = 0; i < 7; i++)
    {
        Fan_SetHardwareState(Data.device, Data.value); // set the hardware state according to the software state.
    }
}

// RELAY FUNCTIONS
void Relay_Initialize()
{
    // set the relay pins as output
    for (size_t i = 0; i < 9; i++)
    {
        pinMode(relay[i], OUTPUT);
    }

    // set the relay pins to off state on startup
    for (size_t i = 0; i < 9; i++)
    {
        digitalWrite(relayState[i], 0);                  // set the software state
        Relay_SetHardwareState(relay[i], relayState[i]); // set the hardware state according to the software state.
    }
}

void Relay_SetHardwareState(byte _device, byte _relayState)
{
    switch (_relayState)
    {
    case 0:
        digitalWrite(_device, LOW);
        break;
    case 1:
        digitalWrite(_device, HIGH);
        break;
    default:
        break;
    }
}

// FAN FUNCTIONS
void Fan_Initialize()
{
    // set the fan pins as output
    for (size_t i = 0; i < 9; i++)
    {
        pinMode(fan[i], OUTPUT);
    }

    // set the fan pins to off state on startup
    for (size_t i = 0; i < 9; i++)
    {
        digitalWrite(fanSpeed[i], 0);              // set the software state
        Fan_SetHardwareState(fan[i], fanSpeed[i]); // set the hardware state according to the software state.
    }
}

void Fan_SetHardwareState(byte _device, byte _fanSpeed)
{
    analogWrite(_device, _fanSpeed);
}

void Server_Initialize()
{
    // connect to server device via UART
    Server.begin(115200);
    Server.flush();
}
// JSON FUNCTIONS
void Server_ParseFrom()
{
    // get data
    if (Server.available() > 0)
    {
        // store the incoming data
        Server.readBytesUntil('\n', incomingData, JSON_LENGTH);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, incomingData);

        // Test if parsing was successful.
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        // Fetch values.
        // Most of the time, you can rely on the implicit casts.
        // In other case, you can do doc["time"].as<long>();
        Data = {doc["device"].as<byte>(), doc["value"].as<byte>()};

    } // end getting data
}
