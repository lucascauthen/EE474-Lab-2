#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

enum myBool {
    FALSE = 0, TRUE = 1
};

typedef enum myBool Bool;

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

unsigned long systemTime; //TODO Implement this behavior

int randomInteger(int low, int high);

void scheduleTask(TCB *tasks[6]);

void delay(unsigned long delayMs);

int randomSeed = 1;


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

    scheduleTask(queue);
}

void delay(unsigned long delayMs) {
    systemTime += delayMs;
    //TODO add sleep function
}

void scheduleTask(TCB *tasks[6]) {
    unsigned int currentTaskIndex = 0;
    while (1) {
        //Major cycle
        while (currentTaskIndex < 6) {
            TCB *task = tasks[currentTaskIndex];
            task->task(task->taskDataPtr);
            delay(50);
            currentTaskIndex++;
        }
        //Minor cycle
        //TODO
    }
}

void powerSubsystemTask(void *powerSubsystemData) {
    PowerSubsystemData *data = (PowerSubsystemData *) powerSubsystemData;
    //Count of the number times this function is called.
    // It is okay if this number wraps to 0 because we just care about if the function call is odd or even
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
    if (data->solarPanelState) {
        if (*data->batteryLevel > 95) {
            *data->solarPanelState = FALSE;
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
        *data->batteryLevel = *data->batteryLevel - (*(data->powerConsumption)) + (*(data->powerConsumption));
    } else { //If not deplyed
        *data->batteryLevel = *data->batteryLevel - 3 * (*(data->powerConsumption));
    }
    executionCount++;
}

void thrusterSubsystemTask(void *thrusterSubsystemData) {
    ThrusterSubsystemData *data = (ThrusterSubsystemData *) thrusterSubsystemData;
    unsigned short left = 0, right = 0, up = 0, down = 0;

    unsigned int signal = *(data->thrusterControl);

    unsigned int direction = signal & (0xF); // Get the last 4 bits
    unsigned int magnitude = (signal & (0xF0)) >> 4; // Get the 5-7th bit and shift if back down
    unsigned int duration = (signal & (0xFF00)) >> 8;

    //TODO Change the fuel level based on the extracted values

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
    SatelliteComsData *data = (SatelliteComsData *) satelliteComsData;

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
}

void consoleDisplayTask(void *consoleDisplayData) {
    ConsoleDisplayData *data = (ConsoleDisplayData *) consoleDisplayData;
    Bool inStatusMode = TRUE; //TODO get this from some extranal input
    if (inStatusMode) {
        //Print
        //Solar Panel State
        //Battery Level
        //Fuel Level
        //Power Consumption
    } else {

    }
}

void warningAlarmTask(void *warningAlarmData) {
    WarningAlarmData *data = (WarningAlarmData *) warningAlarmData;

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


int main() {
    run();
    return 0;
}