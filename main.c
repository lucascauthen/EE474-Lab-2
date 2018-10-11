#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>

long delay = 1000;

enum myBool {
    FALSE = 0, TRUE = 1
};

typedef enum myBool Bool;

enum color {
    GREEN,
    ORANGE,
    RED,
    NONE
};

typedef enum color Color;

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

void powerSubsystemTask(void *powerSubsystemData);

void thrusterSubsystemTask(void *thrusterSubsystemData);

void satelliteComsTask(void *satelliteComsData);

void consoleDisplayTask(void *consoleDisplayData);

void warningAlarmTask(void *warningAlarmData);

unsigned long systemTime(); //TODO Implement this behavior

int randomInteger(int low, int high);

void scheduleTask(TCB *tasks[6]);

void print(char str[], Color color, int line);

int randomSeed = 1;

/*
void print(char str[], int length, int color, int line) {
	//To flash the selected line, you must print exact same string black then recolor
	for (int i = 0; i < length; i++) {
		tft.setTextColor(color);
		tft.setCursor(i*12, line*16);
		tft.print(str[i]);
	}
}
 */

void run() {
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
    if (nextExecutionTime == 0 || systemTime() > nextExecutionTime) {
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
                *data->batteryLevel = (unsigned short)result;
            }
        } else { //If not deplyed
            int result = *data->batteryLevel - 3 * (*(data->powerConsumption));
            if (result < 0) {
                *data->batteryLevel = 0;
            } else {
                *data->batteryLevel = (unsigned short)result;
            }
        }
        nextExecutionTime = systemTime() + delay;
        executionCount++;
    }
}

void thrusterSubsystemTask(void *thrusterSubsystemData) {
    static unsigned long nextExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
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


        nextExecutionTime = systemTime() + delay;
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
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
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
        nextExecutionTime = systemTime() + delay;
    }
}

void consoleDisplayTask(void *consoleDisplayData) {
    static unsigned long nextExecutionTime = 0;
    if (nextExecutionTime == 0 || nextExecutionTime < systemTime()) {
        ConsoleDisplayData *data = (ConsoleDisplayData *) consoleDisplayData;
        Bool inStatusMode = FALSE; //TODO get this from some external input
        //printf("consoleDisplayTask\n");
        if (inStatusMode) {
            //Print
            //Solar Panel State
            //Battery Level
            //Fuel Level
            //Power Consumption
            printf("\tSolar Panel State: %s\n", *data->solarPanelState ? " ON" : "OFF");
            printf("\tBattery Level: %d\n", *data->batteryLevel);
            printf("\tFuel Level: %d\n", *data->fuelLevel);
            printf("\tPower Consumption: %d\n", *data->powerConsumption);
            printf("\tPower Generation: %d\n", *data->powerGeneration);
        } else {

        }
        nextExecutionTime = systemTime() + delay;
    }
}

void warningAlarmTask(void *warningAlarmData) {
    WarningAlarmData *data = (WarningAlarmData *) warningAlarmData;
    //printf("warningAlarmTask\n");
    static Color fuelStatus = NONE;
    static Color batteryStatus = NONE;
    static unsigned long hideFuelTime = 0;
    static unsigned long showFuelTime = 0;
    static unsigned long hideBatteryTime = 0;
    static unsigned long showBatteryTime = 0;

    int fuelDelay = (*data->fuelLevel <= 10) ? 2000 : 1000;
    Color fuelColor = (*data->fuelLevel <= 10) ? RED : ORANGE;

    if (*data->fuelLevel <= 50) {
        if (fuelStatus == fuelColor) {
            if (showFuelTime == 0) { //If showing fuel status
                if (hideFuelTime < systemTime()) {
                    showFuelTime = systemTime() + fuelDelay;
                    hideFuelTime = 0;
                    //TODO hide fuel status with color fuelColor
                    print("FUEL", NONE, 0);
                }
            } else { //If hiding fuel status
                if (showFuelTime < systemTime()) {
                    hideFuelTime = systemTime() + fuelDelay;
                    showFuelTime = 0;
                    //TODO show fuel status with fuelColor
                    print("FUEL", fuelColor, 0);
                }
            }
        } else {
            fuelStatus = fuelColor;
            print("FUEL", fuelColor, 0);
            hideFuelTime = systemTime() + fuelDelay;
        }
    } else if (fuelStatus != GREEN) {
        print("FUEL", GREEN, 0);
        fuelStatus = GREEN;
    }

    int batteryDelay = (*data->batteryLevel <= 10) ? 1000 : 2000;
    Color batteryColor = (*data->batteryLevel <= 10) ? RED : ORANGE;
    if (*data->batteryLevel <= 50) {
        if (batteryStatus == batteryColor) {
            if (showBatteryTime == 0) { //If showing battery status
                if (hideBatteryTime < systemTime()) {
                    showBatteryTime = systemTime() + batteryDelay;
                    hideBatteryTime = 0;
                    //TODO hide battery status with color batteryColor
                    print("BATTERY", NONE, 1);
                }
            } else { //If hiding battery status
                if (showBatteryTime < systemTime()) {
                    hideBatteryTime = systemTime() + batteryDelay;
                    showBatteryTime = 0;
                    //TODO show battery status
                    print("BATTERY", batteryColor, 1);
                }
            }
        } else {
            batteryStatus = batteryColor;
            print("BATTERY", batteryColor, 1);
            hideBatteryTime = systemTime() + batteryDelay;
        }
    } else if (batteryStatus != GREEN) {
        print("BATTERY", GREEN, 1);
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
        randomSeed = randomSeed * multiplier + addOn;
        randNum = randomSeed;

        if (randNum < 0) {
            randNum = randNum + max;
        }

        randNum = randNum / max;

        retVal = ((int) ((high - low + 1) * randNum)) + low;
    }

    return retVal;
}

void print(char str[], Color color, int line) {
    switch (color) {
        case GREEN:
            printf("\t%s GREEN %d\n", str, line);
            break;
        case ORANGE:
            printf("\t%s ORANGE %d\n", str, line);
            break;
        case RED:
            printf("\t%s RED %d\n", str, line);
            break;
        case NONE:
            printf("\t%s BLACK %d\n", str, line);
            break;
    }
}

unsigned long systemTime() {
    return (unsigned long) time(NULL) * 1000;
}


int main() {
    run();
    return 0;
}
