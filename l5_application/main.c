/*
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

// two additional tasks
static void task_one(void *task_parameter);
static void task_two(void *task_parameter);

typedef struct {
  float f1; // 4 bytes
  char c1;  // 1 byte
  float f2;
  char c2;
} __attribute__((packed)) my_s;

int main(void) {
  // create_blinky_tasks();
  // create_uart_task();
  puts("Starting RTOS...Hello World");
  // TODO: Instantiate a struct of type my_s with the name of "s"

    my_s s;
    printf("Size : %d bytes\n"
           "floats 0x%p 0x%p\n"
           "chars  0x%p 0x%p\n",
           sizeof(s), &s.f1, &s.f2, &s.c1, &s.c2);

  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h
  // function, name, stack size, parameter passed, priority, task handle

  xTaskCreate(task_one, "task1", 4096 / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(task_two, "task2", 4096 / sizeof(void *), NULL, 1, NULL);

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

static void create_blinky_tasks(void) {

   // Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   //  Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED

#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    //Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
    //  - During debugging in case system crashes before all output of printf() is sent

    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    // This deposits data to an outgoing queue and doesn't block the CPU
    // Data will be sent later, but this function would return earlier

    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
*/

//--------------Lab FreeRTOS----------------
/*
static void task_one(void *task_parameter) {
  while (true) {
    // Read existing main.c regarding when we should use fprintf(stderr...) in place of printf()
    // For this lab, we will use fprintf(stderr, ...)
    fprintf(stderr, "AAAAAAAAAAAA");

    // Sleep for 100ms
    vTaskDelay(100);
  }
}

static void task_two(void *task_parameter) {
  while (true) {
    fprintf(stderr, "bbbbbbbbbbbb");
    vTaskDelay(100);
  }
}
*/

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------Lab Led Switch Part 0 ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

static const uint32_t pin3 = (1 << 3);

void led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 0) Set the IOCON MUX function(if required) select pins to 000
  // LED0 : P2.3

  LPC_GPIO2->PIN |= pin3;

  // 1) Set the DIR register bit for the LED port pin
  LPC_GPIO2->DIR |= pin3;

  while (true) {
    // 2) Set PIN register bit to 0 to turn ON LED (led may be active low)
    LPC_GPIO2->CLR = pin3;
    vTaskDelay(500);

    // 3) Set PIN register bit to 1 to turn OFF LED
    LPC_GPIO2->SET = pin3;
    vTaskDelay(500);
  }
}

