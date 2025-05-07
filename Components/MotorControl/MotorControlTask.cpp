#include "MotorControlTask.hpp"
#include "CAN.h"
#include "CANRegisters.h"
#include "main.h"


Motor_cmd motor_cmd_init(void)
{
    Motor_cmd motor_cmd = {0};   // zero initialize
    return motor_cmd;
}

//alternative function to call Run
//void MotorControlTask::InitTask() {
    // Not used, required to satisfy base class interface
//}


// Static method for creating and running the task
void MotorControlTask::RunTask(void* pvParams)
{
    MotorControlTask::Inst().Run(pvParams);  // Call the instance's Run method
}

// You can remove the InitTask method entirely since RunTask now handles task creation.


void MotorControlTask::Run(void* pvParams)
{
    while (1)
    {
        // Check for received UART data and process the motor command
        uint8_t command_status = CheckBuffer(dma_uart_command_buf, command_end);

        if (command_status == 0) {
            command_end++;
        }
        else if (command_status == 1) {
            // Parse the motor command
            int parse_status = ParseMotorCommand(&motor_cmd, dma_uart_command_buf, last_message, &adc_log_en, &last_motor_cmd, &huart2);

            if (parse_status != 1) {
                char error_message[20] = {0};
                sprintf(error_message, "Command Error: %d\r\n", parse_status);
                HAL_UART_Transmit(&huart2, (uint8_t*)error_message, strlen(error_message), 100);
            }

            // Reset the DMA buffer and receive the next set of data
            HAL_UART_DMAStop(&huart2);
            HAL_UART_Receive_DMA(&huart2, dma_uart_command_buf, UART_BUF_LEN);
            memset(dma_uart_command_buf, 0, sizeof(dma_uart_command_buf));
            command_end = &dma_uart_command_buf[0];
        }

        uint32_t command_end_addr = (uint32_t)command_end;
        uint32_t buf_end_addr = (uint32_t)(&dma_uart_command_buf[UART_BUF_LEN] - 1);

        // Reset buffer if it reaches the end
        if (command_end_addr >= buf_end_addr) {
            command_end = &dma_uart_command_buf[0];
        }
    }
}

uint8_t MotorControlTask::CheckBuffer(uint8_t* buffer, uint8_t* buffer_index)
{
	 if ((uint8_t)(*buffer_index) == '\r') {
	        // Command finished
	        return 1;
	    }
	    else {
	        // Command not finished
	        return 0;
	    }
}


