#include "CAN.h"

// defined this so I could say "for ever" in an  infinite for loop
#define ever (;;)

/*-------------------------------SPI interface instructions-------------------------------*/

/**
 * @brief write to registry in CAN IC       //FIXME: is this read or write...
 * @param address: hex address of the register
 * 		  bufffer: to store value read
 * @retval None
 */
void CAN_IC_READ_REGISTER(uint8_t address, uint8_t* buffer, CANPeripheral *peripheral)
{
	// Packet includes 3 bytes
	// 1st byte: 0x03 (specifies as read instruction)
	// 2nd byte: address of register to read
	uint8_t packet[2] = {0x03, address};

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET); // Initialize instruction by setting CS pin low
	status = HAL_SPI_Transmit(peripheral->hspi, packet, 2, 100U); //transmit
	status = HAL_SPI_Receive(peripheral->hspi, buffer, 1, 100U); //receive register contents
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET); // Terminate instruction by setting CS pin high
}

/**
 * @brief write to registry in CAN IC
 * @param address: hex address of the register
 * 		  value: value to be written to the register
 * @retval None
 */
void CAN_IC_WRITE_REGISTER(uint8_t address, uint8_t value, CANPeripheral *peripheral)
{
	// Packet includes 3 bytes
	// 1st byte: 0x02 (specifies as write instruction)
	// 2nd byte: address of register to write to
	// 3rd byte: value to write
	uint8_t packet[3] = {0x02, address, value};

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET); //set CS pin low
	status = HAL_SPI_Transmit(peripheral->hspi, packet, 3, 100U);	//transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET); //set CS pin high
}

/**
 * @brief write to a specific series of bits in a register in CAN IC
 * @param address: hex address of the register
 * 		  mask: bit mask
 * 		  value: value to be written to the register
 * @retval None
 */
void CAN_IC_WRITE_REGISTER_BITWISE(uint8_t address, uint8_t mask, uint8_t value, CANPeripheral *peripheral)
{
	// 0x05 specifies bit-write instruction
	// mask specifies which bits can be modified (1 means bit can be modified)
	uint8_t packet[4] = {0x05, address, mask, value};

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET); //set CS pin low
	status = HAL_SPI_Transmit(peripheral->hspi, packet, 4, 100U); //transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET); //set CS pin high
}

/**
 * @brief read status of CAN IC
 * @param buffer: buffer to write status
 * @retval None
 */
void CAN_IC_READ_STATUS(uint8_t* buffer, CANPeripheral *peripheral)
{
	// Packet includes read status instruction
	uint8_t packet[1] = {0xA0};

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET); // Initialize instruction by setting CS pin low
	status = HAL_SPI_Transmit(peripheral->hspi, packet, 1, 100U); //transmit
	status = HAL_SPI_Receive(peripheral->hspi, buffer, 1, 100U); //receive register contents
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET); // Terminate instruction by setting CS pin high
}

/**
 * @brief Reset the CAN IC
 * @param None
 * @retval None
 */
void CAN_IC_RESET(CANPeripheral *peripheral) {
	// Packet includes reset instruction
	uint8_t packet[1] = {0xC0};

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(peripheral->hspi, packet, 1, 100U);  //reset IC to default
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Request to send transmit buffer
 * @param channel: which buffer to send
 * @retval None
 */
void CAN_IC_REQUEST_TO_SEND(uint8_t channel, CANPeripheral *peripheral) {
	// T0: 0x01
	// T1: 0x02
	// T2: 0x04
	uint8_t packet = (1 << channel);

	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(peripheral->hspi, &packet, 1, 100U);  //reset IC to default
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET);
}


/**
  * @brief configure CAN IC through SPI
  * @param None
  * @retval None
  * Configuration is as close to Elysia's CAN configuration whenever possible
  * TODO: add configuration verification and return value accordingly
  */
