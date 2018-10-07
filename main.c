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

unsigned long systemTime(); //TODO implement this function


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
}

void powerSubsystemTask(void *powerSubsystemData) {
    PowerSubsystemData *data = (PowerSubsystemData *) powerSubsystemData;
    unsigned long time = systemTime();

    //powerConsumption
    //TODO Fix powerConsumption
    if (time % 2 == 0) {
        *(data->powerConsumption) += 2;
    } else {

    }
    //powerGeneration
    if (data->solarPanelState) {
        if (*data->batteryLevel > 95) {
            *data->solarPanelState = FALSE;
        } else if (*data->batteryLevel < 50) {
            //Increment the variable by 2 every even numbered time
            if(time % 2 == 0) {
                (*data->powerGeneration) += 2;
            } else { //Increment the variable by 1 every odd numbered time
                (*data->powerGeneration) += 1;
            }
        } else {
            //Increment the variable by 2 every even numbered time
            if(time % 2 == 0) {
                (*data->powerGeneration) += 2;
            }
        }
    } else {
        if(*data->batteryLevel <= 10) {
            *data->solarPanelState = TRUE;
        }
    }

    //batteryLevel
    if(*data->solarPanelState) { //If deployed
        *data->batteryLevel = *data->batteryLevel - (*(data->powerConsumption)) + (*(data->powerConsumption));
    } else { //If not deplyed
        *data->batteryLevel = *data->batteryLevel - 3 * (*(data->powerConsumption));
    }
}

void thrusterSubsystemTask(void *thrusterSubsystemData) {
    ThrusterSubsystemData *data = (ThrusterSubsystemData *) thrusterSubsystemData;

}

void satelliteComsTask(void *satelliteComsData) {
    SatelliteComsData *data = (SatelliteComsData *) satelliteComsData;

}

void consoleDisplayTask(void *consoleDisplayData) {
    ConsoleDisplayData *data = (ConsoleDisplayData *) consoleDisplayData;

}

void warningAlarmTask(void *warningAlarmData) {
    WarningAlarmData *data = (WarningAlarmData *) warningAlarmData;

}


int main() {
    run();
    return 0;
}