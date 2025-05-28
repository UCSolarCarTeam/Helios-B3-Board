/**
 ******************************************************************************
 * File Name          : I2C_Task.hpp
 * Description        : Polls and updates I2CTasks.
 ******************************************************************************
 */
#ifndef HELIOS_I2C_HPP_
#define HELIOS_I2C_HPP_

#include "Task.hpp"
#include "SystemDefines.hpp"
#include "M8N.h"
//#include "Timer.hpp"

/*---------------------------------- Macros/Enums ----------------------------------*/
enum I2C_Task
{
    I2C_STATE = 0,
    I2C_RESET
};

/*---------------------------------- Task Implementation ----------------------------------*/
class I2CTask : public Task
{
public:
    static I2CTask &Inst()
    {
        static I2CTask inst;
        return inst;
    }

    void InitTask();

    /**Getters for CAN formatting */
    uint8_t GPSData();
    uint8_t PollData();
    uint8_t CheckSum();
    //uint8_t LightsInputs();
    //uint16_t DigitalInputs();
    //uint8_t LightStatus();

protected:
    static void RunTask(void *pvParams) { I2CTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                               // Main run code

private:
    // Private Functions
    I2CTask();                            // Private constructor
    I2CTask(const I2CTask &);            // Prevent copy-construction
    I2CTask &operator=(const I2CTask &); // Prevent assignment

    void checkCounterTick();
    uint8_t counterTick = 0;
    //** Potential private variable for refactoring code
};

#endif
