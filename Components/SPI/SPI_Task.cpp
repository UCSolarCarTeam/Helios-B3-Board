/**
******************************************************************************
* File Name          : SPI_Task.cpp
* Description        : Primary SPI task, reads data from pedals.
******************************************************************************
*/
#include "SystemDefines.hpp"
#include "SPI_Task.hpp"
#include "GPIO.hpp"

/**
* @brief Constructor for SPI Task
*/
SPI_Task::SPI_Task() : Task(SPI_TAK_QUEUE_DEPTH_OBJS)
{
}

/**
* @brief Initialize the SPI Task
*/
void SPI_Task::InitTask()
{
   CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize SPI task twice");

   BaseType_t rtValue =
       xTaskCreate((TaskFunction_t)SPI_Task::RunTask,
                   (const char*)"SPI Task",
                   (uint16_t)SPI_TASK_STACK_DEPTH_WORDS,
                   (void*)this,
                   (UBaseType_t)SPI_TASK_PRIORITY,
                   &rtTaskHandle);

   CUBE_ASSERT(rtValue == pdPASS, "SPI_Task::InitTask() - xTaskCreate() failed");
}

/**
* @brief Handles a command.
* @param cm Command reference to handle
*/
void SPI_Task::HandleCommand(Command& cm){

   switch (cm.GetCommand()) {
   case TASK_SPECIFIC_COMMAND: {
       break;
   }
   default:
       CUBE_PRINT("SPI Task - Received Unsupported Command {%d}\n", cm.GetCommand());
       break;
   }

   //No matter what we happens, we must reset allocated data
   cm.Reset();
}

void SPI_Task::BoardSelectLow(){
   GPIO::BOARD_SELECT_0::Off();
   GPIO::BOARD_SELECT_1::Off();
}

uint16_t SPI_Task::reverseBits(uint16_t bitsToReverse){
    uint16_t reversedBits = 0;
    for(int i = 0; i < 16; i++){

        if(bitsToReverse & (1 << i)){
            reversedBits |= (1 << (15-i));
        }

    }
    return reversedBits;
}


bool SPI_Task::SPI_Read(uint16_t sizeInBytes){
    if(HAL_SPI_Receive(SystemHandles::SPI1_Handle,last_read_,sizeInBytes,SPI1_TIMEOUT_MS) == HAL_OK){
        return true;
    }else{
        last_read_[0] = 0xFF;
        last_read_[1] = 0xFF; //Reset to indicate an error
        return false;
    }
}

/**
 * @brief Reads serial data output with bits properly reversed
 */
uint16_t SPI_Task::readData(void){

    bool failedToRead = !SPI_Read(2);
    if(failedToRead){
        return 0xFFFF;
    }

    // Combine the two bytes into a 16-bit raw data value
    uint16_t rawData = (last_read_[0] << 8) | last_read_[1];

    // Mask out the leading 4 bits (which should be zeros) and any trailing bits beyond 10
    rawData = (rawData >> 4) & 0x03FF; // Shift 4 bits to remove leading zeros, then mask to 10 bits

    return reverseBits(rawData);
}

/**
* @brief Instance Run loop for the SPI Task, runs on scheduler start as long as the task is initialized.
* @param pvParams RTOS Passed void parameters, contains a pointer to the object instance (from InitTask), should not be used
*/
void SPI_Task::Run(void * pvParams){


   /**
    * TODO: Logic
    * 1. First decoder selects Y0 as low to enable SPI Select on 2nd decoder
    *  -> Board Select 0 and 1 = LOW
    *
    * 2. SPI Chip Select to select which analog to receive
    * For Acceleration -> CSb00 -> SPI CS0 = 0 && SPI CS1 = 0
    * For Braking -> CSb01 -> SPI CS0 = 0 && SPI CS1 = 1
    *
    * 3. Read the ADC output on SPI MISO
   */
  BoardSelectLow();

  //Acceleration
  GPIO::SPI_DATA_CS0::Off();
  GPIO::SPI_DATA_CS1::Off();



  //Braking
  GPIO::SPI_DATA_CS0::Off();
  GPIO::SPI_DATA_CS1::On();

  osDelay(10); //Delay to reach 100 readings/s
}
