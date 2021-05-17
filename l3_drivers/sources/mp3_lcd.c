#include "mp3_lcd.h"
#include "clock.h"

// Char array
static myChar char_print[127];

void lcd_CS() {
  gpio1__set_as_output(22);
  gpio1__set_low(22);
}
void lcd_DS() {
  gpio1__set_as_output(22);
  gpio1__set_high(22);
}

// Congifure Pins of SSP1 for LCD
void config_lcd_pins() {
  gpio__construct_with_function(0, 7, GPIO__FUNCTION_2);
  gpio0__set_as_output(7);
  gpio__construct_with_function(0, 9, GPIO__FUNCTION_2);
  gpio0__set_as_output(9);
  gpio__construct_with_function(1, 25, GPIO__FUNCITON_0_IO_PIN);
  gpio1__set_as_output(25);
}

// Command Bus
void lcd_cs_bus() { gpio1__set_low(25); }
void lcd_ds_bus() { gpio1__set_high(25); }

// SSP1 Initalization
void mp3_lcd_initialization() {
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP1);
  LPC_SSP1->CR0 = 7;        // 8 Bit transfer
  LPC_SSP1->CR1 = (1 << 1); // Enable SSP

  uint32_t lcd_clock = 8 * 1000 * 1000;
  const uint32_t CPU_clock = clock__get_core_clock_hz();
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_clock / divider) <= lcd_clock) {
      break;
    }
    LPC_SSP1->CPSR = divider;
  }
}

// Transfer Data LCD
void lcd_transfer_data(uint8_t transfer) {
  LPC_SSP1->DR = transfer;
  while (LPC_SSP1->SR & (1 << 4)) {
    ; // Wait
  }
}

// LCD initialize
void lcd_init() {

  lcd_cs_bus();

  // Turn off
  lcd_transfer_data(0xAE);

  // Display Clock
  lcd_transfer_data(0xD5);
  lcd_transfer_data(0x80);

  // Multiplex
  lcd_transfer_data(0xA8);
  lcd_transfer_data(0x3F);

  // Offset
  lcd_transfer_data(0xD3);
  lcd_transfer_data(0x00);

  // Start
  lcd_transfer_data(0x40);

  // Charge
  lcd_transfer_data(0x8D);
  lcd_transfer_data(0x14);

  // Display seg
  lcd_transfer_data(0xA1);

  // COM
  lcd_transfer_data(0xC8);

  // Pins
  lcd_transfer_data(0xDA);
  lcd_transfer_data(0x12);

  // CR
  lcd_transfer_data(0x81);
  lcd_transfer_data(0xCF);

  // Pre charge
  lcd_transfer_data(0xD9);
  lcd_transfer_data(0xF1);

  // Vcomh
  lcd_transfer_data(0xDB);
  lcd_transfer_data(0x40);

  // Display
  lcd_transfer_data(0xA4);

  // Color
  lcd_transfer_data(0xA6);

  // On
  lcd_transfer_data(0xAF);
}

// LCD On
void turn_on_lcd() {
  config_lcd_pins();
  mp3_lcd_initialization();

  lcd_CS();

  lcd_init();

  lcd_fill();
  lcd_update();
  lcd_clear();
  lcd_update();

  /* Print ("CMPE") */
  next_line(0);
  char_C();
  char_M();
  char_P();
  char_E();

  lcd_DS();
  printf("LCD Should Turn-Successfully\n");
}

void horizontal_addressing() {
  lcd_cs_bus();
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x21);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x22);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x07);
}

void vertical_addressing() {
  lcd_cs_bus();
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x21);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x22);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x07);
}

void horizontal_scrolling(page_address firstp, page_address lastp) {
  lcd_CS();
  {
    lcd_cs_bus();
    lcd_transfer_data(0x26);
    lcd_transfer_data(0x00);
    lcd_transfer_data(0x00 | firstp);
    lcd_transfer_data(0x05);
    lcd_transfer_data(0x00 | lastp);
    lcd_transfer_data(0x00);
    lcd_transfer_data(0xFF);
    lcd_transfer_data(0x2F);
  }
  lcd_DS();
}

void next_line(uint8_t address) {
  lcd_cs_bus();
  lcd_transfer_data(0xB0 | address);
  uint8_t segment = 0x00;
  uint8_t column = 0x10;
  lcd_transfer_data(segment);
  lcd_transfer_data(column);
  lcd_ds_bus();
}

