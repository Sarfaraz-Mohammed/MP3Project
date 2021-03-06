#pragma once

#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  page_0,
  page_1,
  page_2,
  page_3,
  page_4,
  page_5,
  page_6,
  page_7,
} page_address;

typedef enum {
  not_init,
  init,
} multiple_line;

// Height x Width Pixels
uint8_t bitmap_[8][128];

void lcd_CS();
void lcd_DS();
void config_lcd_pins();
void lcd_cs_bus();
void lcd_ds_bus();

void mp3_lcd_initialization();
void lcd_transfer_data(uint8_t transfer);
void lcd_init();
void turn_on_lcd();

void horizontal_addressing();
void vertical_addressing();
void horizontal_scrolling(page_address firstp, page_address lastp);
void next_line(uint8_t address);
void lcd_print(char *text, uint8_t page, multiple_line state);
void lcd_clear();
void lcd_fill();
void lcd_update();

typedef void (*myChar)(void);

void char_table();
void display_char(char *string);

void char_A();
void char_B();
void char_C();
void char_D();
void char_E();
void char_F();
void char_G();
void char_H();
void char_I();
void char_J();
void char_K();
void char_L();
void char_M();
void char_N();
void char_O();
void char_P();
void char_Q();
void char_R();
void char_S();
void char_T();
void char_U();
void char_V();
void char_W();
void char_X();
void char_Y();
void char_Z();

void char_a();
void char_b();
void char_c();
void char_d();
void char_e();
void char_f();
void char_g();
void char_h();
void char_i();
void char_j();
void char_k();
void char_l();
void char_m();
void char_n();
void char_o();
void char_p();
void char_q();
void char_r();
void char_s();
void char_t();
void char_u();
void char_v();
void char_w();
void char_x();
void char_y();
void char_z();

void char_0();
void char_1();
void char_2();
void char_3();
void char_4();
void char_5();
void char_6();
void char_7();
void char_8();
void char_9();

void char_dquote();
void char_squote();
void char_comma();
void char_qmark();
void char_excl();
void char_at();
void char_undersc();
void char_star();
void char_hash();
void char_percent();
void char_amper();
void char_parenthL();
void char_parenthR();
void char_plus();
void char_minus();
void char_div();
void char_colon();
void char_scolon();
void char_less();
void char_greater();
void char_equal();
void char_bracketL();
void char_backslash();
void char_bracketR();
void char_caret();
void char_bquote();
void char_braceL();
void char_braceR();
void char_bar();
void char_tilde();
void char_space();
void char_period();
void char_dollar();