#include "gpio_lab.h"
#include "lpc40xx.h"

//------------------------GPIO-1------------------------------------------------
/// Should alter the hardware registers to set the pin as input
void gpio1__set_as_input(uint8_t pin_num) { LPC_GPIO1->DIR &= ~(1 << pin_num); }

/// Should alter the hardware registers to set the pin as output
void gpio1__set_as_output(uint8_t pin_num) { LPC_GPIO1->DIR |= (1 << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpio1__set_high(uint8_t pin_num) { LPC_GPIO1->PIN |= (1 << pin_num); }

/// Should alter the hardware registers to set the pin as low
void gpio1__set_low(uint8_t pin_num) { LPC_GPIO1->PIN &= ~(1 << pin_num); }

// Should alter the hardware registers to set the pin as low
// @param {bool} high - true => set pin high, false => set pin low
void gpio1__set(uint8_t pin_num, bool high) {
  if (high == true) {
    LPC_GPIO1->PIN |= (1 << pin_num);
  } else {
    LPC_GPIO1->PIN &= ~(1 << pin_num);
  }
}

// Should return the state of the pin (input or output, doesn't matter)
// @return {bool} level of pin high => true, low => false
bool gpio1__get_level(uint8_t pin_num) {
  if (LPC_GPIO1->PIN & (1 << pin_num)) {
    return true;
  } else {
    return false;
  }
}

//------------------------GPIO-0------------------------------------------------
void gpio0__set_as_input(uint8_t pin_num) { LPC_GPIO0->DIR &= ~(1 << pin_num); }

void gpio0__set_as_output(uint8_t pin_num) { LPC_GPIO0->DIR |= (1 << pin_num); }

void gpio0__set_high(uint8_t pin_num) { LPC_GPIO0->PIN |= (1 << pin_num); }

void gpio0__set_low(uint8_t pin_num) { LPC_GPIO0->PIN &= ~(1 << pin_num); }

void gpio0__set(uint8_t pin_num, bool high) {
  if (high == true) {
    LPC_GPIO0->PIN |= (1 << pin_num);
  } else {
    LPC_GPIO0->PIN &= ~(1 << pin_num);
  }
}

bool gpio0__get_level(uint8_t pin_num) {
  if (LPC_GPIO0->PIN & (1 << pin_num)) {
    return true;
  } else {
    return false;
  }
}

//------------------------GPIO-2------------------------------------------------
void gpio2__set_as_input(uint8_t pin_num) { LPC_GPIO2->DIR &= ~(1 << pin_num); }

void gpio2__set_as_output(uint8_t pin_num) { LPC_GPIO2->DIR |= (1 << pin_num); }

void gpio2__set_high(uint8_t pin_num) { LPC_GPIO2->PIN |= (1 << pin_num); }

void gpio2__set_low(uint8_t pin_num) { LPC_GPIO2->PIN &= ~(1 << pin_num); }

void gpio2__set(uint8_t pin_num, bool high) {
  if (high == true) {
    LPC_GPIO2->PIN |= (1 << pin_num);
  } else {
    LPC_GPIO2->PIN &= ~(1 << pin_num);
  }
}

bool gpio2__get_level(uint8_t pin_num) {
  if (LPC_GPIO2->PIN & (1 << pin_num)) {
    return true;
  } else {
    return false;
  }
}