void ConfigureCANSPI(CANPeripheral *peripheral)
{
	// TODO: ENSURE THAT THE IC IS IN CONFIG MODE...............

	// oof this is a mind fook
	// TODO: ask violet of this settings

	// Ensure IC is out of reset state (128 clock cycles)
	// HAL_Delay(100);

	// Tq = (2 x (BRP + 1)) / Fosc
	uint8_t CONFIG_CNF1 = 0x01; //BRP = 1 to make tq = 250ns and a SJW of 1Tq
	uint8_t CONFIG_CNF2 = 0x98; //PRSEG = 0, PHSEG1 = 3, SAM = 0, BTLMODE = 1
	uint8_t CONFIG_CNF3 = 0x01; //WAFKIL disabled, PHSEG2 = 2 (BTL enabled) but PHSEG = 1 makes it backwards compatible???? wat

	// uint8_t CONFIG_CNF1 = 0x07;  // BRP = 8 (baud rate prescaler), SJW = 1 TQ
	// uint8_t CONFIG_CNF2 = 0x90;  // PRSEG = 1 TQ, PHSEG1 = 5 TQ, sample point at 75%
	// uint8_t CONFIG_CNF3 = 0x02;  // PHSEG2 = 2 TQ

	HAL_StatusTypeDef status;

	// Reset CAN IC
	CAN_IC_RESET(peripheral);
	// HAL_Delay(100);

	// CANSTAT.OPMOD must read as config mode to be able to write to the registers (0x80)
	uint8_t CANSTAT_STATUS = 0;
	CAN_IC_READ_REGISTER(CANSTAT, &CANSTAT_STATUS, peripheral);

	// Ensure IC is in configuration mode
	if ((CANSTAT_STATUS & 0xE0) != 0x80) {
		CAN_IC_WRITE_REGISTER_BITWISE(CANCTRL, 0xE0, 0x80, peripheral);
		// HAL_Delay(100);
	}

	// Base IC Configuration Registers
	CAN_IC_WRITE_REGISTER_BITWISE(CNF1, 0xFF, CONFIG_CNF1, peripheral); //configure CNF1
	CAN_IC_WRITE_REGISTER_BITWISE(CNF2, 0xFF, CONFIG_CNF2, peripheral); //configure CNF2
	CAN_IC_WRITE_REGISTER_BITWISE(CNF3, 0x47, CONFIG_CNF3, peripheral); //configure CNF3

	// Receive Buffer Configurations
	// Recieve valid standard and extended message frames
	// Enable rollover: RXBnCTRL.BUKT = 1
	CAN_IC_WRITE_REGISTER_BITWISE(BFPCTRL, 0x0F, 0x0F, peripheral);
	CAN_IC_WRITE_REGISTER_BITWISE(RXB0CTRL, 0x64, 0x64, peripheral);
	CAN_IC_WRITE_REGISTER_BITWISE(RXB1CTRL, 0x60, 0x60, peripheral);

	CANSTAT_STATUS = 0;

	// Toggle CAN_TEST_SETUP to 1 for loopback mode, 0 for normal mode
	#if 0
		CAN_IC_WRITE_REGISTER_BITWISE(CANCTRL, 0xE7, 0x44, peripheral);	// Put IC in loop-back mode for testing as well as enable CLKOUT pin with 1:1 prescaler
		// HAL_Delay(100);
		CAN_IC_READ_REGISTER(CANSTAT, &CANSTAT_STATUS, peripheral); // 0x44
	#else
		CAN_IC_WRITE_REGISTER_BITWISE(CANCTRL, 0xE7, 0x04, peripheral); //Put IC in normal operation mode with CLKOUT pin enable and 1:1 prescaler
		// HAL_Delay(100);
		CAN_IC_READ_REGISTER(CANSTAT, &CANSTAT_STATUS, peripheral);
	#endif

	// Reset and configure interrupts
	CAN_IC_WRITE_REGISTER(CANINTE, 0xA0, peripheral); 	//configure interrupts, currently enable ERRIF
	CAN_IC_WRITE_REGISTER(CANINTF, 0x00, peripheral); 	//clear INTE flags
	CAN_IC_WRITE_REGISTER(EFLG, 0x00, peripheral);
}

/*-------------------------------------------------------------------------------------------*/

