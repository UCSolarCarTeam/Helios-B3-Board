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
#define SPI_TASK_FREQUENCY 20 //Every second
#define ADC_MIN 102 // Corresponds to 10% of V_in
#define ADC_MAX 921 // Corresponds to 90% of V_in
#define SPI_TASK_DELAY 1000 / SPI_TASK_FREQUENCY

enum SPI_COMMANDS
{
    SPI_NONE = 0, /** TODO: Add commands */
};

class SPI_Task : public Task
{
public:
    /**
     * @brief Singleton Constructor
     * TODO: Add &hspx as input for portability
     */
    static SPI_Task &Inst()
    { // Singleton Design Pattern
        static SPI_Task inst;
        return inst;
    }

    SPI_Task(); // Public Constructor for testing. Converting to private after

    void InitTask();

    /** Debug Functions Change to private later */
    uint16_t readAccelerationPedal_P();
    uint16_t readAccelerationPedal_N();
    uint16_t readBrakingPedal_P();
    uint16_t readBrakingPedal_N();

    /** Getters */
    uint16_t getAccelerationReading_P();
    uint16_t getAccelerationReading_N();
    uint16_t getBrakingReading_P();
    uint16_t getBrakingReading_N();

    float getAccelerationPedalPercent();
    float getBrakePedalPercent();
    uint8_t getAccelerationIntPercent();
    uint8_t getBrakeIntPercent();

protected:
    static void RunTask(void *pvParams) { SPI_Task::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                               // Main run code

    SPI_HandleTypeDef *hspi_ = nullptr;

    void HandleCommand(Command &cm);
    uint8_t last_read_[2] = {0xFF, 0xFF};

private:
    // SPI_Task(SPI_HandleTypeDef* hspi);               // Private constructor. PRODUCTION
    SPI_Task(const SPI_Task &);            // Prevent copy-construction
    SPI_Task &operator=(const SPI_Task &); // Prevent assignment

    //** Helper Functions */
    void BoardSelectLow();                               // Set board select GPIO pins low
    void BoardSelectHigh();                              // Set board select GPIO pins high
    uint16_t readData(void);                             // Reads serial data output of the conversion result
    bool SPI_Read(uint16_t sizeInBytes);                 // Reads SPI with HAL command onto protected last_read_
    float calculatePedalPosition(uint16_t pedalReading); // calculate the pedal position as a percentage
    
    void calculateAccelerationPedalPercent();
    void calculateBrakePedalPercent();

    // Getter Variables for CAN Task Communication
    uint16_t g_accelerationReading_P;
    uint16_t g_accelerationReading_N;
    uint16_t g_brakeReading_N;
    uint16_t g_brakeReading_P;

    float accelerationPedalPercent;
    float brakingPedalPercent;
};

#endif // HELIOS_SPITASK_HPP_
