/**
 ******************************************************************************
 * File Name          : SPI_Task.hpp
 * Description        : Primary SPI task, reads data from pedals.
 ******************************************************************************
*/
#ifndef HELIOS_SPITASK_HPP_
#define HELIOS_SPITASK_HPP_

#include "Task.hpp"
#include "main_system.hpp" //Has specific SPI HAL function included
#include <inttypes.h>
#include "SystemDefines.hpp"

/* Macros/Enums ------------------------------------------------------------*/
#define SPI1_TIMEOUT_MS 1000
#define SPI_TASK_FREQUENCY 1
#define SPI_TASK_DELAY  1000/SPI_TASK_FREQUENCY

enum SPI_COMMANDS  {
    SPI_NONE = 0, /** TODO: Add commands */
};

class SPI_Task: public Task
{
public:
    /**
     * @brief Singleton Constructor
     * TODO: Add &hspx as input for portability
     */
    static SPI_Task& Inst(SPI_HandleTypeDef* hspi = nullptr){ //Singleton Design Pattern
        static SPI_Task inst(hspi);
        return inst;
    }

    SPI_Task(SPI_HandleTypeDef* hspi);               // Public Constructor testing

    void InitTask();

    /** Debug Functions Change to private later */
    void readAccelerationPedal();
    void readBrakingPedal();

protected:
    static void RunTask(void* pvParams) { SPI_Task::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); //Main run code

    SPI_HandleTypeDef* hspi_ = nullptr;

    void HandleCommand(Command& cm);
    uint8_t last_read_[2] = {0xFF, 0xFF};

private:
    // SPI_Task(SPI_HandleTypeDef* hspi);               // Private constructor. PRODUCTION
    SPI_Task(const SPI_Task&);                        // Prevent copy-construction
    SPI_Task& operator=(const SPI_Task&);             // Prevent assignment

    //** Helper Functions */
    void BoardSelectLow();                            //Set board select GPIO pins low
    uint16_t reverseBits(uint16_t bitsToReverse);     //Reverses bits of ADC reading as MSB is read first
    uint16_t readData(void);                          //Reads serial data output of the conversion result
    bool SPI_Read(uint16_t sizeInBytes);               //Reads SPI with HAL command onto protected last_read_

};


#endif    // HELIOS_SPITASK_HPP_