int main(void) {
  // Create FreeRTOS LED task
  xTaskCreate(led_task, "led", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
  return 0;
}

*/
//-----------------Lab LED Switch Part 2--------------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

typedef struct {
  // uint8_t port;

  uint8_t pin;
} port_pin_s;

void led_task(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  const port_pin_s *led = (port_pin_s *)(task_parameter);

  while (true) {
    gpio1__set_high(led->pin);
    vTaskDelay(100);

    gpio1__set_low(led->pin);
    vTaskDelay(100);
  }
}

int main(void) {
  // TODO:
  // Create two tasks using led_task() function
  // Pass each task its own parameter:
  // This is static such that these variables will be allocated in RAM and not go out of scope
  //LEDs P1.26 and P1.24

  static port_pin_s led0 = {26};
  static port_pin_s led1 = {24};

  // &led0 is a task parameter going to led_task
  xTaskCreate(led_task, "led0", 4096 / sizeof(void *), &led0, PRIORITY_LOW, NULL);
  xTaskCreate(led_task, "led1", 4096 / sizeof(void *), &led1, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
  return 0;
}

*/

//-----------------LAB LED Switch Part 3-------------------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

static SemaphoreHandle_t switch_press_indication;

typedef struct {
  // uint8_t port;

  uint8_t pin;
} port_pin_s;

void led_task(void *task_parameter) {
  const port_pin_s *led = (port_pin_s *)(task_parameter);

  while (true) {
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore (signal)
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // TODO: Blink the LED
      gpio1__set_low(led->pin);
      vTaskDelay(100);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
      gpio1__set_high(led->pin);
      vTaskDelay(100);
    }
  }
}

void switch_task(void *task_parameter) {
  const port_pin_s *switch_1 = (port_pin_s *)task_parameter;

  while (true) {
    // TODO: If switch pressed, set the binary semaphore
    if (gpio1__get_level(switch_1->pin)) {
      xSemaphoreGive(switch_press_indication);
    }

    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
    vTaskDelay(100);
  }
}

int main(void) {
  switch_press_indication = xSemaphoreCreateBinary();

  // Hint: Use on-board LEDs first to get this logic to work
  //       After that, you can simply switch these parameters to off-board LED and a switch
  static port_pin_s switch_1 = {15};
  static port_pin_s led = {26};

  xTaskCreate(led_task, "led", 2048 / sizeof(void *), &led, 1, NULL);
  xTaskCreate(switch_task, "switch", 2048 / sizeof(void *), &switch_1, 1, NULL);

  vTaskStartScheduler();
  return 0;
}
*/

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------Lab Interrupts and Binary Semaphores Part 0 ------------------------

/*
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include <stdio.h>

void gpio_interrupt(void);

// Step 1:
void main(void) {
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "unused");

  //Setting P0.30 as input
  LPC_GPIO0->DIR &= ~(1 << 30);

  //configuring registers to falling edge
  LPC_GPIOINT->IO0IntEnF |= (1 << 30);

  //Enabling GPIO interrupt
  NVIC_EnableIRQ(GPIO_IRQn);

  while (1) {
    if (LPC_GPIOINT->IO0IntStatF & (1 << 30)) {
      //Toggle P1.24 LED
      gpio1__set_low(24);
      delay__ms(100);
      gpio1__set_high(24);
    }
  }
}

// Step 2:
void gpio_interrupt(void) {
  // a) Clear Port0/2 interrupt using CLR0 or CLR2 registers
  // b) Use fprintf(stderr) or blink and LED here to test your ISR
  fprintf(stderr, "Interrupt triggered ");
  LPC_GPIOINT->IO0IntClr = (1 << 30);
}
*/

// ------------Lab Interrupts and Binary Semaphores Part 1 ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include <stdio.h>

static SemaphoreHandle_t switch_pressed_signal;

void gpio_interrupt(void);

// WARNING: You can only use printf(stderr, "foo") inside of an ISR
void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  LPC_GPIOINT->IO0IntClr = (1 << 30);
}

void sleep_on_sem_task(void *p) {
  while (1) {
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {
      gpio1__set_low(24);
      vTaskDelay(100);
      gpio1__set_high(24);
    } else {
      puts("Error: Took too long");
    }
  }
}

void main(void) {

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "name");

  switch_pressed_signal = xSemaphoreCreateBinary(); // Create your binary semaphore

  // configure_your_gpio_interrupt();
  // TODO: Setup interrupt by re-using code from Part 0
  LPC_GPIO0->DIR &= ~(1 << 30);
  LPC_GPIOINT->IO0IntEnF |= (1 << 30);
  NVIC_EnableIRQ(GPIO_IRQn); // Enable interrupt gate for the GPIO

  xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
*/
// ------------Lab Interrupts and Binary Semaphores Part 2 ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

void gpio0__interrupt_dispatcher(void);

// Objective of the assignment is to create a clean API to register sub-interrupts like so:
void pin30_isr(void) {
  gpio1__set_low(24);
  delay__ms(100);
  gpio1__set_high(24);
}
void pin29_isr(void) {
  gpio1__set_low(18);
  delay__ms(100);
  gpio1__set_high(18);
}

// Example usage:
void main(void) {
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "name");

  gpio0__attach_interrupt(30, GPIO_INTR__RISING_EDGE, pin30_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, pin29_isr);
}
*/
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------Lab ADC + PWM Part 0 ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "task.h"
#include <stdio.h>

void pwm_task(void *p) {
  pwm1__init_single_edge(1000);

  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCTION_1); //001
  gpio2__set_as_output(0); // output

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  pwm1__set_duty_cycle(PWM1__2_0, 50);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, percent);
    if (++percent > 100) {
      percent = 0;
    }

    vTaskDelay(100);
  }
}

