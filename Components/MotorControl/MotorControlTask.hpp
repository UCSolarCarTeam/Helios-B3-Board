/**
 ******************************************************************************
 * File Name          : MotorControlTask.hpp
 * Description        : Header for the Motor Control Task
 ******************************************************************************
 */
#ifndef HELIOS_MOTOR_CONTROL_TASK_HPP_
#define HELIOS_MOTOR_CONTROL_TASK_HPP_

#ifndef ELYSIA
#define ELYSIA

/* Includes ------------------------------------------------------------------*/
#include "main_system.hpp"
#include "Task.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"

#include "SystemDefines.hpp"
#include "CubeDefines.hpp"
#include "CANTxTask.hpp"
#include "CAN.h"

extern volatile float accelerationPedalPercent;
extern volatile float brakingPedalPercent;

/* Enums ------------------------------------------------------------------*/

/* Macros ------------------------------------------------------------------*/

/* Class ------------------------------------------------------------------*/

enum MotorStates {
    Accelerating,
    RegenBraking,
    MechanicalBreaking,
    Off
};

enum ResetStatus {
    NotResetting,
    SettingReset,
    Resetting
};

typedef struct DriveCommandsInfo
{
    float motorCurrentOut;
    enum MotorStates motorState;
    uint8_t prevResetStatus;
    enum ResetStatus resetStatus;
    uint8_t regenQueueIndex;
    uint8_t accelQueueIndex;
} DriveCommandsInfo;

class MotorControlTask : public Task
{
public:
    static MotorControlTask &Inst()
    {
        static MotorControlTask inst;
        return inst;
    }

    void InitTask();

    // Getters for CANTx Task
    CANMsg getMotorDrive();
    CANMsg getMotorPower();

    // Helper functions
    uint32_t getAvgRegen();
    uint32_t getAvgAccel();
    float calculateMotorCurrent(float accelPercentage);
    float lowPassFilter(float presentMotorCurrent, float prevMotorCurrent);
    float calculateAccelMotorCurrent(float accelPercentage, float prevMotorCurrent);
    float calculateRegenMotorCurrent(float regenPercentage, float prevMotorCurrent);

    // Task functions
    void sendDriveCommands(uint32_t* prevWakeTimePtr, DriveCommandsInfo* driveCommandsInfo, uint32_t* switching);
    void sendDriveCommandsTask(void const* arg);

    uint8_t vehicleVelocitySafeToGoForward();
    uint8_t vehicleVelocitySafeToGoReverse();
    uint8_t isNewDirectionSafe(uint8_t forward, uint8_t reverse);

protected:
    static void RunTask(void *pvParams) { MotorControlTask::Inst().Run(pvParams); }   // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                                         // Main run code
    void HandleCommand(Command &cm);

private:
    MotorControlTask();                                                          // Private constructor
    MotorControlTask(const MotorControlTask &);                                 // Prevent copy-construction
    MotorControlTask &operator=(const MotorControlTask &);                     // Prevent assignmen
};


#define REGEN_QUEUE_SIZE 5
#define ACCEL_QUEUE_SIZE 5

#define BUS_CURRENT_OUT 1.0f // Percentage 0 -1 always 100%

#define DRIVE_COMMANDS_FREQ 10
#define MOTOR_DRIVE_STDID 0x501U
#define MOTOR_DRIVE_DLC 8
#define MOTOR_POWER_STDID 0x502U
#define MOTOR_POWER_DLC 8
#define MOTOR_RESET_STDID 0x503U
#define MOTOR_PERCENTAGE_REDUCER 1.0f

#define SWITCHING_CURRENT 0.01f

#define MAX_FORWARD_RPM 20000
#define MAX_REVERSE_RPM -20000
#define NON_ZERO_THRESHOLD 0.17f
#define MAX_PEDAL_THRESHOLD 0.71f
#define MAX_ANALOG 4095 // 12bit ADC (2^12)
#define REGEN_INPUT_SCALING 0.175f
#define MOTOR_CURRENT_SMOOTHING_FACTOR 0.20f // Smooth current output to prevent big jumps (0-1)

#define SAFE_VEHICLE_VELOCITY_TO_GO_FORWARD -1.38f // A small negative number to have some room for things like slow turns (metres/s)
#define SAFE_VEHICLE_VELOCITY_TO_GO_REVERSE 1.38f // A small positive number to have some room for things like slow turns (metres/s)

#define LAP_PIN CONTEXT_Pin
#define LAP_GPIO_PORT CONTEXT_GPIO_Port

extern uint8_t auxBmsInputs[3];
extern float   motor0VehicleVelocityInput;
extern float   motor1VehicleVelocityInput;




//void sendDriveCommands(uint32_t* prevWakeTimePtr, DriveCommandsInfo* driveCommandsInfo, uint32_t* switching);     are used now as class members
//void sendDriveCommandsTask(void const* arg);

#endif    //ELYSIA
#endif
