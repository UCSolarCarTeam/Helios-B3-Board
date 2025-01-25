// -- Includes ------------------------------------------------------------------
#include "IOExpander.hpp"

// -- Function Implementation --------------------------------------------------
/**
 * @brief Commit pending write, also initializes the device for read states
 * 
 * @return true on success, false on failure
 */ 
bool IOExpander::Commit() {
    if (I2C_Write(address_, pending_write_, 2)) {
        last_write_[0] = pending_write_[0];
        last_write_[1] = pending_write_[1]; 
        return true;
    }
    return false;
}

/**
 * @brief Set output to given state
 * @note Does NOT commit the write 
 *      (device won't change state until Commit() is called)
 * 
 * @param pin Pin to set
 * @param state State to set (LOW, HIGH, INPUT)
 * 
 * @return true on success, false on failure (invalid arguments)
 */
bool IOExpander::SetPin(IOPin pin, IOState state) {
    // Check for invalid arguments
    if(pin >= IOExpanderPin::IO_NUM_PINS ||
       state >= IOState::ERROR) return false;
    
    // Set pending write
    uint8_t port = (uint8_t)pin / 10;
    uint8_t num = (uint8_t)pin % 10;
    pending_write_[port] = (pending_write_[port] & ~(1 << num)) | ((uint8_t)state << num);

    return true;
}

/**
 * @brief Set output to given state and commit the write
 * 
 * @param pin Pin to set
 * @param state State to set (LOW, HIGH, INPUT)
 * 
 * @return true on success, false on failure
 */
bool IOExpander::SetPinNow(IOPin pin, IOState state) {
    if (SetPin(pin, state)) {
        return Commit();
    }
    return false;
}

/**
 * @brief Toggle pin state (from LOW to HIGH or HIGH to LOW)
 * 
 * Based on the current pending write state
 * 
 * @param pin Pin to toggle
 * @return true on success, false on failure (invalid arguments)
 */
bool IOExpander::TogglePin(IOPin pin) {
    // Check for invalid arguments
    if(pin >= IOExpanderPin::IO_NUM_PINS) return false;
    
    // Set pending write
    uint8_t port = (uint8_t)pin / 10;
    uint8_t num = (uint8_t)pin % 10;
    pending_write_[port] ^= (1 << num);

    return true;
}

/**
 * @brief Toggle pin state and commit
 * 
 * @param pin Pin to toggle
 * @return true on success, false on failure 
 */
bool IOExpander::TogglePinNow(IOPin pin) {
    if (TogglePin(pin)) {
        Commit();
    }
    return false;
}

/**
 * @brief Update IO Expander Read State
 * @return true on success, false on failure
 */
bool IOExpander::Update() {
    if (I2C_Read(address_, last_read_, 2)) {
        return true;
    }
    return false;
}

/**
 * @brief Get the pin state (note. comes from the LAST time Update() was called) 
 * 
 * @param pin Pin to get state of
 * @return IOState State of the pin
 *     ERROR if pin is not set as an input
 *           or if pin is invalid
 */
IOState IOExpander::GetPinState(IOPin pin) {
    // Check for invalid arguments
    if(pin >= IOExpanderPin::IO_NUM_PINS) return IOState::ERROR;
    
    // Check if pin is an input based on last write
    uint8_t port = (uint8_t)pin / 10;
    uint8_t num = (uint8_t)pin % 10;
    if (last_write_[port] & (1 << num)) {
        return last_read_[port] & (1 << num) ? IOState::HIGH : IOState::LOW;
    }

    return IOState::ERROR;
}

/**
 * @brief Get pin state with immediate update 
 * 
 * @param pin Pin to get state of
 * @return IOState State of the pin
 *    ERROR if pin is not set as an input
 *         or if pin is invalid
 *         or if I2C read fails
 */
IOState IOExpander::GetPinStateNow(IOPin pin) {
    if (Update()) {
        return GetPinState(pin);
    }
    return IOState::ERROR;
}

/**
 * @brief Get pin state of all pins
 * 
 * @return array of IOState, each index corresponding to each pin
 */
std::array<IOState, 16> IOExpander::GetExpanderState() {
    std::array<IOState, 16> expanderState;

    for(uint8_t i = 0; i < 8; i++) {
        IOPin pin = static_cast<IOPin>(i);
        expanderState[i] = GetPinState(pin);
    }

    for(uint8_t i = 10; i < 18; i++) {
        IOPin pin = static_cast<IOPin>(i);
        expanderState[i - 2] = GetPinState(pin);
    }

    return expanderState;
}

/**
 * @brief Get pin in state of all pins with Update
 * 
 * @return array of IOState, each index corresponding to a pin
 */
std::array<IOState, 16> IOExpander::GetExpanderStateNow() {
    if(Update()) {
        return GetExpanderState();
    }
        // Initialize and return an array filled with ERROR state for failure
    std::array<IOState, 16> errorState;
    errorState.fill(IOState::ERROR);
    return errorState;
}

