// Arduino code to be used in conjunction with PureData patch.


#include <math.h>
#include <Arduino.h>


#define BAUD_RATE      115200      // Speed of the serial port
#define BUFFER_SIZE    256       // Size of the cyclic buffer
#define COMMAND_PREFIX '~'       // Prefix of a command -> Should be a char
#define COMMAND_SUFFIX '^'       // Suffix of a command -> Should be a char
#define ERR_NONE             0
#define ERR_PREFIX_NOT_FOUND 1
#define ID             1
#define DIGITAL_SEND_PREFIX  '<'
#define DIGITAL_SEND_SUFFIX  '>'
char pinArray[] = {'?','0','1','2','3','4','5','6','7','8','9'};
int pins[] = {2,3,4,5,6,7,9,10,11,12,13};
int pinStates[] = {0,0,0,0,0,0,0,0,0,0,0};
int pinOldState[] = {0,0,0,0,0,0,0,0,0,0,0};
long time = 0;         // the last time the output pin was toggled
long debounce = 20;
volatile char message[] = {'<','1','/','1','/','0','/','0','>'};
volatile char old_message[] = {'<','1','/','1','/','0','/','0','>'};
volatile int msgReady = 0;
int state = HIGH;
char delimiter[]       = "/";
char* valPosition;
char *buffer         = NULL;     // Pointer to the cyclic buffer
char *offsetChar     = NULL;     // Offset of the char to be filled in
char *offsetCommand  = NULL;     // Offset of the next command
char *currentCommand = NULL;     // Command line extracted from the byffer
int lastError      = ERR_NONE;   // Error code when command cannot be extracted
// Analog Read variables
const int numReadings = 3;

int readings[numReadings];      // the readings from the analog input                 // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int AnalogValues[] = {0,0,0,0};
int OldAnalogValues[] = {0,0,0,0};
int inputPin[] = {0,0,0,0};


// ===========================================================================
// =============================================================== Setup() ===
// ===========================================================================
void setup()
{
        // --- Allocate the buffer and initialize pointers -----------------------
        buffer = (char *)malloc(BUFFER_SIZE);
        offsetCommand = buffer;
        offsetChar = buffer;
        // --- Initialize the serial port ----------------------------------------
        pinMode(8, OUTPUT);
        digitalWrite(8, HIGH);
        for (int count=0; count<10; count++) {
                pinMode(pins[count],INPUT);
        }
        Serial.begin(BAUD_RATE);
        for (int thisReading = 0; thisReading < numReadings; thisReading++)
                readings[thisReading] = 0;
}


void send_packet(int pin, int value)
{
        int HighByte = 0;
        int LowByte = 0;
        Serial.print('(');
        Serial.print(ID);
        Serial.print('/');
        Serial.print(pin);
        Serial.print('/');
        HighByte = value / 256;
        Serial.print(HighByte);
        Serial.print('/');
        LowByte = value % 256;
        if (LowByte < 10) {
                Serial.print('0');
                Serial.print('0');
        }
        else if (LowByte >= 10 && LowByte < 100) {
                Serial.print('0');
        }
        Serial.print(LowByte);
        Serial.print(')');
}

void readAnalogPot(int count){

        for (int index=0; index<numReadings; index++) {
                // read from the sensor:
                readings[index] = analogRead(inputPin[count]);
                total= total + readings[index];
        }
        // calculate the average:
        AnalogValues[count] = total / numReadings;
        total=0;
}

void digitalPinRead()
{
        for (int count=0; count<1; count++) {
                pinStates[count] = digitalRead(pins[count]);
                if (pinStates[count] == HIGH && pinOldState[count] == LOW && millis() - time > debounce) {
                        if (state == HIGH) {
                                state = LOW;
                                message[5] = pinArray[count];
                                message[7] = '1';
                                for (int count = 0; count < 9; count++) {
                                        Serial.write(message[count]);
                                }
                        }
                        else {
                                state = HIGH;
                                message[5] = pinArray[count];
                                message[7] = '1';
                                for (int count = 0; count < 9; count++) {
                                        Serial.write(message[count]);
                                }
                        }
                        time = millis();
                }
                pinOldState[count] = pinStates[count];
        }
}



void pinChange(){
        for (int count = 0; count < 11; count++) {
                pinStates[count] = digitalRead(pins[count]);
                if (pinStates[count] != pinOldState[count]) {
                        if (pinStates[count] == 1) {
                                message[4] = pinArray[count];
                                if (message[6] == '0') message[6]='1';
                                else                   message[6]='0';
                        }
                        pinOldState[count] = pinStates[count];
                        msgReady = 1;
                }

        }

}
void digital_pr(int inPin, int state){
        pinMode(inPin, OUTPUT);
        digitalWrite(inPin, state);
}

