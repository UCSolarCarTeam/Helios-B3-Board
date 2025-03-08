/**
 ******************************************************************************
 * File Name          : MotorControlTask.hpp
 * Description        : Sends commands to motor driver
 ******************************************************************************
 */
#ifndef MOTORCONTROL_MOTORCONTROLTASK_HPP_
#define MOTORCONTROL_MOTORCONTROLTASK_HPP_

#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CAN/CANTxTask.hpp"
#include "Timer.hpp"

/*---------------------------------- Macros/Enums ----------------------------------*/
typedef struct {
    uint8_t m1_status;  // 0 = off, 1 = on
    uint8_t m1_control;	// 0 = RPM, 1 = Torque
    uint8_t m1_mode;	// 0 - normal
    					// 1 - boost
    					// 2 - reverse (in analog control)
    					// 3 - regeneration
    uint8_t m1_dir;		// 0 = Forward, 1 = Reverse
    int16_t m1_val;    // Value of RPM or Torque

    uint8_t m2_status;
    uint8_t m2_control;
    uint8_t m2_mode;
    uint8_t m2_dir;
    int16_t m2_val;
} Motor_cmd;
/*---------------------------------- Task Implementation ----------------------------------*/
class MotorControlTask : public Task
{
public:
    static MotorControlTask &Inst()
    {
        static MotorControlTask inst;
        return inst;
    }

    void InitTask();


protected:
    static void RunTask(void *pvParams) { MotorControlTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                               // Main run code

private:
    // Private Functions
    MotorControlTask();                            // Private constructor
    MotorControlTask(const MotorControlTask &);            // Prevent copy-construction
    MotorControlTask &operator=(const MotorControlTask &); // Prevent assignment

    uint8_t CheckBuffer(uint8_t* buffer, uint8_t* buffer_index);
    uint8_t ParseMotorCommand(Motor_cmd* motor_cmd, uint8_t* buffer, uint8_t* last_message, uint8_t* adc_log_enable, Motor_cmd* last_motor_cmd, UART_HandleTypeDef* huart);
    void SendMotorCommand(Motor_cmd* motor_cmd, Motor_cmd* last_motor_cmd);
    void PrintMotorCommand(UART_HandleTypeDef* huart, Motor_cmd* motor_cmd);
};

#endif /* MOTORCONTROL_MOTORCONTROLTASK_HPP_ */
