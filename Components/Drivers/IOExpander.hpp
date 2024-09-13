#ifndef PCA8575_IO_EXPANDER_HPP_
#define PCA8575_IO_EXPANDER_HPP_
// -- Includes ------------------------------------------------------------------
#include "main_system.hpp" // This should have the platform specific HAL included
#include <stdint.h>

// -- Macros --------------------------------------------------------------------

#define I2C_7B_ADDR_TO_8B_W(addr) ((addr << 1) & 0xFE)
#define I2C_WRITE_TO_READ_ADDR(addr) (addr | 0x01)

// -- Typedefs ------------------------------------------------------------------
typedef i2c_t I2C_HandleTypeDef;

// -- Class ---------------------------------------------------------------------
class IOExpander {
public:
    IOExpander(i2c_t& i2c) : i2c_(i2c) {}

protected:
    // -- Variables -------------------------------------------------------------
    i2c_t& i2c_;


protected:
    // -- Platform specific functions -------------------------------------------
    bool I2C_Write(uint8_t dev, uint8_t* data, uint8_t len);
    bool I2C_Read(uint8_t dev, uint8_t* dest, uint8_t len);

    // -- Wrapper functionality for I2C -----------------------------------------
    inline uint8_t I2C_WriteRegister(uint8_t dev, uint8_t reg, uint8_t data) {
        uint8_t buf[2] = {reg, data};
        return I2C_Write(dev, buf, 2);
    }

    inline uint8_t I2C_ReadRegisters(uint8_t dev, uint8_t reg, uint8_t* dest, uint8_t len) {
        if (!I2C_Write(dev, &reg, 1)) return 1;
        return I2C_Read(I2C_WRITE_TO_READ_ADDR(dev), dest, len);
    }
};

// -- Platform Specific Functions ------------------------------------------------
inline bool IOExpander::I2C_Write(uint8_t dev, uint8_t* data, uint8_t len) {
    if (HAL_I2C_Master_Transmit(&i2c_t, dev, data, len, 1000) == HAL_OK)
        return true;
    return false;
}

inline bool IOExpander::I2C_Read(uint8_t dev, uint8_t* dest, uint8_t len) {
    if (HAL_I2C_Master_Receive(&i2c_t, dev, dest, len, 1000) == HAL_OK)
        return true;
    return false;
}

#endif // PCA8575_IO_EXPANDER_HPP_