// This #include statement was automatically added by the Particle IDE.
//

#include "SparkJson.h"
#include "MQTT.h"

//Declare pins

uint16_t tc1 = A0;                          //Thermocouple 1
uint16_t tc2 = A1;                          //Thermocouple 2
uint16_t tc3 = A2;                          //Thermocouple 3
uint16_t tc4 = A3;                          //Thermocouple 4

char mqttBroker[32] = "ctfhosts.ddns.net";  //MQTT Broker URL / IP
String mqttPub = "bbq/temperature";         //MQTT Publication Channel
String mqttSub = "bbq/commands";            //MQTT Subscription Channel
String mqttLog = "log/";                    //MQTT logging channel
String value = "0";                         //Initialize reporting value
int wait = 0;                               //Time between loops
String myID;                                //Variable for the Photon device ID
String strMqtt;                             //Variable to contain the MQTT string
int str_len;                                //Variable for String Length
int counter = 64;                           //Variable to count time for reporting
int reportDelay = 1000;                     //Time Between reports
int minFan = 5000;                          //Minimum fan run time
int fanCounter = 0;                         //variable for keeping track of fan run time
int fanState = 0;
int temp = 250;
int fanControl = 1;                         //Variable to enable fan
int fanRunTime = 0;                         //How long has the fan been running
int fanStartTime = 0;                       //millis() when fan started

int tcRead1;                                //Thermocouple reading
int tcRead2;                                //Thermocouple reading
int tcRead3;                                //Thermocouple reading
int tcRead4;                                //Thermocouple reading

int m;                                      //Moisture reading
int a = 0;                                  //Averaged reading
int c;                                      //Count of reads

int rssi;                                   //RSSI strength variable

int led = D7;                               //Which LED to blink

MQTT client(mqttBroker, 1883, callback);    //Initialized MQTT broker

void setup()
        {

        // register the cloud function
        Particle.function("changeTemp", changeTemp);
        Particle.variable("temp", temp);
        Particle.function("fanEnable", fanEnable);

        Serial.begin(9600);

        //Get the deviceID
        myID = System.deviceID();
        strMqtt = mqttLog;
        str_len = myID.length() + 1;
        char char_myID[str_len];
        myID.toCharArray(char_myID, str_len);

        //Create json status object
        StaticJsonBuffer<200> jsonBuffer;
        char buffer [200];

        JsonObject& root = jsonBuffer.createObject();
        root["deviceID"] = char_myID;
        root["status"] = "Connected at startup";

        //Publish JsonObject
        root.printTo(Serial);
        root.printTo(buffer, sizeof(buffer));

        //Set pin modes
        pinMode(led, OUTPUT);
        pinMode(0, OUTPUT);

        // connect to the MQTT broker
        client.connect("connect");

        //Publish our status json to the broker
        client.publish(strMqtt, buffer);

        //Subscribe to the broker to recieve messages
        client.subscribe(mqttSub);
    }


void loop()
    {
        blink(1);
        readSensor(64, tc1);
        tcRead1 = m;

        readSensor(64, tc2);
        tcRead2 = m;

        readSensor(64, tc3);
        tcRead3 = m;

        readSensor(64, tc4);
        tcRead4 = m;

        if (tcRead1 < temp)
            {
                Serial.println("Enable Fan");
                if (fanControl == 1)
                    {
                        if (millis() > fanCounter)
                            {
                                Serial.print("fanControl = ");
                                Serial.println(fanControl);
                                digitalWrite(0, LOW);
                                fanState = 1;
                                fanCounter = millis() + minFan;
                            }

                        if (fanStartTime > 0)
                            {
                                fanRunTime = ((fanStartTime - millis()) * -1) / 1000;
                            }
                        else
                            {
                                fanStartTime = millis();
                            }
                    }
            }
        else
            {
                digitalWrite(0, HIGH);
                fanState = 0;
                fanStartTime = 0;
                fanCounter = 0;
            }

        if (fanControl == 0)
            {
                Serial.print("fanControl = ");
                Serial.println(fanControl);
                digitalWrite(0, HIGH);
                fanState = 0;
            }

        //Report Timer
        if (millis()>counter)
            {
                //Publish to the MQTT Broker
                report(tcRead1, tcRead2, tcRead3, tcRead4, fanRunTime, fanState, mqttPub);

                counter = counter + reportDelay;
            }

        client.loop();
        delay(wait);

    }

