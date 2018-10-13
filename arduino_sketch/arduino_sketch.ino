
// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.
//Technical support:goodtft@163.com

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <limits.h> // Used for random number generation

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define NONE   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFC00

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

enum myBool {
    FALSE = 0, TRUE = 1
};
typedef enum myBool Bool;

long runDelay = 5000;
long randomGenerationSeed = 1000;
Bool shouldPrintTaskTiming = TRUE;


//Thrust Control
unsigned int ThrusterControl = 0;

//Power Management
unsigned short BatteryLevel = 100;
unsigned short FuelLevel = 100;
unsigned short PowerConsumption = 0;
unsigned short PowerGeneration = 0;

//Solar Panel Control
Bool SolarPanelState = FALSE;

//Status Management and Annunciation
//Same as Power Management

//Warning Alarm
Bool FuelLow = FALSE;
Bool BatteryLow = FALSE;

struct TaskStruct {
    void (*task)(void *);

    void *taskDataPtr;
};

typedef struct TaskStruct TCB;


struct PowerSubsystemDataStruct {
    Bool *solarPanelState;
    unsigned short *batteryLevel;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
};
typedef struct PowerSubsystemDataStruct PowerSubsystemData;

struct ThrusterSubsystemDataStruct {
    unsigned int *thrusterControl;
    unsigned short *fuelLevel;
};
typedef struct ThrusterSubsystemDataStruct ThrusterSubsystemData;

struct SatelliteComsDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    Bool *solarPanelState;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
    unsigned int *thrusterControl;
};
typedef struct SatelliteComsDataStruct SatelliteComsData;

struct ConsoleDisplayDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    Bool *solarPanelState;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
    unsigned short *powerConsumption;
    unsigned short *powerGeneration;
};
typedef struct ConsoleDisplayDataStruct ConsoleDisplayData;

struct WarningAlarmDataStruct {
    Bool *fuelLow;
    Bool *batteryLow;
    unsigned short *batteryLevel;
    unsigned short *fuelLevel;
};
typedef struct WarningAlarmDataStruct WarningAlarmData;


//Controls the execution of the power subsystem
void powerSubsystemTask(void *powerSubsystemData);

//Controls the execution of the thruster subsystem
void thrusterSubsystemTask(void *thrusterSubsystemData);

//Controls the execution of the satellite coms subsystem
void satelliteComsTask(void *satelliteComsData);

//Controls the execution of the console display subsystem
void consoleDisplayTask(void *consoleDisplayData);

//Controls the execution of the warning alarm subsystem
void warningAlarmTask(void *warningAlarmData);

//Returns the current system time in milliseconds
unsigned long systemTime();

//Returns a random integer between low and high inclusively
int randomInteger(int low, int high);

//Runs the loop of all six tasks, does not run the task if the task pointer is null
void scheduleTask(TCB *tasks[6]);

//Prints a string to the tft given text, the length of the text, a color, and a line number
void print(char str[], int length, int color, int line);

//Starts up the system by creating all the objects that are needed to run the system
void setupSystem();

//Prints timing information for a function based on its last runtime
void printTaskTiming(char taskName[], unsigned long lastRunTime);