uint8_t MotorControlTask::ParseMotorCommand(Motor_cmd* motor_cmd, uint8_t* buffer, uint8_t* last_message, uint8_t* adc_log_enable, Motor_cmd* last_motor_cmd, UART_HandleTypeDef* huart)
{
	for(int i = 0; i < UART_BUF_LEN; i++){
	        last_message[i] = buffer[i];
	    }

	    char motor[5] = {0};
	    char option[20] = {0};
	    int16_t number = {0};

	    // format: m[id] [option] [number]
	    sscanf((char *)last_message, "%s %s %hd", motor, option, &number);

	    char update_msg[50] = {0};

	    // Motor 1
	    if (strcmp(motor, "m1") == 0){
	        // Status
	        if(strcmp(option, "on") == 0){
	            motor_cmd->m1_status = 1;
	            sprintf(update_msg, "Motor 1 Status: %d -> %d\r\n", last_motor_cmd->m1_status, motor_cmd->m1_status);
	        }
	        else if(strcmp(option, "off") == 0){
	            motor_cmd->m1_status = 0;
	            sprintf(update_msg, "Motor 1 Status: %d -> %d\r\n", last_motor_cmd->m1_status, motor_cmd->m1_status);
	        }

	        // Control Mode and Value
	        else if(strcmp(option, "torque") == 0){
	            if (number > 100){
	                number = 100;
	            } else if(number < -100){
	                number = -100;
	            }

	            motor_cmd->m1_val = number;
	            motor_cmd->m1_control = 1;

	            if(number < 0){
	                motor_cmd->m1_dir = 1;
	            } else {
	                motor_cmd->m1_dir = 0;
	            }

	            sprintf(update_msg, "Motor 1 Torque: %d%% -> %d%%\r\n", last_motor_cmd->m1_val, motor_cmd->m1_val);

	        }
	        else if(strcmp(option, "speed") == 0){
	            if(number > 10000){
	                number = 10000;
	            } else if(number < -10000){
	                number = -10000;
	            }

	            motor_cmd->m1_val = number;
	            motor_cmd->m1_control = 2;
	            if (number < 0){
	                motor_cmd->m1_dir = 1;
	            } else {
	                motor_cmd->m1_dir = 0;
	            }
	            sprintf(update_msg, "Motor 1 Speed: %0.1f RPM -> %0.1f RPM\r\n", (float)last_motor_cmd->m1_val/10, (float)motor_cmd->m1_val/10);
	        }

	        // Motor Mode
	        else if(strcmp(option, "default") == 0){
	            motor_cmd->m1_mode = 0;
	            sprintf(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
	        } else if(strcmp(option, "boost") == 0){
	            motor_cmd->m1_mode = 1;
	            sprintf(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
	        } else if(strcmp(option, "reverse") == 0){
	            motor_cmd->m1_mode = 2;
	            sprintf(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
	        } else if(strcmp(option, "regen") == 0){
	            motor_cmd->m1_mode = 3;
	            sprintf(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
	        }

	        else {
	            // unsupported option
	            return -3;
	        }
	    }

	    // Motor 2
	    else if(strcmp(motor, "m2") == 0){
	        // Status
	        if(strcmp(option, "on") == 0) {
	            motor_cmd->m2_status = 1;
	            sprintf(update_msg, "Motor 2 Status: %d -> %d\r\n", last_motor_cmd->m2_status, motor_cmd->m2_status);
	        }
	        else if(strcmp(option, "off") == 0){
	            motor_cmd->m2_status = 0;
	            sprintf(update_msg, "Motor 2 Status: %d -> %d\r\n", last_motor_cmd->m2_status, motor_cmd->m2_status);
	        }

	        // Control Mode and Value
	        else if(strcmp(option, "torque") == 0){
	            if (number > 100){
	                number = 100;
	            } else if(number < -100){
	                number = -100;
	            }
	            motor_cmd->m2_val = number;
	            motor_cmd->m2_control = 1;
	            if(number < 0){
	                motor_cmd->m2_dir = 1;
	            } else {
	                motor_cmd->m2_dir = 0;
	            }
	            sprintf(update_msg, "Motor 2 Torque: %d%% -> %d%%\r\n", last_motor_cmd->m2_val, motor_cmd->m2_val);
	        }
	        else if(strcmp(option, "speed") == 0){
	            if(number > 10000){
	                number = 10000;
	            } else if(number < -10000){
	                number = -10000;
	            }

	            motor_cmd->m2_val = number;
	            motor_cmd->m2_control = 2;
	            if (number < 0){
	                motor_cmd->m2_dir = 1;
	            } else {
	                motor_cmd->m2_dir = 0;
	            }
	            sprintf(update_msg, "Motor 2 Speed: %d RPM -> %d RPM\r\n", last_motor_cmd->m2_val/10, motor_cmd->m2_val/10);
	        }

	        // Motor Mode
	        else if(strcmp(option, "default") == 0){
	            motor_cmd->m2_mode = 0;
	            sprintf(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
	        } else if(strcmp(option, "boost") == 0){
	            motor_cmd->m2_mode = 1;
	            sprintf(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
	        } else if(strcmp(option, "reverse") == 0){
	            motor_cmd->m2_mode = 2;
	            sprintf(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
	        } else if(strcmp(option, "regen") == 0){
	            motor_cmd->m2_mode = 3;
	            sprintf(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
	        }

	        else { // unsupported option
	            return -3;
	        }
	    }

	    else if (strcmp(motor, "send") == 0){ // Send Command to Motors
	        SendMotorCommand(motor_cmd, last_motor_cmd);
	    }

	    else if (strcmp(motor, "print") == 0){ // Print to UART Terminal
	    	if(strcmp(option, "all") == 0){
	    		// Print both commands
	    		char msg[20] = {0};
	    		sprintf(msg, "Last Sent Command\r\n");
	    		HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), 100);
	    		PrintMotorCommand(huart, last_motor_cmd);

	    		sprintf(msg, "Current Command\r\n");
	    		HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), 100);
	    		PrintMotorCommand(huart, motor_cmd);

	    	} else if(strcmp(option, "last") == 0){
	    		// print last motor_cmd
	    		PrintMotorCommand(huart, last_motor_cmd);

	    	} else if(strcmp(option, "") == 0){
	    		// Print current command
	    		PrintMotorCommand(huart, motor_cmd);
	    	} else {
	            // unsupported option
	            return -3;
	        }
	    }

	    else if(strcmp(motor, "logadc") == 0){ // ADC Log
	    	if(strcmp(option, "on") == 0){
	    		*adc_log_enable = 1;
	            sprintf(update_msg, "ADC Logging %s\r\n", option);

	    	} else if(strcmp(option, "off") == 0){
	    		*adc_log_enable = 0;
	            sprintf(update_msg, "ADC Logging %s\r\n", option);

	    	} else { // unsupported option
	    		return -3;
	    	}
	    }

	    else { // unsupported motor
	        return -2;
	    }

	    HAL_UART_Transmit(huart, (uint8_t*)update_msg, strlen(update_msg), 100);

	    return 1;
}


void MotorControlTask::SendMotorCommand(Motor_cmd* motor_cmd, Motor_cmd* last_motor_cmd)
{
	 // Copy the current motor command to the last motor command
	    memcpy(last_motor_cmd, motor_cmd, sizeof(Motor_cmd));

	    uint8_t m1_message[8] = {0}; // 8 bytes for CAN m1_message
	    uint8_t m2_message[8] = {0}; // 8 bytes for CAN m2_message

	    if(motor_cmd->m1_status == 1){
	        m1_message[1] |= motor_cmd->m1_dir << 7;           // Direction (could be useless, check!)
	        m1_message[1] = motor_cmd->m1_val & 0xFF;          // Control Value Top
	        m1_message[0] = (motor_cmd->m1_val >> 8) & 0xFF;   // Control Value Bottom

	        uint8_t byte_3 = 0b00100000;            // bit 5 = 1 for software enable
	        byte_3 |= motor_cmd->m1_control;        // Control Mode 0 = Default, 1 = Torque, 2 = Speed
	        byte_3 |= (motor_cmd->m1_mode << 2);    // Motor Mode   0 = Default, 1 = Boost,  2 = Reverse, 3 = Regen

	        m1_message[2] = byte_3;
	    }

	    if(motor_cmd->m2_status == 1){
	        m2_message[1] |= motor_cmd->m2_dir << 7;           // Direction
	        m2_message[1] = motor_cmd->m2_val & 0xFF;          // Control Value Top
	        m2_message[0] = (motor_cmd->m2_val >> 8) & 0xFF;   // Control Value Bottom

	        uint8_t byte_3 = 0b00100000;            // bit 5 = 1 for software enable
	        byte_3 |= motor_cmd->m2_control;        // Control Mode 0 = Default, 1 = Torque, 2 = Speed
	        byte_3 |= motor_cmd->m2_mode << 2;      // Motor Mode   0 = Default, 1 = Boost,  2 = Reverse, 3 = Regen

	        m2_message[2] = byte_3;
	    }


	    // Create CAN message struct object for both m1 and m2 msg
	    // Send through extendedID 0x550
	    CANMsg m1_can_msg = {
	           .ID = 0,
	           .extendedID = 0x550,
	           .DLC = 8,
	           .data = {0}
	       };
	       memcpy(m1_can_msg.data, m1_message, 8);

	       CANMsg m2_can_msg = {
	           .ID = 0,
	           .extendedID = 0x550,
	           .DLC = 8,
	           .data = {0}
	       };
	       memcpy(m2_can_msg.data, m2_message, 8);

	       // Send CAN messages
	       sendExtendedCANMessage(&m1_can_msg, &peripheral1);
	       sendExtendedCANMessage(&m2_can_msg, &peripheral1);
	    // Send CAN message here
}


void MotorControlTask::PrintMotorCommand(UART_HandleTypeDef* huart, Motor_cmd* motor_cmd)
{

	char message[100] = {0};

	sprintf(message, "Motor: 1\r\n Status: %d\r\n Control: %d\r\n Mode: %d\r\n Dir: %d\r\n Val: %d\r\n\r\n", motor_cmd->m1_status, motor_cmd->m1_control, motor_cmd->m1_mode, motor_cmd->m1_dir, motor_cmd->m1_val);
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 100);

	sprintf(message, "Motor: 2\r\n Status: %d\r\n Control: %d\r\n Mode: %d\r\n Dir: %d\r\n Val: %d\r\n\r\n", motor_cmd->m2_status, motor_cmd->m2_control, motor_cmd->m2_mode, motor_cmd->m2_dir, motor_cmd->m2_val);
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 100);
}