String getValue(String data, char separator, int index)
{

        int maxIndex = data.length()-1;
        int j=0;
        String chunkVal = "";

        for(int i=0; i<=maxIndex && j<=index; i++)
        {
                chunkVal.concat(data[i]);

                if(data[i]==separator)
                {
                        j++;

                        if(j>index)
                        {
                                chunkVal.trim();
                                return chunkVal;
                        }

                        chunkVal = "";
                }
        }
}

boolean readSerial()
{
        boolean rc;
        char newChar;
        int commandSize;
        char *rawCommand;
        int commandSubSize;
        // --- Initialization ----------------------------------------------------
        rc = false;
        lastError = ERR_NONE;
        if(currentCommand != NULL)
        {
                free(currentCommand);
                currentCommand = NULL;
        }
        // --- Get incoming data -------------------------------------------------
        while(Serial.available())
        {
                // --- Read one char and put it in the buffer ------------------------
                newChar = Serial.read();
                *offsetChar = newChar;
                // --- Check if a command is potentially terminated ------------------
                if(newChar == COMMAND_SUFFIX)
                {
                        // --- Extract the raw command line ------------------------------
                        commandSize = (((offsetChar - buffer) - (offsetCommand - buffer)) + 1);
                        // Cyclic buffer: occurs when the current offset is returning to
                        // The beginning of the buffer (start of command > end of command)
                        if(commandSize < 0)
                                commandSize += BUFFER_SIZE;
                        // Do not forget to add room to the '\0' character
                        rawCommand = (char *)malloc(commandSize) + 1;
                        if(offsetChar > offsetCommand)
                                // Start offset < Finish offset: 1 single copy
                                strncpy(rawCommand, offsetCommand, commandSize);
                        else
                        {
                                // Start offset > Finish offset: command line is splitted
                                commandSubSize = BUFFER_SIZE - (offsetCommand - buffer);
                                strncpy(rawCommand, offsetCommand, commandSubSize);
                                strncpy(rawCommand + commandSubSize, buffer, commandSize - commandSubSize);
                        }
                        rawCommand[commandSize] = '\0';
                        // --- Verify the integrity of the command -----------------------
                        if(rawCommand[0] == COMMAND_PREFIX)
                        {
                                // --- Command OK: return it without prefix and suffix ---
                                commandSize = strlen(rawCommand) - 1;
                                currentCommand = (char *)malloc(commandSize);
                                strncpy(currentCommand, rawCommand + 1, commandSize);
                                currentCommand[commandSize - 1] = '\0';
                                rc = true;
                        }
                        else
                                lastError = ERR_PREFIX_NOT_FOUND;
                        // Whatever the command is correct or not, the suffix terminates
                        // it, so place the offset of the command to the starting position
                        offsetCommand = offsetChar;
                        // --- Process the incoming command before processing next inputs ---
                        return rc;
                }
                // --- No command terminated, process the next incoming character ----
                offsetChar++;
                if((offsetChar - buffer) >= BUFFER_SIZE)
                        offsetChar = buffer;
                // --- Let some time to serial port to get new data ------------------
                delay(1);
        }
        // --- Buffer is empty and no command being terminated ---
        return rc;
}


// ===========================================================================
// ================================================================ loop() ===
// ===========================================================================
void loop()
{
        // --- Read serial and try to get a command ------------------------------
        if(readSerial() == false)
        {
                // --- No command found: check if an error occured -------------------
                if(lastError != ERR_NONE)
                {
                        Serial.print("> Error while reading command: ");
                        Serial.println(lastError);
                }
        }
        else
        {
                // --- Process incoming command --------------------------------------
                //       Serial.print(currentCommand);
                String IDr = getValue(currentCommand, '/', 0);
                Serial.print(IDr);
                String Mode = getValue(currentCommand, '/', 1);
                Serial.print(Mode);
                String Pin = getValue(currentCommand, '/', 2);
                Serial.print(Pin);
                String Value = getValue(currentCommand, '/', 3);
                Serial.print(Value);
                String Nothing = getValue(currentCommand, '/', 4);
                if (IDr.toInt() == ID && Mode.toInt() == 1) {
                        digital_pr(Pin.toInt(), Value.toInt());
                }

                if (Mode.toInt() == 9) {
                        if ( Pin.toInt() == 0 )
                                analogWrite(9, 0);
                        if ( Pin.toInt() == 1)
                                if ( (Value.toInt() >= 36) && ( Value.toInt() <= 96 ) )
                                        analogWrite(9, (Value.toInt()-36)*4);
                }
        }
}
