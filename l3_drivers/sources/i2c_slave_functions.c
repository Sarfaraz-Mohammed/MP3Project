#include "i2c_slave_functions.h"

bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  // TODO: Read the data from slave_memory[memory_index] to *memory pointer
  // TODO: return true if all is well (memory index is within bounds)
  if (memory_index < 256) {
    *memory = slave_memory[memory_index];
    return true;
  } else {
    printf("Memory is out of range\n");
    return false;
  }
}

bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
  // TODO: Write the memory_value at slave_memory[memory_index]
  // TODO: return true if memory_index is within bounds
  if (memory_index < 256) {
    slave_memory[memory_index] = memory_value;
    return true;
  } else {
    printf("Memory is out of range\n");
    return false;
  }
}