//Arduino setup function
void setup(void) {
    Serial.begin(9600); //Sets baud rate to 9600
    Serial.println(F("TFT LCD test")); //Prints to serial monitor

//determines if shield or board
#ifdef USE_Elegoo_SHIELD_PINOUT
    Serial.println(F("Using Elegoo 2.4\" TFT Arduino Shield Pinout"));
#else
    Serial.println(F("Using Elegoo 2.4\" TFT Breakout Board Pinout"));
#endif

    //prints out tft size
    Serial.print("TFT size is ");
    Serial.print(tft.width());
    Serial.print("x");
    Serial.println(tft.height());

    tft.reset();
    tft.setTextSize(2);
    //prints out the current LCD driver version
    uint16_t identifier = tft.readID();
    if (identifier == 0x9325) {
        Serial.println(F("Found ILI9325 LCD driver"));
    } else if (identifier == 0x9328) {
        Serial.println(F("Found ILI9328 LCD driver"));
    } else if (identifier == 0x4535) {
        Serial.println(F("Found LGDP4535 LCD driver"));
    } else if (identifier == 0x7575) {
        Serial.println(F("Found HX8347G LCD driver"));
    } else if (identifier == 0x9341) {
        Serial.println(F("Found ILI9341 LCD driver"));
    } else if (identifier == 0x8357) {
        Serial.println(F("Found HX8357D LCD driver"));
    } else if (identifier == 0x0101) {
        identifier = 0x9341;
        Serial.println(F("Found 0x9341 LCD driver"));
    } else if (identifier == 0x1111) {
        identifier = 0x9328;
        Serial.println(F("Found 0x9328 LCD driver"));
    } else { //prints to serial monitor if wiring is bad or unknown LCD driver
        Serial.print(F("Unknown LCD driver chip: "));
        Serial.println(identifier, HEX);
        Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
        Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
        Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
        Serial.println(F("If using the breakout board, it should NOT be #defined!"));
        Serial.println(F("Also if using the breakout, double-check that all wiring"));
        Serial.println(F("matches the tutorial."));
        identifier = 0x9328;

    }
    tft.begin(identifier);
    tft.fillScreen(NONE);

}

//Arduino loop
void loop(void) {
    setupSystem();
}

void setupSystem() {
    TCB *queue[6];

    /*
     * Init the various tasks
     */

    //Power Subsystem
    TCB powerSubsystem;
    PowerSubsystemData powerSubsystemData;
    powerSubsystemData.solarPanelState = &SolarPanelState;
    powerSubsystemData.batteryLevel = &BatteryLevel;
    powerSubsystemData.powerConsumption = &PowerConsumption;
    powerSubsystemData.powerGeneration = &PowerGeneration;

    powerSubsystem.taskDataPtr = (void *) &powerSubsystemData;
    powerSubsystem.task = &powerSubsystemTask;

    queue[0] = &powerSubsystem;

    //Thruster Subsystem
    TCB thrusterSubsystem;
    ThrusterSubsystemData thrusterSubsystemData;
    thrusterSubsystemData.fuelLevel = &FuelLevel;
    thrusterSubsystemData.thrusterControl = &ThrusterControl;

    thrusterSubsystem.taskDataPtr = (void *) &thrusterSubsystemData;
    thrusterSubsystem.task = &thrusterSubsystemTask;

    queue[1] = &thrusterSubsystem;

    //Satellite Comms
    TCB satelliteComs;
    SatelliteComsData satelliteComsData;
    satelliteComsData.thrusterControl = &ThrusterControl;
    satelliteComsData.fuelLevel = &FuelLevel;
    satelliteComsData.powerGeneration = &PowerGeneration;
    satelliteComsData.powerConsumption = &PowerConsumption;
    satelliteComsData.batteryLevel = &BatteryLevel;
    satelliteComsData.solarPanelState = &SolarPanelState;
    satelliteComsData.batteryLow = &BatteryLow;
    satelliteComsData.fuelLow = &FuelLow;

    satelliteComs.taskDataPtr = (void *) &satelliteComsData;
    satelliteComs.task = &satelliteComsTask;

    queue[2] = &satelliteComs;

    //Console Display
    TCB consoleDisplay;
    ConsoleDisplayData consoleDisplayData;
    consoleDisplayData.fuelLow = &FuelLow;
    consoleDisplayData.batteryLow = &BatteryLow;
    consoleDisplayData.solarPanelState = &SolarPanelState;
    consoleDisplayData.batteryLevel = &BatteryLevel;
    consoleDisplayData.fuelLevel = &FuelLevel;
    consoleDisplayData.powerConsumption = &PowerConsumption;
    consoleDisplayData.powerGeneration = &PowerGeneration;

    consoleDisplay.taskDataPtr = (void *) &consoleDisplayData;
    consoleDisplay.task = &consoleDisplayTask;

    queue[3] = &consoleDisplay;

    //Warning Alarm
    TCB warningAlarm;
    WarningAlarmData warningAlarmData;
    warningAlarmData.batteryLevel = &BatteryLevel;
    warningAlarmData.batteryLow = &BatteryLow;
    warningAlarmData.fuelLow = &FuelLow;
    warningAlarmData.fuelLevel = &FuelLevel;

    warningAlarm.taskDataPtr = (void *) &warningAlarmData;
    warningAlarm.task = &warningAlarmTask;

    queue[4] = &warningAlarm;
    queue[5] = 0x0;

    scheduleTask(queue);
}

