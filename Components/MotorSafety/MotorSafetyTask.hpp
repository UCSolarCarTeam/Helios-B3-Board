/**
 ******************************************************************************
 * File Name          : MotorSafetyTask.hpp
 * Description        : Polls and updates I2C GPIO expander in B3.
 ******************************************************************************
 */
#ifndef MOTORSAFETY_MOTORSAFETYTASK_HPP_
#define MOTORSAFETY_MOTORSAFETYTASK_HPP_


#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CAN/CANTxTask.hpp"
#include "Timer.hpp"

/*---------------------------------- Macros/Enums ----------------------------------*/
#define CAN_TX_ADDRESS 0x500

typedef struct {
    uint32_t ID;
    uint8_t DLC;
    uint8_t data[8];
} CANMessage;

/*---------------------------------- Task Implementation ----------------------------------*/
class MotorSafetyTask : public Task
{
public:
    static MotorSafetyTask &Inst()
    {
        static MotorSafetyTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void *pvParams) { MotorSafetyTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                               // Main run code

private:
    // Private Functions
    MotorSafetyTask();                            // Private constructor
    MotorSafetyTask(const MotorSafetyTask &);            // Prevent copy-construction
    MotorSafetyTask &operator=(const MotorSafetyTask &); // Prevent assignment

    void sendADCValues(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma , uint16_t* dma_adc_buf, uint8_t enable);
    uint8_t motorSafetyTask(uint16_t* dma_adc_buf, int16_t *motor_rpm, int16_t *motor_torque, int16_t *inv_peak_cur);

};

#endif /* MOTORSAFETY_MOTORSAFETYTASK_HPP_ */