// Show char
void display_char(char *string) {
  lcd_CS();
  lcd_ds_bus();
  for (int i = 0; i < strlen(string); i++) {
    myChar lcd_display = char_print[(int)(string[i])];
    lcd_display();
  }
}

// Print to LCD
void lcd_print(char *text, uint8_t page, multiple_line state) {

  if (state) {
    config_lcd_pins();
    mp3_lcd_initialization();
    char_table();
    lcd_CS();
    lcd_init();
    lcd_clear();
    lcd_update();
    next_line(page);
    display_char(text);
    lcd_DS();
  } else {
    lcd_CS();
    next_line(page);
    display_char(text);
    lcd_DS();
  }
}

void lcd_clear() {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      bitmap_[row][column] = 0x00;
    }
  }
}

void lcd_fill() {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      bitmap_[row][column] = 0xFF;
    }
  }
}

void lcd_update() {
  horizontal_addressing();
  // vertical_addr_mode();
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      lcd_ds_bus();
      lcd_transfer_data(bitmap_[row][column]);
    }
  }
}

// Char Table
void char_table() {
  char_print[(int)'A'] = char_A;
  char_print[(int)'B'] = char_B;
  char_print[(int)'C'] = char_C;
  char_print[(int)'D'] = char_D;
  char_print[(int)'E'] = char_E;
  char_print[(int)'F'] = char_F;
  char_print[(int)'G'] = char_G;
  char_print[(int)'H'] = char_H;
  char_print[(int)'I'] = char_I;
  char_print[(int)'J'] = char_J;
  char_print[(int)'K'] = char_K;
  char_print[(int)'L'] = char_L;
  char_print[(int)'M'] = char_M;
  char_print[(int)'N'] = char_N;
  char_print[(int)'O'] = char_O;
  char_print[(int)'P'] = char_P;
  char_print[(int)'P'] = char_P;
  char_print[(int)'Q'] = char_Q;
  char_print[(int)'R'] = char_R;
  char_print[(int)'S'] = char_S;
  char_print[(int)'T'] = char_T;
  char_print[(int)'U'] = char_U;
  char_print[(int)'V'] = char_V;
  char_print[(int)'W'] = char_W;
  char_print[(int)'X'] = char_X;
  char_print[(int)'Y'] = char_Y;
  char_print[(int)'Z'] = char_Z;
  char_print[(int)'a'] = char_a;
  char_print[(int)'b'] = char_b;
  char_print[(int)'c'] = char_c;
  char_print[(int)'d'] = char_d;
  char_print[(int)'e'] = char_e;
  char_print[(int)'f'] = char_f;
  char_print[(int)'g'] = char_g;
  char_print[(int)'h'] = char_h;
  char_print[(int)'i'] = char_i;
  char_print[(int)'j'] = char_j;
  char_print[(int)'k'] = char_k;
  char_print[(int)'l'] = char_l;
  char_print[(int)'m'] = char_m;
  char_print[(int)'n'] = char_n;
  char_print[(int)'o'] = char_o;
  char_print[(int)'p'] = char_p;
  char_print[(int)'q'] = char_q;
  char_print[(int)'r'] = char_r;
  char_print[(int)'s'] = char_s;
  char_print[(int)'t'] = char_t;
  char_print[(int)'u'] = char_u;
  char_print[(int)'v'] = char_v;
  char_print[(int)'w'] = char_w;
  char_print[(int)'x'] = char_x;
  char_print[(int)'y'] = char_y;
  char_print[(int)'z'] = char_z;

  char_print[(int)'0'] = char_0;
  char_print[(int)'1'] = char_1;
  char_print[(int)'2'] = char_2;
  char_print[(int)'3'] = char_3;
  char_print[(int)'4'] = char_4;
  char_print[(int)'5'] = char_5;
  char_print[(int)'6'] = char_6;
  char_print[(int)'7'] = char_7;
  char_print[(int)'8'] = char_8;
  char_print[(int)'9'] = char_9;

  char_print[(int)'"'] = char_dquote;
  char_print[(int)'\''] = char_squote;
  char_print[(int)','] = char_comma;
  char_print[(int)'?'] = char_qmark;
  char_print[(int)'!'] = char_excl;
  char_print[(int)'@'] = char_at;
  char_print[(int)'_'] = char_undersc;
  char_print[(int)'*'] = char_star;
  char_print[(int)'#'] = char_hash;
  char_print[(int)'%'] = char_percent;

  char_print[(int)'&'] = char_amper;
  char_print[(int)'('] = char_parenthL;
  char_print[(int)')'] = char_parenthR;
  char_print[(int)'+'] = char_plus;
  char_print[(int)'-'] = char_minus;
  char_print[(int)'/'] = char_div;
  char_print[(int)':'] = char_colon;
  char_print[(int)';'] = char_scolon;
  char_print[(int)'<'] = char_less;
  char_print[(int)'>'] = char_greater;

  char_print[(int)'='] = char_equal;
  char_print[(int)'['] = char_bracketL;
  char_print[(int)'\\'] = char_backslash;
  char_print[(int)']'] = char_bracketR;
  char_print[(int)'^'] = char_caret;
  char_print[(int)'`'] = char_bquote;
  char_print[(int)'{'] = char_braceL;
  char_print[(int)'}'] = char_braceR;
  char_print[(int)'|'] = char_bar;
  char_print[(int)'~'] = char_tilde;

  char_print[(int)' '] = char_space;
  char_print[(int)'.'] = char_period;
  char_print[(int)'$'] = char_dollar;
}