void scheduleTask(TCB *tasks[6]) {
    unsigned int currentTaskIndex = 0;
    while (1) {
        //Major cycle
        while (currentTaskIndex < 6) {
            TCB *task = tasks[currentTaskIndex];
            if (task != 0x0) { //Filter out null pointer
                task->task(task->taskDataPtr);
            }
            currentTaskIndex++;
        }
        currentTaskIndex = 0;
    }
}

void powerSubsystemTask(void *powerSubsystemData) {
    //Count of the number times this function is called.
    // It is okay if this number wraps to 0 because we just care about if the function call is odd or even
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || systemTime() > nextExecutionTime) {
        printTaskTiming("powerSubsystemTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        PowerSubsystemData *data = (PowerSubsystemData *) powerSubsystemData;
        static unsigned int executionCount = 0;
        //powerConsumption
        static Bool consumptionIncreasing = TRUE;
        if (consumptionIncreasing) {
            if (executionCount % 2 == 0) {
                *(data->powerConsumption) += 2;
            } else {
                *(data->powerConsumption) -= 1;
            }
            if (*(data->powerConsumption) > 10) {
                consumptionIncreasing = FALSE;
            }
        } else {
            if (executionCount % 2 == 0) {
                *(data->powerConsumption) -= 2;
            } else {
                *(data->powerConsumption) += 1;
            }
            if (*(data->powerConsumption) < 5) {
                consumptionIncreasing = TRUE;
            }
        }

        //powerGeneration
        if (*data->solarPanelState) {
            if (*data->batteryLevel > 95) {
                *data->solarPanelState = FALSE;
                *data->powerGeneration = 0;
            } else if (*data->batteryLevel < 50) {
                //Increment the variable by 2 every even numbered time
                if (executionCount % 2 == 0) {
                    (*data->powerGeneration) += 2;
                } else { //Increment the variable by 1 every odd numbered time
                    (*data->powerGeneration) += 1;
                }
            } else {
                //Increment the variable by 2 every even numbered time
                if (executionCount % 2 == 0) {
                    (*data->powerGeneration) += 2;
                }
            }
        } else {
            if (*data->batteryLevel <= 10) {
                *data->solarPanelState = TRUE;
            }
        }
        //batteryLevel
        if (*data->solarPanelState) { //If deployed
            short result = *data->batteryLevel - (*(data->powerConsumption)) + (*(data->powerGeneration));
            if (result < 0) {
                *data->batteryLevel = 0;
            } else {
                *data->batteryLevel = min((unsigned short) result, 100);
            }
        } else { //If not deplyed
            int result = *data->batteryLevel - 3 * (*(data->powerConsumption));
            if (result < 0) {
                *data->batteryLevel = 0;
            } else {
                *data->batteryLevel = (unsigned short) result;
            }
        }
        nextExecutionTime = systemTime() + runDelay;
        executionCount++;
    }
}

