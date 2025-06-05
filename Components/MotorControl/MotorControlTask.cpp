#include <string.h> // for memcpy

#include "cmsis_os.h"

#include "MotorControlTask.hpp"

CANMsg motor_drive_msg;
CANMsg motor_power_msg;

float regenValuesQueue[REGEN_QUEUE_SIZE] = {0};
float accelValuesQueue[ACCEL_QUEUE_SIZE] = {0};

uint8_t auxBmsInputs[3];
float motor0VehicleVelocityInput; /* TODO: Add Motor Feedback from CAN Rx Task */
float motor1VehicleVelocityInput; /* TODO: Add Motor Feedback from CAN Rx Task */

MotorControlTask::MotorControlTask() : Task(MOTOR_CONTROL_TASK_QUEUE_DEPTH_OBJS){}

void MotorControlTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Motor Control task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)MotorControlTask::RunTask,
                    (const char *)"MotorControlTask",
                    (uint16_t)MOTOR_CONTROL_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)MOTOR_CONTROL_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "MotorControlTask::InitTask() - xTaskCreate() failed");
}

void MotorControlTask::Run(void *pvParams)
{
    uint32_t prevWakeTime = osKernelSysTick();

    DriveCommandsInfo driveCommandsInfo =
    {
        .motorCurrentOut = 0.0f,
        .motorState = Off,
        .prevResetStatus = 0,
        .resetStatus = NotResetting,
        .regenQueueIndex = 0,
        .accelQueueIndex = 0,
    };

    uint32_t switching = 0;

    for (;;)
    {
        sendDriveCommands(&prevWakeTime, &driveCommandsInfo, &switching);
    }
}

// -----------------------------------------

CANMsg MotorControlTask::getMotorDrive(){
    motor_drive_msg.ID = 0;
	return motor_drive_msg;
}

CANMsg MotorControlTask::getMotorPower(){
    motor_power_msg.ID = 0;
	return motor_power_msg;
}

uint32_t MotorControlTask::getAvgRegen()
{
    float sum = 0;

    for (int i = 0; i < REGEN_QUEUE_SIZE; i++)
    {
        sum += regenValuesQueue[i];
    }

    return (uint32_t)((sum / (float)REGEN_QUEUE_SIZE));
}

uint32_t MotorControlTask::getAvgAccel()
{
    float sum = 0;

    for (int i = 0; i < ACCEL_QUEUE_SIZE; i++)
    {
        sum += accelValuesQueue[i];
    }

    return (uint32_t)((sum / (float)ACCEL_QUEUE_SIZE));
}

float MotorControlTask::calculateMotorCurrent(float accelPercentage)
{
    // To avoid a software overcurrent, our motor config
    // is set to have 100 A max current, we scale it so we send 69 A
    // on full pedal press

    if ((accelPercentage - NON_ZERO_THRESHOLD) > 0 )
    {
        return (accelPercentage - NON_ZERO_THRESHOLD)
               / (MAX_PEDAL_THRESHOLD - NON_ZERO_THRESHOLD)
               * MOTOR_PERCENTAGE_REDUCER;
    }
    else
    {
        return 0.0;
    }
}

float MotorControlTask::lowPassFilter(float presentMotorCurrent, float prevMotorCurrent)
{
    // Essentially a simple IIR low pass filter, which is a system that resists change.
    // The purpose is to smooth the output current so that there aren't any big jumps that could cause the motors to trip.
    // The MOTOR_CURRENT_SMOOTHING_FACTOR determines how resistant the low pass filter is to change
    //Disappointing
    return prevMotorCurrent + MOTOR_CURRENT_SMOOTHING_FACTOR * (presentMotorCurrent - prevMotorCurrent);
}

float MotorControlTask::calculateAccelMotorCurrent(float accelPercentage, float prevMotorCurrent)
{
    return lowPassFilter(calculateMotorCurrent(accelPercentage), prevMotorCurrent);
}

float MotorControlTask::calculateRegenMotorCurrent(float regenPercentage, float prevMotorCurrent)
{
    // Scale presentMotorCurrent by REGEN_INPUT_SCALING because regen uses less current than normal acceleration. Motors will trip if current is greater.
    float presentMotorCurrent = calculateMotorCurrent(regenPercentage) * REGEN_INPUT_SCALING ;
    return lowPassFilter(presentMotorCurrent, prevMotorCurrent);
}

uint8_t MotorControlTask::vehicleVelocitySafeToGoForward()
{
    return (motor0VehicleVelocityInput >= SAFE_VEHICLE_VELOCITY_TO_GO_FORWARD && motor1VehicleVelocityInput >= SAFE_VEHICLE_VELOCITY_TO_GO_FORWARD);
}

