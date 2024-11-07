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
    hspi_ = SystemHandles::SPI1_Handle;
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

void SPI_Task::BoardSelectHigh(){
   GPIO::BOARD_SELECT_0::On();
   GPIO::BOARD_SELECT_1::On();
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
    HAL_StatusTypeDef status =  HAL_SPI_Receive(hspi_,last_read_,sizeInBytes,SPI1_TIMEOUT_MS);
    if(status == HAL_OK){
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
    // 0000_00MX_XXXX_XXXL    0b1111_1111_1100
    rawData = (rawData << 6) & 0xFFC0;
    //Should be MX_XXXX_XXXL_00_0000 -> MXXX_XXXX_XL00_0000
    return rawData >> 4; //0000_00_MXXX_XXXX_XL
}

void SPI_Task::readAccelerationPedal_P(){
  GPIO::SPI_DATA_CS0::Off();
  GPIO::SPI_DATA_CS1::Off();
  BoardSelectLow();
  uint16_t accelerationPedalReading = readData();
  CUBE_PRINT("Acceleration P reading: %u\n", accelerationPedalReading);
  BoardSelectHigh();
}

void SPI_Task::readAccelerationPedal_N(){
  GPIO::SPI_DATA_CS0::On();
  GPIO::SPI_DATA_CS1::Off();
  BoardSelectLow();
  uint16_t accelerationPedalReading = readData();
  CUBE_PRINT("Acceleration N reading: %u\n", accelerationPedalReading);
  BoardSelectHigh();
}

void SPI_Task::readBrakingPedal_P(){
  GPIO::SPI_DATA_CS0::Off();
  GPIO::SPI_DATA_CS1::On();
  //Delay here
  BoardSelectLow();
  uint16_t breakPedalReading = readData();
  CUBE_PRINT("Braking P reading: %u\n", breakPedalReading);
  BoardSelectHigh();
}

void SPI_Task::readBrakingPedal_N(){
  GPIO::SPI_DATA_CS0::On();
  GPIO::SPI_DATA_CS1::On();
  BoardSelectLow();
  uint16_t breakPedalReading = readData();
  CUBE_PRINT("Braking N reading: %u\n", breakPedalReading);
  BoardSelectHigh();
}
/**
* @brief Instance Run loop for the SPI Task, runs on scheduler start as long as the task is initialized.
* @param pvParams RTOS Passed void parameters, contains a pointer to the object instance (from InitTask), should not be used
*/
void SPI_Task::Run(void * pvParams){


   /**
    * 1. First decoder selects Y0 as low to enable SPI Select on 2nd decoder
    *  -> Board Select 0 and 1 = LOW
    *
    * 2. SPI Chip Select to select which analog to receive
    * For Acceleration_P -> CSb00 -> SPI CS0 = 0 && SPI CS1 = 0
    * For Acceleration_N -> CSb01 -> SPI CS0 = 0 && SPI CS1 = 1
    * For Braking_P -> CSb10 -> SPI CS0 = 1 && SPI CS1 = 0
    * For Braking_N -> CSb11 -> SPI CS0 = 1 && SPI CS1 = 1
    *
    * 3. Read the ADC output on SPI MISO
   */
  while(1){
    readAccelerationPedal_P();
    readAccelerationPedal_N();
    readBrakingPedal_P();
    readBrakingPedal_N();
    osDelay(SPI_TASK_DELAY); //Delay to reach 100 readings/s
  }
}