void main(void) {
  xTaskCreate(pwm_task, "pwm", configMINIMAL_STACK_SIZE, NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}
*/

// --------------------Lab ADC + PWM Part 1 ------------------------
/*
#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>

void adc_task(void *p) {
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  // pin_configure_adc_channel_as_io_pin(); // TODO You need to write this function
  LPC_IOCON->P1_31 &= ~((1 << 3) | (1 << 4)); //disabling mode
  LPC_IOCON->P1_31 &= ~(1 << 7); //setting mux
  gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_3);

  while (1) {
    // Get the ADC reading using a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);
    printf("%" PRIu16 "\n", adc_value);
    vTaskDelay(100);
  }
}

void main(void) {
  xTaskCreate(adc_task, "ADC", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
*/
// --------------------Lab ADC + PWM Part 2 ------------------------
/*
#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>

// QUEUE handle
static QueueHandle_t adc_to_pwm_task_queue;

void adc_task(void *p) {
  adc__initialize();
  adc__enable_burst_mode();
  LPC_IOCON->P1_31 &= ~((1 << 3) | (1 << 4) | (1 << 7));
  gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_3);
  gpio1__set_as_output(31);

  while (1) {
    const uint16_t adc_reading = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    vTaskDelay(100);
  }
}

void pwm_task(void *p) {
  pwm1__init_single_edge(1000);
  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCTION_1);
  gpio2__set_as_input(0);              // port 2_0 set as output
  pwm1__set_duty_cycle(PWM1__2_0, 50); // initial 50% duty cycle

  uint8_t percent = 0;
  int adc_reading = 0;

  while (1) {
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      percent = (adc_reading * 100) / 4095;     // formula for setting percent
      pwm1__set_duty_cycle(PWM1__2_0, percent); // set duty cycle using percent
    }
  }
}

void main(void) {
  // Queue will only hold 1 integer
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));

  xTaskCreate(adc_task, "ADC", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(pwm_task, "PWM", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
*/

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------Lab Spi Flash Interface Part 1  ------------------------
/*
#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "ssp2_lab.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>

typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  // uint8_t extended_device_id;
  uint8_t opcode;
} adesto_flash_id_s;

adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};

  adesto_cs();
  data.opcode = ssp2__exchange_byte_new(0x9F);
  data.manufacturer_id = ssp2__exchange_byte_new(0x00);
  data.device_id_1 = ssp2__exchange_byte_new(0x00);
  data.device_id_2 = ssp2__exchange_byte_new(0x00);
  // data.extended_device_id = ssp2__exchange_byte_lab(0xFF);
  adesto_ds();

  return data;
}

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24;
  ssp2__init(spi_clock_mhz);

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();

    fprintf(stderr, "Manufaturer: %X\n Device_1: %X\n Device_2: %X\n ", id.manufacturer_id, id.device_id_1,
            id.device_id_2); // Extended_Device: %X\n , id.extended_device_id
    vTaskDelay(500);
  }
}

void main(void) {
  xTaskCreate(spi_task, "SPI", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
*/
// ------------Lab Spi Flash Interface Part 2  ------------------------
/*
#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "ssp2_lab.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>

static SemaphoreHandle_t ssp_mutex;

typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  // uint8_t extended_device_id;
  uint8_t opcode;
} adesto_flash_id_s;

adesto_flash_id_s ssp2__adesto_read_signature(void) {
  adesto_flash_id_s data = {0};

  adesto_cs();
  data.opcode = ssp2__exchange_byte_new(0x9F);
  data.manufacturer_id = ssp2__exchange_byte_new(0xFF);
  data.device_id_1 = ssp2__exchange_byte_new(0xFF);
  data.device_id_2 = ssp2__exchange_byte_new(0xFF);
  // data.extended_device_id = ssp2__exchange_byte_lab(0xFF);
  adesto_ds();

  return data;
}

void spi_id_verification_task(void *p) {

  const uint32_t spi_clock_mhz = 24;
  ssp2__init(spi_clock_mhz);
  while (1) {
    adesto_flash_id_s id = ssp2__adesto_read_signature();
    if (xSemaphoreTake(ssp_mutex, 100)) {

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (0x1F != id.manufacturer_id) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      } else {
        fprintf(stderr, "Task \n");
        fprintf(stderr, "Manufaturer: %X\n Device_1: %X\n Device_2: %X\n ", id.manufacturer_id, id.device_id_1,
                id.device_id_2);
        vTaskDelay(500);
      }
      xSemaphoreGive(ssp_mutex);
    }
  }
}

void main(void) {
  ssp_mutex = xSemaphoreCreateMutex();

  xTaskCreate(spi_id_verification_task, "SSP1", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(spi_id_verification_task, "SSP2", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}
*/
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------Lab UART Part 1  ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "task.h"
#include "uart_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char input_byte;
    printf("Starting Polled Get:\n");
    uart_lab__polled_get(UART_3, &input_byte);
    // printf("%c\n", input_byte);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    char output_byte = 'A';
    printf("Starting Polled Put:\n");
    uart_lab__polled_put(UART_3, output_byte);
    vTaskDelay(500);
  }
}