void thrusterSubsystemTask(void *thrusterSubsystemData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
        printTaskTiming("thrusterSubsystemTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        ThrusterSubsystemData *data = (ThrusterSubsystemData *) thrusterSubsystemData;
        unsigned short left = 0, right = 0, up = 0, down = 0;

        unsigned int signal = *(data->thrusterControl);

        unsigned int direction = signal & (0xF); // Get the last 4 bits
        unsigned int magnitude = (signal & (0xF0)) >> 4; // Get the 5-7th bit and shift if back down
        unsigned int duration = (signal & (0xFF00)) >> 8;

        //printf("\t\tDirection %d\n", direction);
        //printf("\t\tMagnitude %d\n", magnitude);
        //printf("\t\tDuration %d\n", duration);

        //printf("thrusterSubsystemTask\n");

        //TODO Change the fuel level based on the extracted values
        if (*data->fuelLevel > 0 && (int) *data->fuelLevel >= ((int) *data->fuelLevel - 4 * duration / 100)) {
            *data->fuelLevel = max(0, *data->fuelLevel -
                                      4 * duration / 100); //magnitude at this point is full on and full off
        } else {
            *data->fuelLevel = 0;
        }


        nextExecutionTime = systemTime() + runDelay;
    }

}

unsigned int getRandomThrustSignal() {
    unsigned int signal = 1;
    unsigned short direction = (unsigned short) randomInteger(0, 4);
    if (direction == 4) //No thrust
        return 0;
    signal = signal << direction;
    unsigned int magnitude = randomInteger(0, 15);
    unsigned int duration = randomInteger(0, 255);

    signal = signal | (magnitude << 4);
    signal = signal | (duration << 8);
    return signal;
}

void satelliteComsTask(void *satelliteComsData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
        printTaskTiming("satelliteComsTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        SatelliteComsData *data = (SatelliteComsData *) satelliteComsData;
        //printf("satelliteComsTask\n");
        //TODO: In future labs, send the following data:
        /*
            * Fuel Low
            * Battery Low
            * Solar Panel State
            * Battery Level
            * Fuel Level
            * Power Consumption
            * Power Generation
         */

        *(data->thrusterControl) = getRandomThrustSignal();
        nextExecutionTime = systemTime() + runDelay;
    }
}

void consoleDisplayTask(void *consoleDisplayData) {
    static unsigned long nextExecutionTime = 0;
    static unsigned long lastExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
        printTaskTiming("consoleDisplayTask", lastExecutionTime);
        lastExecutionTime = systemTime();
        ConsoleDisplayData *data = (ConsoleDisplayData *) consoleDisplayData;
        Bool inStatusMode = TRUE; //TODO get this from some external input
        //printf("consoleDisplayTask\n");
        if (inStatusMode) {
            //Print
            //Solar Panel State
            //Battery Level
            //Fuel Level
            //Power Consumption
            Serial.print("\tSolar Panel State: ");
            Serial.println((*data->solarPanelState ? " ON" : "OFF"));
            Serial.print("\tBattery Level: ");
            Serial.println(*data->batteryLevel);
            Serial.print("\tFuel Level: ");
            Serial.println(*data->fuelLevel);
            Serial.print("\tPower Consumption: ");
            Serial.println(*data->powerConsumption);
            Serial.print("\tPower Generation: ");
            Serial.println(*data->powerGeneration);

        } else {
            if (*data->fuelLow == TRUE) {
                Serial.println("Fuel Low!");
            }
            if (*data->batteryLow== TRUE) {
                Serial.println("Battery Low!");
            }
        }
        Serial.println();
        nextExecutionTime = systemTime() + runDelay;
    }
}