// Adjust the requested temperature
int changeTemp(String command)
    {
        Serial.println("**************");
        Serial.println("changeTemp()");
        Serial.println("**************");
        temp = command.toInt();

        return 1;
    }

// Enable the fire box Fan
int fanEnable(String command)
    {
        Serial.println("**************");
        Serial.println("fanEnable()");
        Serial.println("**************");
        fanControl = command.toInt();

        return 1;
    }

//Function handles reporting to the MQTT broker
void report(int tcRead1, int tcRead2, int tcRead3, int tcRead4, int fanRunTime, int fanState, String feed)
    {
        String stringTcRead1;
        stringTcRead1 = String(tcRead1);
        int str_len;
        str_len = stringTcRead1.length() + 1;
        char char_tcRead1[str_len];
        stringTcRead1.toCharArray(char_tcRead1, str_len);

        String stringTcRead2;
        stringTcRead2 = String(tcRead2);
        str_len = stringTcRead2.length() + 1;
        char char_tcRead2[str_len];
        stringTcRead2.toCharArray(char_tcRead2, str_len);

        String stringTcRead3;
        stringTcRead3 = String(tcRead3);
        str_len = stringTcRead3.length() + 1;
        char char_tcRead3[str_len];
        stringTcRead3.toCharArray(char_tcRead3, str_len);

        String stringTcRead4;
        stringTcRead4 = String(tcRead4);
        str_len = stringTcRead4.length() + 1;
        char char_tcRead4[str_len];
        stringTcRead4.toCharArray(char_tcRead4, str_len);

        rssi = WiFi.RSSI();
        String stringRssi;
        stringRssi = String(rssi);
        str_len = stringRssi.length() + 1;
        char char_rssi[str_len];
        stringRssi.toCharArray(char_rssi, str_len);

        String stringFanState;
        stringFanState = String(fanState);
        str_len = stringFanState.length() + 1;
        char char_FanState[str_len];
        stringFanState.toCharArray(char_FanState, str_len);

        String stringFanRunTime;
        stringFanRunTime = String(fanRunTime);
        str_len = stringFanRunTime.length() + 1;
        char char_FanRunTime[str_len];
        stringFanRunTime.toCharArray(char_FanRunTime, str_len);

        str_len = myID.length() + 1;
        char char_myID[str_len];
        myID.toCharArray(char_myID, str_len);

        //Build json REPORT object
        StaticJsonBuffer<300> jsonBuffer;
        char bufferReport [300];

        JsonObject& root = jsonBuffer.createObject();
        root["deviceID"] = char_myID;
        root["tcRead1"] = char_tcRead1;
        root["tcRead2"] = char_tcRead2;
        root["tcRead3"] = char_tcRead3;
        root["tcRead4"] = char_tcRead4;
        root["fanRunTime"] = char_FanRunTime;
        root["rssi"] = char_rssi;
        root["fanState"] = char_FanState;


        //root.printTo(Serial);
        root.printTo(bufferReport, sizeof(bufferReport));
        client.connect("connect");

        if (client.isConnected())
            {
                client.publish(feed,bufferReport);
                blink(3);

            }
        }

// Allows us to recieve a message from the subscription
void callback(char* topic, byte* payload, unsigned int length)
    {
        char p[length + 1];
        memcpy(p, payload, length);
        p[length] = NULL;
        String message(p);

    //This is where you put code to handle any message recieved from the broker

    }

void blink(int blinks)
    {

        int x = 0;

        do
        {
          digitalWrite(led, HIGH);
          delay(100);
          digitalWrite(led, LOW);
          delay(100);
          x = x + 1;

        } while (x < blinks);
    }

int readSensor(int c, uint16_t p)
    {
        float v;
        float t;
        int x = 0;
        int a = 0;

        while (x < c)
            {
                a = (analogRead(p)) + a;

                x = x + 1;
            }

            v = a / c;

            v =  (v*0.0008);
            t = ((v - 1.25) / 0.005);
            t = (t * 1.8) + 32;

            m = t;

        return m;
    }