void main(void) {
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_3, clock__get_peripheral_clock_hz(), 38400);

  xTaskCreate(uart_read_task, "Read", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "Write", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}
*/
// ------------Lab UART Part 2  ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "task.h"
#include "uart_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char input_byte;
    printf("Starting Polled Get:\n");
    uart_lab__get_char_from_queue(&input_byte, 1000);
    printf("%c\n", input_byte);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    char output_byte = 'A';
    printf("Starting Polled Put:\n");
    uart_lab__polled_put(UART_3, output_byte);
    vTaskDelay(500);
  }
}

void main(void) {
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_3, clock__get_peripheral_clock_hz(), 38400);
  uart__enable_receive_interrupt(UART_3);
  xTaskCreate(uart_read_task, "Read", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "Write", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}
*/
// ------------Lab UART Part 3  ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "task.h"
#include "uart_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void board_1_sender_task(void *p) {
  char number_as_string[16] = {0};

  while (true) {
    const int number = rand();
    sprintf(number_as_string, "%i", number);

    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }

    // printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;
  vTaskDelay(100);

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received: %c\n", byte);

    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      // printf("Received this number from the other board: %s\n", number_as_string);
    } else if (counter < 16) {
      number_as_string[counter] = byte;
      // printf("Arr[%i]: %s \n", counter, number_as_string);
      counter++;
    }
  }
}