uint8_t MotorControlTask::vehicleVelocitySafeToGoReverse()
{
    return (motor0VehicleVelocityInput <= SAFE_VEHICLE_VELOCITY_TO_GO_REVERSE && motor1VehicleVelocityInput <= SAFE_VEHICLE_VELOCITY_TO_GO_REVERSE);
}

uint8_t MotorControlTask::isNewDirectionSafe(uint8_t forward, uint8_t reverse)
{
    if (forward && reverse) {
        // Forward and reverse pressed at the same time, not safe
        return 0;
        
    } else if ((forward && vehicleVelocitySafeToGoForward()) || (reverse && vehicleVelocitySafeToGoReverse())){
        // Forward or reverse pressed and vehicle velocity is safe to go in that direction
        return 1;
        
    } else {
        // default case, not safe
        return 0;
    }
}

void MotorControlTask::sendDriveCommands(uint32_t* prevWakeTimePtr,
                       DriveCommandsInfo* driveCommandsInfo,
                       uint32_t* switching)
{
    // osDelayUntil(prevWakeTimePtr, DRIVE_COMMANDS_FREQ);

    /*
    ---------- ADC Pedal Sampling ----------
    1. Read now pedal percentages
    2. calculate average pedal percentages from the buffer
    */

    regenValuesQueue[driveCommandsInfo->regenQueueIndex++] = brakingPedalPercent;
    accelValuesQueue[driveCommandsInfo->accelQueueIndex++] = accelerationPedalPercent;

    // Update Queue Indices
    driveCommandsInfo->accelQueueIndex %= REGEN_QUEUE_SIZE;
    driveCommandsInfo->regenQueueIndex %= ACCEL_QUEUE_SIZE;

    // Convert values back to floating percentages for motors
    float regenPercentage = (float)getAvgRegen() / 100.0f; // Get value between 0 and 1
    float accelPercentage = (float)getAvgAccel() / 100.0f;

    /* TODO: Add getter function for GPIO Forward, Reverse Brake pins */
    /* TODO: Add motor reset pin */
    // Determine drive commands (NOTE: ACTIVE LOW)
    uint8_t forward = forward_temp_GPIO;
    uint8_t reverse = reverse_temp_GPIO;
    uint8_t mech_brake = brake_temp_GPIO; // Mechanical Brake
    // uint8_t reset = reset_temp_GPIO;

    /* TODO: Add switch case handle for CANRx Task to receive AuxBMS states */
    // Read AuxBMS messages
    // NOTE: Hard coding states for now...
    char allowCharge = 1;
    char allowDischarge = 1;

    /*--------------- Determine Data to Send ---------------*/
    float motorVelocityOut; // RPM
    if (!isNewDirectionSafe(forward, reverse)) {                    
        motorVelocityOut = 0;
        driveCommandsInfo->motorCurrentOut = 0;

    } else if (driveCommandsInfo->resetStatus == SettingReset) {
        // If reset button is pressed, set motorState to Off and motorCurrentOut to 0
        driveCommandsInfo->motorCurrentOut = calculateRegenMotorCurrent(0, driveCommandsInfo->motorCurrentOut);

        if (driveCommandsInfo->motorCurrentOut < SWITCHING_CURRENT) {
            driveCommandsInfo->resetStatus = Resetting;
            driveCommandsInfo->motorCurrentOut = 0;
        }

    } else if (regenPercentage > NON_ZERO_THRESHOLD) {
        // Regen state
        // To stop using regen braking, set motorCurrentOut to desired value and zero motorVelocityOut
        // To stop without regen braking, zero both motorCurrentOut and motorVelocityOut
        // https://tritium.com.au/includes/TRI88.004v4-Users-Manual.pdf - Section 13

        if (driveCommandsInfo->motorState == Accelerating) {
            *switching = 1;
        }

        driveCommandsInfo->motorState = RegenBraking;

        if (*switching) {
            // If regen to accel, set regen percentage to 0
            driveCommandsInfo->motorCurrentOut = calculateRegenMotorCurrent(0, driveCommandsInfo->motorCurrentOut);

            if (driveCommandsInfo->motorCurrentOut < SWITCHING_CURRENT) {
                // reset switching flag after switching
                *switching = 0;
                driveCommandsInfo->motorCurrentOut = 0;
            }
            
        } else {
            motorVelocityOut = 0;
            // Allow regen braking based on input from AuxBMS
            if (allowCharge) {
                driveCommandsInfo->motorCurrentOut = calculateRegenMotorCurrent(
                                                        regenPercentage, 
                                                        driveCommandsInfo->motorCurrentOut
                                                    );

            } else {
                // If AuxBMS does not allow charging, set motorCurrentOut to 0
                driveCommandsInfo->motorCurrentOut = 0;
            }
        }
    } else if (mech_brake) {
        // If mechanical is pressed, set motorState to MechanicalBreaking and motorCurrentOut to 0
        driveCommandsInfo->motorState = MechanicalBreaking;
        motorVelocityOut = 0;
        driveCommandsInfo->motorCurrentOut = 0;

    } else if (accelPercentage > NON_ZERO_THRESHOLD) {
        // Accel state (drive state)

        if(driveCommandsInfo->motorState == RegenBraking) {
            *switching = 1;
        }

        driveCommandsInfo->motorState = Accelerating;

        if(*switching) {
            // If accel to regen, set accel percentage to 0
            driveCommandsInfo->motorCurrentOut = calculateAccelMotorCurrent(0, driveCommandsInfo->motorCurrentOut);

            if(driveCommandsInfo->motorCurrentOut < SWITCHING_CURRENT) {
                // reset switching flag after switching
                *switching = 0;
                driveCommandsInfo->motorCurrentOut = 0;
            }
        
        } else {
            if (forward && allowDischarge) {
                // Forward and Discharge is allowed
                driveCommandsInfo->motorState = Accelerating
                motorVelocityOut = MAX_FORWARD_RPM; // FAR FUTURE TODO: Based on ADC LOL (needs math, Omar's curve fitting) - Dom

                driveCommandsInfo->motorCurrentOut = CalculateAccelMotorCurrent(
                                                        accelPercentage, 
                                                        driveCommandsInfo->motorCurrentOut
                                                    );

            } else if (reverse && allowDischarge) {
                // Reverse and Discharge is allowed
                driveCommandsInfo->motorState = Accelerating;
                HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
                motorVelocityOut = MAX_REVERSE_RPM; // FAR FUTURE TODO: Based on ADC - Dom

                driveCommandsInfo->motorCurrentOut = calculateAccelMotorCurrent(
                                                        accelPercentage, 
                                                        driveCommandsInfo->motorCurrentOut
                                                    );
            } else {
                // If neither forward nor reverse
                // If discharge is not allowed, set motorState to Off
                driveCommandsInfo->motorState = Off;
                motorVelocityOut = 0;
                driveCommandsInfo->motorCurrentOut = 0;
            }
        }
    } else {
        // Off state
        if(driveCommandsInfo->motorState == Accelerating) {
            *switching = 1;
        }

        driveCommandsInfo->motorState = Off;

        if(*switching) {
            driveCommandsInfo->motorCurrentOut =
                    calculateRegenMotorCurrent(0, driveCommandsInfo->motorCurrentOut);

            if(driveCommandsInfo->motorCurrentOut < SWITCHING_CURRENT) {
                *switching = 0;
                driveCommandsInfo->motorCurrentOut = 0;
            }
            
        } else {
        driveCommandsInfo->motorState = Off;
        motorVelocityOut = 0;
        driveCommandsInfo->motorCurrentOut = 0;
        }
    }

    // Reset input velocities to default
    // This is TEMPORARY. Should be a fix for this that does a better job of determining if the velocities that were received
    // are stale values. i.e. motor controllers haven't transmitted a message in a while
    motor0VehicleVelocityInput = 0.0f;
    motor1VehicleVelocityInput = 0.0f;

    // Transmit Motor Drive command
    float dataToSendFloat[2];
    // ADD EXTENDED ID HERE IF NEEDED
    motor_drive_msg.extendedID = MOTOR_DRIVE_STDID;
    motor_drive_msg.DLC = MOTOR_DRIVE_DLC;
    dataToSendFloat[0] = motorVelocityOut;
    dataToSendFloat[1] = driveCommandsInfo->motorCurrentOut;
    memcpy(motor_drive_msg.data, dataToSendFloat, sizeof(float) * 2);
    
    CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, MOTOR_DRIVE_INPUT));

    // Transmit Motor Power command
    // ADD EXTENDED ID HERE IF NEEDED
    motor_power_msg.extendedID = MOTOR_POWER_STDID;
    motor_power_msg.DLC = MOTOR_POWER_DLC;
    dataToSendFloat[0] = 0.0f; // Reserved (defined by WaveSculptor datasheet)
    dataToSendFloat[1] = BUS_CURRENT_OUT;
    memcpy(motor_power_msg.data, dataToSendFloat, sizeof(float) * 2);

    CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, MOTOR_POWER_INPUT));

    // Transmit Motor Reset command if button switch went from off to on
    // `!` for active low
    // uint8_t reset = !HAL_GPIO_ReadPin(RESET_GPIO_Port, RESET_Pin); // ELECTRICAL WANTS TO CHANGE IT, ASSUME IT EXISTS
    uint8_t reset = reset_temp_GPIO;

    if (!driveCommandsInfo->prevResetStatus && !reset) /// off -> on
    {
        driveCommandsInfo->resetStatus = SettingReset;
    }

    if(driveCommandsInfo->resetStatus == Resetting) {

        CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, MOTOR_RESET_INPUT));

        driveCommandsInfo->resetStatus = NotResetting;
    }

    driveCommandsInfo->prevResetStatus = reset;
}