void warningAlarmTask(void *warningAlarmData) {
    WarningAlarmData *data = (WarningAlarmData *) warningAlarmData;
    //printf("warningAlarmTask\n");
    static int fuelStatus = NONE;
    static int batteryStatus = NONE;
    static unsigned long hideFuelTime = 0;
    static unsigned long showFuelTime = 0;
    static unsigned long hideBatteryTime = 0;
    static unsigned long showBatteryTime = 0;

    *data->fuelLow = *data->fuelLevel <= 10 ? TRUE : FALSE;
    *data->batteryLow = *data->batteryLow <= 10 ? TRUE : FALSE;

    int fuelDelay = (*data->fuelLevel <= 10) ? 2000 : 1000;
    int fuelColor = (*data->fuelLevel <= 10) ? RED : ORANGE;

    if (*data->fuelLevel <= 50) {
        if (fuelStatus == fuelColor) {
            if (showFuelTime == 0) { //If showing fuel status
                if (hideFuelTime < systemTime()) {
                    showFuelTime = systemTime() + fuelDelay;
                    hideFuelTime = 0;
                    //TODO hide fuel status with color fuelColor
                    print("FUEL", 4, NONE, 0);
                }
            } else { //If hiding fuel status
                if (showFuelTime < systemTime()) {
                    hideFuelTime = systemTime() + fuelDelay;
                    showFuelTime = 0;
                    //TODO show fuel status with fuelColor
                    print("FUEL", 4, fuelColor, 0);
                }
            }
        } else {
            fuelStatus = fuelColor;
            print("FUEL", 4, fuelColor, 0);
            hideFuelTime = systemTime() + fuelDelay;
        }
    } else if (fuelStatus != GREEN) {
        print("FUEL", 4, GREEN, 0);
        fuelStatus = GREEN;
    }

    int batteryDelay = (*data->batteryLevel <= 10) ? 1000 : 2000;
    int batteryColor = (*data->batteryLevel <= 10) ? RED : ORANGE;
    if (*data->batteryLevel <= 50) {
        if (batteryStatus == batteryColor) {
            if (showBatteryTime == 0) { //If showing battery status
                if (hideBatteryTime < systemTime()) {
                    showBatteryTime = systemTime() + batteryDelay;
                    hideBatteryTime = 0;
                    //TODO hide battery status with color batteryColor
                    print("BATTERY", 7, NONE, 1);
                }
            } else { //If hiding battery status
                if (showBatteryTime < systemTime()) {
                    hideBatteryTime = systemTime() + batteryDelay;
                    showBatteryTime = 0;
                    //TODO show battery status
                    print("BATTERY", 7, batteryColor, 1);
                }
            }
        } else {
            batteryStatus = batteryColor;
            print("BATTERY", 7, batteryColor, 1);
            hideBatteryTime = systemTime() + batteryDelay;
        }
    } else if (batteryStatus != GREEN) {
        print("BATTERY", 7, GREEN, 1);
        batteryStatus = GREEN;
    }
}


int randomInteger(int low, int high) {
    double randNum = 0.0;
    int multiplier = 2743;
    int addOn = 5923;
    double max = INT_MAX + 1.0;

    int retVal = 0;

    if (low > high)
        retVal = randomInteger(high, low);
    else {
        randomGenerationSeed = randomGenerationSeed * multiplier + addOn;
        randNum = randomGenerationSeed;

        if (randNum < 0) {
            randNum = randNum + max;
        }

        randNum = randNum / max;

        retVal = ((int) ((high - low + 1) * randNum)) + low;
    }

    return retVal;
}

void print(char str[], int length, int color, int line) {
    //To flash the selected line, you must print exact same string black then recolor
    for (int i = 0; i < length; i++) {
        tft.setTextColor(color);
        tft.setCursor(i * 12, line * 16);
        tft.print(str[i]);
    }
}

void printTaskTiming(char taskName[], unsigned long lastRunTime) {
    if(shouldPrintTaskTiming) {
        Serial.print(taskName);
        Serial.print(" - cycle delay: ");
        if(lastRunTime > 0) {
            Serial.println((double) (systemTime() - lastRunTime) / 1000.0, 4);
        } else {
            Serial.println(0.0, 4);
        }
    }
}

unsigned long systemTime() {
    return millis();
}