void main(void) {
  uart_lab__init(UART_3, clock__get_peripheral_clock_hz(), 38400);
  uart_lab__init(UART_2, clock__get_peripheral_clock_hz(), 38400);
  uart__enable_receive_interrupt(UART_3);
  xTaskCreate(board_1_sender_task, "Sender", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(board_2_receiver_task, "Receiver", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}
*/
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------FreeRTOS Producer Consumer Tasks ------------------------
/*
#include "FreeRTOS.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "task.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

switch_e get_switch_input_from_switch0() {
  gpio1__set_as_input(19);

  if (LPC_GPIO1->PIN & (1 << 19)) {
    return switch__on;
  } else {
    return switch__off;
  }
}

void producer(void *p) {
  while (1) {
    // TODO: Get some input value from your board
    const switch_e switch_value = get_switch_input_from_switch0();
    printf("\n%s(), line %d, before\n", __FUNCTION__, __LINE__);
    xQueueSend(switch_queue, &switch_value, 0); // will switch context to consumer task because higher priority
    printf("\n%s(), line %d, after\n", __FUNCTION__, __LINE__);
    vTaskDelay(1000);
  }
}

void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    printf("\n%s(), line %d, before\n", __FUNCTION__, __LINE__);
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    printf("\n%s(), line %d, after\n", __FUNCTION__, __LINE__);
  }
}

void main(void) {
  xTaskCreate(producer, "Producer", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(consumer, "Consumer", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  vTaskStartScheduler();
}
*/
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------FreeRTOS Watchdogs Part 0------------------------
/*
#include "FreeRTOS.h"
#include "acceleration.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "task.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static QueueHandle_t sensor_queue;
int16_t get_sensor_value(void);
int32_t compute_average(void);
void write_file_using_fatfs_pi(average_value);

void producer(void *task_parameter) {
  while (true) {
    int16_t average_value = compute_average();
    if (xQueueSend(sensor_queue, &average_value, 0)) {
      // printf("sending %d\n", average_value);
      vTaskDelay(100);
    }
  }
}

void consumer(void *task_parameter) {
  int16_t average_value;
  while (true) {
    if (xQueueReceive(sensor_queue, &average_value, portMAX_DELAY)) {
      write_file_using_fatfs_pi(average_value);
    }
  }
}

int16_t get_sensor_value(void) {
  acceleration__axis_data_s new_struct;
  new_struct = acceleration__get_data();
  int16_t sensor_value = new_struct.x;
  return sensor_value;
}

int32_t compute_average(void) {
  int32_t array[100];
  int32_t sum = 0;
  int32_t average = 0;

  for (int i = 0; i < 100; i++) {
    int16_t sensor_value = get_sensor_value();
    array[i] = sensor_value;
  }

  for (int y = 0; y < 100; y++) {
    sum += array[y];
  }

  average = sum / 100;
  return average;
}

void write_file_using_fatfs_pi(average_value) {
  const char *filename = "sensor.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (0x02 | 0x30));

  if (FR_OK == result) {
    char string[64];
    sprintf(string, "Value,%i\n", average_value);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}

void main(void) {
  sensor_queue = xQueueCreate(1, sizeof(int16_t));
  acceleration__init();

  xTaskCreate(producer, "producer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer, "consumer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskStartScheduler();
}
*/
// ------------FreeRTOS watchdogs Part 1------------------------
/*
#include "FreeRTOS.h"
#include "acceleration.h"
#include "board_io.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "task.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static QueueHandle_t sensor_queue;
EventGroupHandle_t event = NULL;
static EventBits_t bits;
int16_t get_sensor_value(void);
int32_t compute_average(void);
void write_file_using_fatfs_pi(average_value);
uint8_t producer_bit = 0x01;
uint8_t consumer_bit = 0x02;

void producer(void *task_parameter) {
  while (true) {
    uint8_t counter = 0;
    if (counter <= 100) {
      int16_t average_value = compute_average(); // get sensor value
      if (xQueueSend(sensor_queue, &average_value, 0)) {
        printf("sending %d\n", average_value);
        vTaskDelay(100);
      }
      counter = 0;
    }
    xEventGroupSetBits(event, producer_bit);
    vTaskDelay(500);
  }
}

void consumer(void *task_parameter) {
  int16_t average_value;
  while (true) {
    if (xQueueReceive(sensor_queue, &average_value, portMAX_DELAY)) {
      uint16_t tick_count = xTaskGetTickCount();
      printf("receiving %d\n", average_value);
      // write_file_using_fatfs_pi(average_value);
    }
    xEventGroupSetBits(event, consumer_bit);
  }
}

int16_t get_sensor_value(void) {
  acceleration__axis_data_s new_struct;
  new_struct = acceleration__get_data();
  int16_t sensor_value = new_struct.x;
  return sensor_value;
}

int32_t compute_average(void) {
  int32_t array[100];
  int32_t sum = 0;
  int32_t average = 0;

  for (int i = 0; i < 100; i++) {
    int16_t sensor_value = get_sensor_value();
    array[i] = sensor_value;
  }

  for (int y = 0; y < 100; y++) {
    sum += array[y];
  }

  average = sum / 100;
  return average;
}

void watchdog_task(void *params) {
  while (1) {
    bits = xEventGroupWaitBits(event, (producer_bit | consumer_bit), pdTRUE, pdTRUE, 2000);
    if (bits == 0x03) {
      printf("Running smoothly...\n");
    }

    if (!(bits & (producer_bit))) {
      if ((!(bits & (consumer_bit)))) {
        // both bits not set
        printf("Producer and Consumer Tasks Failed");
      } else {
        // producer bit not set
        printf("Producer Task Failed");
      }
    } else if (!(bits & (consumer_bit))) {
      // consumer bit not set
      printf("Consumer Task Failed");
    }
  }
}

void main(void) {
  sensor_queue = xQueueCreate(1, sizeof(int16_t));
  acceleration__init();
  event = xEventGroupCreate();

  xTaskCreate(producer, "producer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer, "consumer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(watchdog_task, "watchdog", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}
*/
// ------------FreeRTOS Watchdogs Part 2------------------------
/*
#include "FreeRTOS.h"
#include "acceleration.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static QueueHandle_t sensor_queue;
EventGroupHandle_t event = NULL;
static EventBits_t bits;
int16_t get_sensor_value(void);
int32_t compute_average(void);
void write_file_using_fatfs_pi(average_value);
uint8_t producer_bit = 0x01;
uint8_t consumer_bit = 0x02;

void producer(void *task_parameter) {
  while (true) {
    uint8_t counter = 0;
    if (counter <= 100) {
      int16_t average_value = compute_average(); // get sensor value
      if (xQueueSend(sensor_queue, &average_value, 0)) {
        printf("sending %d\n", average_value);
        vTaskDelay(100);
      }
      counter = 0;
    }
    xEventGroupSetBits(event, producer_bit);
    vTaskDelay(500);
  }
}

void consumer(void *task_parameter) {
  int16_t average_value;
  while (true) {
    if (xQueueReceive(sensor_queue, &average_value, portMAX_DELAY)) {
      uint16_t tick_count = xTaskGetTickCount();
      printf("receiving %d\n", average_value);
      write_file_using_fatfs_pi(average_value, tick_count);
    }
    xEventGroupSetBits(event, consumer_bit);
  }
}

int16_t get_sensor_value(void) {
  acceleration__axis_data_s new_struct;
  new_struct = acceleration__get_data();
  int16_t sensor_value = new_struct.x;
  return sensor_value;
}

int32_t compute_average(void) {
  int32_t array[100];
  int32_t sum = 0;
  int32_t average = 0;

  for (int i = 0; i < 100; i++) {
    int16_t sensor_value = get_sensor_value();
    array[i] = sensor_value;
  }

  for (int y = 0; y < 100; y++) {
    sum += array[y];
  }

  average = sum / 100;
  return average;
}

void watchdog_task(void *params) {
  while (1) {
    bits = xEventGroupWaitBits(event, (producer_bit | consumer_bit), pdTRUE, pdTRUE, 2000);
    if (bits == 0x03) {
      printf("Running smoothly...\n");
    }

    if (!(bits & (producer_bit))) {
      if ((!(bits & (consumer_bit)))) {
        // both bits not set
        printf("Producer and Consumer Tasks Failed");
      } else {
        // producer bit not set
        printf("Producer Task Failed");
      }
    } else if (!(bits & (consumer_bit))) {
      // consumer bit not set
      printf("Consumer Task Failed");
    }
  }
}

void write_file_using_fatfs_pi(average_value, tick_count) {
  const char *filename = "sensor.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (0x02 | 0x30));

  if (FR_OK == result) {
    char string[64];
    sprintf(string, "Value,%i , %i\n", average_value, tick_count);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}

void main(void) {
  sj2_cli__init();
  i2c2_slave_init(0x86);
  sensor_queue = xQueueCreate(1, sizeof(int16_t));
  acceleration__init();
  event = xEventGroupCreate();

  xTaskCreate(producer, "producer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer, "consumer", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(watchdog_task, "watchdog", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}
*/

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------I2C Slave Lab ------------------------
/*
#include "FreeRTOS.h"
#include "acceleration.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  sj2_cli__init();
  i2c1__slave_init(0x86);
  vTaskStartScheduler();
  return 0;
}
*/
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------
// ------------MP3 Project ------------------------

#include "FreeRTOS.h"
#include "acceleration.h"
#include "adc.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "mp3_decoder.h"
#include "mp3_lcd.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "song_display.h"
#include "ssp2.h"
#include "ssp2_lab.h"
#include "task.h"
#include "uart.h"
#include "uart_lab.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

QueueHandle_t Q_trackname;
QueueHandle_t Q_songdata;
typedef char trackname_t[64];
typedef char songdata_t[512];

TaskHandle_t player_handle;

SemaphoreHandle_t next_song;
SemaphoreHandle_t previous_song;
SemaphoreHandle_t play_pause;
SemaphoreHandle_t choice;

static void interrupts();
static void next_song_ISR();
static void previous_song_ISR();
static void play_pause_ISR();
static void play_pause_Button();

typedef struct {
  char Tag[3];
  char Title[30];
  char Artist[30];
  char Album[30];
  char Year[4];
  uint8_t genre;
} mp3_meta;

void print_metadata(char *meta) {
  uint8_t title_counter = 0;
  uint8_t artist_counter = 0;
  uint8_t album_counter = 0;
  uint8_t year_counter = 0;
  mp3_meta song_meta = {0};
  for (int i = 0; i < 128; i++) {
    if ((((int)(meta[i]) > 47) && ((int)(meta[i]) < 58)) || (((int)(meta[i]) > 64) && ((int)(meta[i]) < 91)) ||
        (((int)(meta[i]) > 96) && ((int)(meta[i]) < 123)) || ((int)(meta[i])) == 32) {
      char character = (int)(meta[i]);
      if (i < 3) {
        song_meta.Tag[i] = character; // Tag
      } else if (i > 2 && i < 33) {
        song_meta.Title[title_counter++] = character; // Song Name
      } else if (i > 32 && i < 63) {
        song_meta.Artist[artist_counter++] = character; // Artist
      } else if (i > 62 && i < 93) {
        song_meta.Album[album_counter++] = character; // Album
      } else if (i > 92 && i < 97) {
        song_meta.Year[year_counter++] = character; // Year
      }
    }
  }

  lcd_print(song_meta.Title, page_0, init);
  lcd_print(song_meta.Artist, page_3, 0);
  lcd_print(song_meta.Album, page_5, 0);
  // lcd_print(song_meta.Year, page_7, 0);
}

static void mp3_reader_task(void *p) {
  trackname_t song_name;
  songdata_t byte512;
  UINT br;

  while (1) {
    xQueueReceive(Q_trackname, song_name, portMAX_DELAY);

    lcd_print(song_name, page_0, init);
    horizontal_addressing(page_0, page_0);

    const char *file_name = song_name;
    FIL file;
    FRESULT result = f_open(&file, file_name, (FA_READ));

    static char meta_128[128];

    f_lseek(&file, f_size(&file) - (sizeof(char) * 128));
    f_read(&file, meta_128, sizeof(meta_128), &br);
    print_metadata(meta_128);
    f_lseek(&file, 0);

    if (FR_OK == result) {
      f_read(&file, byte512, sizeof(byte512), &br);
      while (br != 0) {
        f_read(&file, byte512, sizeof(byte512), &br);
        xQueueSend(Q_songdata, byte512, portMAX_DELAY);
        if (uxQueueMessagesWaiting(Q_trackname)) {
          printf("New song request\n");
          break;
        }
      }
      f_close(&file);
    } else if (br == 0) {
      xSemaphoreGive(choice);
      xSemaphoreGive(next_song);
      printf("Read byte is 0\n");
    } else {
      printf("Failed to open mp3 object_file \n");
    }
  }
}

static void mp3_player_task(void *p) {
  int counter = 1;
  while (1) {
    songdata_t byte512;
    xQueueReceive(Q_songdata, byte512, portMAX_DELAY);
    for (int i = 0; i < 512; i++) {
      while (!get_DREQ_high()) {
        vTaskDelay(1);
      }
      send_data_decoder(byte512[i]);
    }
    // printf("Transmitting: %d (times)\n", counter);
    counter++;
  }
}

static void mp3_song_controller_task(void *p) {
  volatile size_t index = 0;
  song_list_display();
  while (1) {
    if (xSemaphoreTake(choice, portMAX_DELAY)) {
      if (xSemaphoreTake(next_song, 10)) {
        if (index >= num_songs()) {
          index = 0;
        }
        xQueueSend(Q_trackname, song_list_name(index), portMAX_DELAY);
        index++;
      } else if (xSemaphoreTake(previous_song, 10)) {
        if (index == 0) {
          index = num_songs();
        }
        xQueueSend(Q_trackname, song_list_name(index), portMAX_DELAY);
        index--;
      }
    }
    vTaskDelay(100);
  }
}

int main(void) {
  // Queues Create
  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, sizeof(songdata_t));
  next_song = xSemaphoreCreateBinary();
  previous_song = xSemaphoreCreateBinary();
  play_pause = xSemaphoreCreateBinary();
  choice = xSemaphoreCreateBinary();

  // Inits
  interrupts();
  decoder_initialize();
  sj2_cli__init();
  song_list_display();
  song_counter();

  // Task Create
  xTaskCreate(mp3_reader_task, "reader", (2048 * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(mp3_player_task, "player", (2048 * 4) / sizeof(void *), NULL, 1, &player_handle);
  xTaskCreate(mp3_song_controller_task, "song_controller", (2048 * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(play_pause_Button, "play_pause", (2048) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
  return 0;
}

static void interrupts() {
  LPC_IOCON->P0_25 &= ~(0b111);
  gpio0__set_as_input(25);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "INTR Port 0");
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, play_pause_ISR);
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, next_song_ISR);
  gpio0__attach_interrupt(25, GPIO_INTR__RISING_EDGE, previous_song_ISR);
}

static void next_song_ISR() {
  xSemaphoreGiveFromISR(choice, NULL);
  xSemaphoreGiveFromISR(next_song, NULL);
}

static void play_pause_ISR() { xSemaphoreGiveFromISR(play_pause, NULL); }

static void previous_song_ISR() {
  xSemaphoreGiveFromISR(choice, NULL);
  xSemaphoreGiveFromISR(previous_song, NULL);
}

volatile bool pause = true;

static void play_pause_Button() {
  while (1) {
    if (xSemaphoreTake(play_pause, portMAX_DELAY)) {
      if (pause) {
        vTaskSuspend(player_handle);
        vTaskDelay(300);
        pause = false;
      } else {
        vTaskResume(player_handle);
        vTaskDelay(300);
        pause = true;
      }
    }
  }
}
