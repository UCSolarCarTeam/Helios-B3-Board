/**
 ******************************************************************************
 * File Name          : SPI_Task.hpp
 * Description        : Primary SPI task, reads data from pedals.
 ******************************************************************************
*/
#ifndef HELIOS_SPITASK_HPP_
#define HELIOS_SPITASK_HPP_

#include "Task.hpp"
#include "SystemDefines.hpp"

/* Macros/Enums ------------------------------------------------------------*/
enum SPI_COMMANDS  {
    SPI_NONE = 0, /** TODO: Add commands */
};

//SPI1 PA6, PA7
class SPI_Task: public Task
{
public:
    static SPI_Task& Inst(){ //Singleton Design Pattern
        static SPI_Task inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { WatchdogTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); //Main run code


private:
    SPI_Task();                                       // Private constructor
    SPI_Task(const SPI_Task&);                        // Prevent copy-construction
    SPI_Task& operator=(const SPI_Task&);             // Prevent assignment
};

#endif    // HELIOS_SPITASK_HPP_