uint8_t checkAvailableTXChannel(CANPeripheral *peripheral)
{
    // uint32_t prevWakeTime = xTaskGetTickCount(); 	//Delay is fine if we have a CanTxGatekeeperTask

	//  Check if TXBnCTRL.TXREQ is set, if not then buffer is available to use
    for ever
    {
		/*
		TODO: REMOVE THIS STUFF
			When CANINTF.TXn is cleared, it indicates that "n" buffer is clear and can be used to put a message in...
			Use flags instead as it will be easier and faster...
		*/

		// Use "Read Status" command from IC to retrieve status of TXBnCTRL.TXREQ bits (remember to use mask, check datasheet for byte structure)
    	uint8_t CANStatus;
    	CAN_IC_READ_STATUS(&CANStatus, peripheral);

        uint8_t TXB0Status = CANStatus & 0b00000100;
        uint8_t TXB1Status = CANStatus & 0b00010000;
        uint8_t TXB2Status = CANStatus & 0b01000000;

        // CAN_IC_READ_REGISTER(TXB0CTRL, &TXB0Status, peripheral);
        // TXB0Status = TXB0Status & 0x08; //Not masking out bits
        if (!TXB0Status) {
            return 0;
        }

        // CAN_IC_READ_REGISTER(TXB1CTRL, &TXB1Status, peripheral);
        // TXB1Status = TXB1Status & 0x08; //Not masking out bits
        if (!TXB1Status) {
            return 1;
        }

        // CAN_IC_READ_REGISTER(TXB2CTRL, &TXB2Status, peripheral);
        // TXB2Status = TXB2Status & 0x08; //Not masking out bits
        if (!TXB2Status) {
            return 2;
        }

		osDelay(50);
        // prevWakeTime += TX_CHANNEL_CHECK_DELAY;
        // osDelayUntil(prevWakeTime);
    }
}

//TODO: make sendtxtask and a queue for it like the old mcu
/**
  * @brief send CAN message
  * @param None
  * @retval None
  */
void sendCANMessage(CANMsg *msg, CANPeripheral *peripheral)
{
	// Find TxBuffer to use
	uint8_t channel = checkAvailableTXChannel(peripheral);
    uint8_t initialBufferAddress = TXB0CTRL + 16*(channel);

	// Initializations
	uint8_t sendCommand = 0x80 + (0x01 << channel); 	   //instruction to send CAN message on buffer 1
	uint8_t TXBNSIDH = (msg->ID & 0b11111111000) >> 3; // mask upper ID register (SD 10-3)
	uint8_t TXBNSIDL = (msg->ID & 0b111) << 5; 	   	   // mask lower ID register (SD 2-0)
	uint8_t TXBNDLC = msg->DLC & 0x0F;				   // mask DLC

	// Set Standard Identifier and DLC
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 1, TXBNSIDH, peripheral); // SD 10-3
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 2, TXBNSIDL, peripheral); // SD 2-0
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 5, TXBNDLC, peripheral);  // DLC

	// Set data to registers
	uint8_t initialDataBufferAddress = initialBufferAddress + 6;
	for(int i = 0; i < msg->DLC; i++)
	{
		CAN_IC_WRITE_REGISTER(initialDataBufferAddress + i, msg->data[i], peripheral); //write to relevant data registers
	}

	// set transmit buffer priority to 3 (max)
	// write to TXBNCTRL<1:0>
	CAN_IC_WRITE_REGISTER_BITWISE(initialBufferAddress, 0x03, 0x03, peripheral);

	// Initiate message transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(peripheral->hspi, &sendCommand, 1, 100U);  // Send command to transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief send CAN message with extended identifier
  * @param None
  * @retval None
  */
