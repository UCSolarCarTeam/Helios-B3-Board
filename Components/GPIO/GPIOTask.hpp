/**
 ******************************************************************************
 * File Name          : GPIOTask.hpp
 * Description        : Polls and updates I2C GPIO expander in B3.
 ******************************************************************************
 */
#ifndef HELIOS_GPIOTASK_HPP_
#define HELIOS_GPIOTASK_HPP_

#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CANTx/CANTxTask.hpp"
#include "Timer.hpp"
#include "IOExpander.hpp"


/*---------------------------------- Macros/Enums ----------------------------------*/
enum GPIO_COMMANDS
{
    GPIO_STATE = 0,
    GPIO_RESET
};

/*---------------------------------- Task Implementation ----------------------------------*/
class GPIOTask : public Task
{
public:
    static GPIOTask &Inst()
    {
        static GPIOTask inst;
        return inst;
    }

    void InitTask();

    /** Getters for CAN formatting */
    uint8_t LightsInputs();
    uint16_t DigitalInputs();
    uint8_t LightStatus();

    /* Getters for Motor Command GPIO */
    uint8_t getForwardGPIO();
    uint8_t getReverseGPIO();
    uint8_t getBrakeGPIO();
    uint8_t getResetGPIO();
    uint8_t getMotorControlGPIO();

protected:
    static void RunTask(void *pvParams) { GPIOTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                               // Main run code

private:
    // Private Functions
    GPIOTask();                            // Private constructor
    GPIOTask(const GPIOTask &);            // Prevent copy-construction
    GPIOTask &operator=(const GPIOTask &); // Prevent assignment

    void checkCounterTick();
    uint8_t counterTick = 0;
    //** Potential private variable for refactoring code
    // IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0)); */
};

#endif
