#ifndef PCA8575_IO_EXPANDER_HPP_
#define PCA8575_IO_EXPANDER_HPP_
/**
 * @file IOExpander.hpp
 * 
 * @brief IO Expander Driver
 * @note Designed to work as a class with 1:1 control of a PCA8575 IO Expander
 * 
 * @note Initialization Notes: 
 *      The IO expander will have a Power On Reset state 
 *          where all pins are set to HI/INPUT (see AN256 p.7)
 *      The IO expander will has a base address of 0x20, 
 *          use the CalculateAddress function to get the address based on the state of AD0, AD1, AD2
 *      If the MCU is reset, but the IO Expander is not, keep in mind the class assumes the IO Expander
 *          is in the default state. This means any pins not explicitly set with SetPin will be set to HI/INPUT.
 *      If reset recovery is important, manually call Update(), read the state of any OUTPUT pins, and set them.
 * 
 * @note Usage Notes:
 *      Note that SetPin and GetPinState functions do NOT cause transactions on the I2C bus,
 *         this is to allow for multiple pin changes or reads to be batched.
 *      If an updated pin read value is needed, call Update() before GetPinState or use GetPinStateNow.
 * 
 * @note Current Implementation:
 *      - The I2C line should be "exclusive" at the time of use (only one task uses this I2C line)
 *          If multiple tasks need to use this I2C line, modify I2C to be a interface (and use LL drivers ideally)
 *          and protect each line with a Mutex. Do NOT rely on the __HAL_LOCK.
 */


// -- Includes ------------------------------------------------------------------
#include "main_system.hpp" // This should have the platform specific HAL included
#include <stdint.h>

// -- Macros --------------------------------------------------------------------
#define I2C_TIMEOUT_MS 1000
#define I2C_ASSERT_WRITE_ADDR(addr) (addr & 0xFE)

// -- Enums ---------------------------------------------------------------------
enum IOExpanderPin {
    IO_P00 = 0,
    IO_P01 = 1,
    IO_P02 = 2,
    IO_P03 = 3,
    IO_P04 = 4,
    IO_P05 = 5,
    IO_P06 = 6,
    IO_P07 = 7,

    IO_P10 = 10,
    IO_P11 = 11,
    IO_P12 = 12,
    IO_P13 = 13,
    IO_P14 = 14,
    IO_P15 = 15,
    IO_P16 = 16,
    IO_P17 = 17,

    IO_NUM_PINS
};

enum IOState {
    LOW = 0,
    HIGH = 1,
    HI = HIGH,

    INPUT = 1,
    ERROR
};

// -- Typedefs ------------------------------------------------------------------
typedef hi2c_ptr I2C_HandleTypeDef*;

// -- Class ---------------------------------------------------------------------
/**
 * @brief PCA8575 IO Expander Class
 */
class IOExpander {
public:
    /**
     * @brief IOExpander Constructor
     * 
     * @param i2c I2C Handle
     * @param addr Device address, 8-bit form (shifted left 1), 0x20 | (AD2 << 2) | (AD1 << 1) | AD0
     */
    IOExpander(hi2c_ptr& i2c, uint8_t addr) : i2c_(I2C_ASSERT_WRITE_ADDR(addr)), address_(addr) {}

    bool Init(bool recover = false); // Initialize IO Expander

    bool Commit(); // Commit pending write
    bool SetPin(IOExpanderPin pin, IOState state); // Set pin state
    bool SetPinNow(IOExpanderPin pin, IOState state); // Set pin state and commit

    bool Update(); // Update IO Expander Read State
    IOState GetPinState(IOExpanderPin pin); // Get pin state (note. last read state)
    IOState GetPinStateNow(IOExpanderPin pin); // Get pin state with update

    static inline const uint8_t CalculateAddress(uint8_t a2, uint8_t a1, uint8_t a0);

protected:
    // -- Variables -------------------------------------------------------------
    hi2c_ptr& i2c_ = nullptr;
    uint8_t address_ = 0;

    uint8_t last_write_[2] = {0xFF, 0xFF};
    uint8_t pending_write_[2] = {0xFF, 0xFF};
    uint8_t last_read_[2] = {0xFF, 0xFF};

protected:
    // -- Platform specific functions -------------------------------------------
    bool I2C_Write(uint8_t dev, uint8_t* data, uint8_t len);
    bool I2C_Read(uint8_t dev, uint8_t* dest, uint8_t len);
};

// -- Static Functions -----------------------------------------------------------
/**
 * @brief Calculates address given the three address pins
 * 
 * @param ad2 Address pin AD2, must be 0 (LOW) or 1 (HIGH) 
 * @param ad1 Address pin AD1, must be 0 (LOW) or 1 (HIGH)
 * @param ad0 Address pin AD0, must be 0 (LOW) or 1 (HIGH) 
 * @return const uint8_t 
 */
inline const uint8_t IOExpander::CalculateAddress(uint8_t ad2, uint8_t ad1, uint8_t ad0) {
    return 0x20 | (ad2 << 2) | (ad1 << 1) | ad0;
}

// -- Platform Specific Functions ------------------------------------------------
inline bool IOExpander::I2C_Write(uint8_t dev, uint8_t* data, uint8_t len) {
    if (HAL_I2C_Master_Transmit(&i2c_t, dev, data, len, I2C_TIMEOUT_MS) == HAL_OK)
        return true;
    return false;
}

inline bool IOExpander::I2C_Read(uint8_t dev, uint8_t* dest, uint8_t len) {
    if (HAL_I2C_Master_Receive(&i2c_t, dev, dest, len, I2C_TIMEOUT_MS) == HAL_OK)
        return true;
    return false;
}

#endif // PCA8575_IO_EXPANDER_HPP_