void sendExtendedCANMessage(CANMsg *msg, CANPeripheral *peripheral)
{
	// Find TxBuffer to use
	// uint8_t initialBufferAddress = TXB0CTRL + 16*(channel); //TXB0CTRL for channel 1, TXB1CTRL for channel 2, TXB2CTRL for channel 3
    uint8_t channel = checkAvailableTXChannel(peripheral);
	uint8_t initialBufferAddress = TXB0CTRL + 16*(channel);

	// Initializations
	uint8_t sendCommand = 0x80 + (1 << channel); //instruction to send CAN message on channel
	uint8_t TXBNSIDH = (msg->extendedID >> 21) & 0xFF;
	uint8_t TXBNSIDL = (((msg->extendedID >> 18) & 0x07) << 5) | 0x08 | ((msg->extendedID >> 16) & 0x03);
	uint8_t TXBNEID8 = (msg->extendedID >> 8) & 0xFF;
	uint8_t TXBNEID0 = msg->extendedID & 0xFF;
	uint8_t TXBNDLC = msg->DLC & 0x0F;

	// Set Extended Identifier and DLC
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 1, TXBNSIDH, peripheral); // SD 10-3
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 2, TXBNSIDL, peripheral); // SD 2-0, ED 17-16
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 3, TXBNEID8, peripheral); // ED 15-8
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 4, TXBNEID0, peripheral); // ED 7-0
	CAN_IC_WRITE_REGISTER(initialBufferAddress + 5, TXBNDLC, peripheral);  // DLC

	uint8_t initialDataBufferAddress = initialBufferAddress + 6;
	for(int i = 0; i < msg->DLC; i++)
	{
		CAN_IC_WRITE_REGISTER(initialDataBufferAddress + i, msg->data[i], peripheral); //write to relevant data registers
	}

	CAN_IC_WRITE_REGISTER_BITWISE(initialBufferAddress, 0x03, 0x03, peripheral); //set transmit buffer priority to 3 (max)

	// Initiate message transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(peripheral->hspi, &sendCommand, 1, 100U);  //Send command to transmit
	HAL_GPIO_WritePin(peripheral->CS_PORT, peripheral->CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief Receive CAN message
  * @param None
  * @retval None
  */
void receiveCANMessage(uint8_t channel, uint32_t* ID, uint8_t* DLC, uint8_t* data, CANPeripheral *peripheral)
{
	// Check if channel is valid, should never go in with wrong channel
	if (channel > 1) {
		return;
	}

	uint8_t initialBufferAddress = RXB0CTRL + 16*(channel); //RXB0CTRL for channel 0, RXB1CTRL for channel 1

	uint8_t RXBNSIDH = 0;
	uint8_t RXBNSIDL = 0;
	uint8_t RXBDLC = 0;

	CAN_IC_READ_REGISTER(initialBufferAddress + 1, &RXBNSIDH, peripheral); // SD 10-3
	CAN_IC_READ_REGISTER(initialBufferAddress + 2, &RXBNSIDL, peripheral); //SD 2-0, IDE, ED 17-16
	CAN_IC_READ_REGISTER(initialBufferAddress + 5, &RXBDLC, peripheral);   //DLC

	if(RXBNSIDL & 0x08)	// Check RXBmSIDL.IDE to verify if CAN message has extended identifier
	{
		uint8_t RXBNEID8 = 0;
		uint8_t RXBNEID0 = 0;
		CAN_IC_READ_REGISTER(initialBufferAddress + 3, &RXBNEID8, peripheral); // ED 15-8
		CAN_IC_READ_REGISTER(initialBufferAddress + 4, &RXBNEID0, peripheral); // ED 7-0
		*ID = (RXBNSIDH << 21) | (((RXBNSIDL >> 5) & 0x07) << 18) | ((RXBNSIDL & 0x03) << 16) | (RXBNEID8 << 8) | (RXBNEID0);
	} else // CAN message is standard
	{
		*ID = (RXBNSIDH << 3) | (RXBNSIDL >> 5);
	}

	// Check data length of CAN message
	*DLC = RXBDLC & 0x0F;
	if(*DLC > 8){
		*DLC = 0;
	}

	uint8_t initialDataBufferAddress = initialBufferAddress + 6;
	for(int i = 0; i < *DLC; i++)
	{
		CAN_IC_READ_REGISTER(initialDataBufferAddress + i, &data[i], peripheral); //read from relevant data registers
	}

	CAN_IC_WRITE_REGISTER_BITWISE(CANINTF, channel + 1, 0, peripheral); //clear interrupts
	return;
}