void char_A() {
  lcd_transfer_data(0x7E);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x7E);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_B() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x36);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_C() {
  lcd_transfer_data(0x3E);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x22);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_D() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x3E);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_E() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_F() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_G() {
  lcd_transfer_data(0x3E);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x3A);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_H() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_I() {
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_J() {
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x3F);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_K() {
  lcd_transfer_data(0x7f);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x14);
  lcd_transfer_data(0x22);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_L() {
  lcd_transfer_data(0x7f);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_M() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x02);
  lcd_transfer_data(0x0C);
  lcd_transfer_data(0x02);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_N() {
  lcd_transfer_data(0x7f);
  lcd_transfer_data(0x02);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x7f);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_O() {
  lcd_transfer_data(0x3e);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x3e);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_P() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x06);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_Q() {
  lcd_transfer_data(0x3E);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x51);
  lcd_transfer_data(0x21);
  lcd_transfer_data(0x5E);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_R() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x19);
  lcd_transfer_data(0x29);
  lcd_transfer_data(0x46);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_S() {
  lcd_transfer_data(0x26);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x32);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_T() {
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_U() {
  lcd_transfer_data(0x3F);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x3F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_V() {
  lcd_transfer_data(0x1F);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x1F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_W() {
  lcd_transfer_data(0x3F);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x3F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_X() {
  lcd_transfer_data(0x63);
  lcd_transfer_data(0x14);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x14);
  lcd_transfer_data(0x63);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_Y() {
  lcd_transfer_data(0x07);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x70);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x07);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_Z() {
  lcd_transfer_data(0x61);
  lcd_transfer_data(0x51);
  lcd_transfer_data(0x49);
  lcd_transfer_data(0x45);
  lcd_transfer_data(0x43);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_a() {
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x78);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_b() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_c() {
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_d() {
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_e() {
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x18);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_f() {
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x7E);
  lcd_transfer_data(0x09);
  lcd_transfer_data(0x01);
  lcd_transfer_data(0x02);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_g() {
  lcd_transfer_data(0x18);
  lcd_transfer_data(0xA4);
  lcd_transfer_data(0xA4);
  lcd_transfer_data(0xA4);
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_h() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x78);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_i() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x7D);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_j() {
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x80);
  lcd_transfer_data(0x80);
  lcd_transfer_data(0x84);
  lcd_transfer_data(0x7D);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_k() {
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x10);
  lcd_transfer_data(0x28);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_l() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x41);
  lcd_transfer_data(0x7F);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_m() {
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x18);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x78);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_n() {
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x78);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_o() {
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x38);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_p() {
  lcd_transfer_data(0xFC);
  lcd_transfer_data(0x24);
  lcd_transfer_data(0x24);
  lcd_transfer_data(0x24);
  lcd_transfer_data(0x18);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_q() {
  lcd_transfer_data(0x18);
  lcd_transfer_data(0x24);
  lcd_transfer_data(0x24);
  lcd_transfer_data(0x28);
  lcd_transfer_data(0xFC);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_r() {
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x08);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_s() {
  lcd_transfer_data(0x48);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_t() {
  lcd_transfer_data(0x04);
  lcd_transfer_data(0x3E);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_u() {
  lcd_transfer_data(0x3C);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_v() {
  lcd_transfer_data(0x1C);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x20);
  lcd_transfer_data(0x1C);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

void char_w() {
  lcd_transfer_data(0x3c);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x30);
  lcd_transfer_data(0x40);
  lcd_transfer_data(0x3C);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_x() {
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x28);
  lcd_transfer_data(0x10);
  lcd_transfer_data(0x28);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_y() {
  lcd_transfer_data(0x9C);
  lcd_transfer_data(0xA0);
  lcd_transfer_data(0xA0);
  lcd_transfer_data(0xA0);
  lcd_transfer_data(0x7C);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_z() {
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x64);
  lcd_transfer_data(0x54);
  lcd_transfer_data(0x4C);
  lcd_transfer_data(0x44);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}

/*Danish's contribution*/
void char_0() {
  lcd_transfer_data(0b00111110);
  lcd_transfer_data(0b01010001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01000101);
  lcd_transfer_data(0b00111110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_1() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01000010);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0b01000000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_2() {
  lcd_transfer_data(0b01000010);
  lcd_transfer_data(0b01100001);
  lcd_transfer_data(0b01010001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01000110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_3() {
  lcd_transfer_data(0b00100010);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_4() {
  lcd_transfer_data(0b00011000);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00010010);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0b00010000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_5() {
  lcd_transfer_data(0b00100111);
  lcd_transfer_data(0b01000101);
  lcd_transfer_data(0b01000101);
  lcd_transfer_data(0b01000101);
  lcd_transfer_data(0b00111001);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_6() {
  lcd_transfer_data(0b00111100);
  lcd_transfer_data(0b01001010);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b00110000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_7() {
  lcd_transfer_data(0b00000001);
  lcd_transfer_data(0b01110001);
  lcd_transfer_data(0b00001001);
  lcd_transfer_data(0b00000101);
  lcd_transfer_data(0b00000011);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_8() {
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_9() {
  lcd_transfer_data(0b00000110);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b00101001);
  lcd_transfer_data(0b00011110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_dquote() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00000111);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00000111);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_squote() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00000101);
  lcd_transfer_data(0b00000011);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_comma() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b10100000);
  lcd_transfer_data(0b01100000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_qmark() {
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0b00000001);
  lcd_transfer_data(0b01010001);
  lcd_transfer_data(0b00001001);
  lcd_transfer_data(0b00000110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_excl() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01011111);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_at() {
  lcd_transfer_data(0b00110010);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01111001);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b00111110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_undersc() {
  lcd_transfer_data(0b10000000);
  lcd_transfer_data(0b10000000);
  lcd_transfer_data(0b10000000);
  lcd_transfer_data(0b10000000);
  lcd_transfer_data(0b10000000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_star() {
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00111110);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_hash() {
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_percent() {
  lcd_transfer_data(0b00100011);
  lcd_transfer_data(0b00010011);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b01100100);
  lcd_transfer_data(0b01100010);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_amper() {
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0b01001001);
  lcd_transfer_data(0b01010101);
  lcd_transfer_data(0b00100010);
  lcd_transfer_data(0b01010000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_parenthL() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00011100);
  lcd_transfer_data(0b00100010);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_parenthR() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b00100010);
  lcd_transfer_data(0b00011100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_plus() {
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00111110);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_minus() {
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_div() {
  lcd_transfer_data(0b00100000);
  lcd_transfer_data(0b00010000);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_colon() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_scolon() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01010110);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_less() {
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00100010);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_greater() {
  lcd_transfer_data(0b10000010);
  lcd_transfer_data(0b01000100);
  lcd_transfer_data(0b00101000);
  lcd_transfer_data(0b00010000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_equal() {
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0b00010100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_bracketL() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_backslash() {
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00010000);
  lcd_transfer_data(0b00100000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_bracketR() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_caret() {
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0b00000001);
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_bquote() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00000001);
  lcd_transfer_data(0b00000010);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_braceL() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_braceR() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01000001);
  lcd_transfer_data(0b00110110);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_bar() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01111111);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_tilde() {
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0b00001000);
  lcd_transfer_data(0b00000100);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_space() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_period() {
  lcd_transfer_data(0x00);
  lcd_transfer_data(0b01100000);
  lcd_transfer_data(0b01100000);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}
void char_dollar() {
  lcd_transfer_data(0b00100100);
  lcd_transfer_data(0b00101010);
  lcd_transfer_data(0b01101011);
  lcd_transfer_data(0b00101010);
  lcd_transfer_data(0b00010010);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
  lcd_transfer_data(0x00);
}