/*
Ver 2.0
menu com oled display
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <EEPROM.h>
//#include "steppers.h"

#define i2c_display_Address 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1
#define WHITE SH110X_WHITE
#define BLACK SH110X_BLACK

#define ROT_A D8
#define SELECT 3 //RX
#define ROT_B D7

#define RATE_ROTATION_MS 1006 //mudar

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 'eqmount_logo_clean', 128x64px
const unsigned char bitmap_eqmount_logo [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf8, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x01, 0xff, 0xc0, 0x00, 0x3c, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x3f, 0xf8, 0x1f, 0xf0, 0x00, 0x00, 0x0e, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x06, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0f, 0xff, 0x07, 0xfc, 0x00, 0x06, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x06, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0xff, 0x01, 0xf8, 0x07, 0x80, 0x0e, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xef, 0xce, 0x00, 0xe3, 0xc1, 0xc0, 0x0c, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x1c, 0x00, 0x7f, 0xf1, 0xc0, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x38, 0x00, 0x3f, 0xf1, 0x80, 0x70, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xef, 0xf8, 0x00, 0x3f, 0xf3, 0x80, 0xe0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x87, 0xf8, 0x00, 0x3f, 0xf7, 0x01, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0xf8, 0x00, 0x3f, 0xfe, 0x07, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3e, 0x7f, 0x00, 0x3e, 0x7e, 0x1e, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x79, 0xff, 0xc0, 0x39, 0xff, 0x7c, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xe7, 0xfd, 0xfc, 0x3f, 0xc3, 0xf0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01, 0xcf, 0xfc, 0x7f, 0xff, 0x07, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x03, 0x9f, 0xfe, 0x0f, 0xfc, 0x1f, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x03, 0x1f, 0xff, 0x03, 0xfe, 0x7c, 0x70, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x03, 0x0f, 0xf7, 0xff, 0xdf, 0xff, 0xf0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x03, 0x80, 0x07, 0xff, 0x1f, 0xff, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0xff, 0xff, 0xba, 0xfc, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xff, 0xf0, 0x0f, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xfc, 0x3f, 0xf8, 0x00, 0xff, 0xf0, 0x7f, 0xfe, 0x3b, 0x80, 0x0c, 0xff, 0xfc, 0xff, 0xfc, 
	0xff, 0xfc, 0x7f, 0xfc, 0x00, 0xff, 0xf8, 0x7f, 0xff, 0x39, 0xc0, 0x1c, 0xff, 0xfd, 0xff, 0xfc, 
	0xe0, 0x00, 0xe0, 0x1e, 0x00, 0xc0, 0x3c, 0x70, 0x03, 0x39, 0xc0, 0x18, 0xc0, 0x01, 0xc0, 0x0e, 
	0xc0, 0x01, 0xc0, 0x06, 0x00, 0xc0, 0x0e, 0x70, 0x03, 0xb8, 0xe0, 0x38, 0xc0, 0x01, 0xc0, 0x0e, 
	0xc0, 0x01, 0xc0, 0x07, 0x00, 0xc0, 0x0e, 0x70, 0x03, 0xb8, 0xe0, 0x30, 0xc0, 0x01, 0xc0, 0x0e, 
	0xc0, 0x01, 0xc0, 0x07, 0x00, 0xc0, 0x0e, 0x70, 0x03, 0x38, 0x70, 0x70, 0xc0, 0x01, 0xc0, 0x0e, 
	0xcf, 0xf1, 0xc0, 0x07, 0x00, 0xc0, 0x06, 0x70, 0x07, 0x38, 0x70, 0x60, 0xcf, 0xf1, 0xc0, 0x0e, 
	0xcf, 0xf9, 0xc0, 0x07, 0x00, 0xc0, 0x06, 0x77, 0xff, 0x38, 0x38, 0xe0, 0xdf, 0xf1, 0xcf, 0xfc, 
	0xcf, 0xf1, 0xc0, 0x07, 0x00, 0xc0, 0x06, 0x77, 0xfe, 0x38, 0x38, 0xc0, 0xcf, 0xf1, 0xdf, 0xf8, 
	0xc0, 0x01, 0xc0, 0x07, 0x00, 0xc0, 0x0e, 0x77, 0xf8, 0x38, 0x1d, 0xc0, 0xc0, 0x01, 0xdf, 0xf0, 
	0xc0, 0x01, 0xc0, 0x07, 0x00, 0xc0, 0x0e, 0x70, 0xf0, 0x38, 0x1f, 0x80, 0xc0, 0x01, 0xc3, 0xc0, 
	0xc0, 0x00, 0xe0, 0x0e, 0x00, 0xc0, 0x1c, 0x70, 0x78, 0x38, 0x0f, 0x80, 0xc0, 0x01, 0xc1, 0xe0, 
	0xe0, 0x00, 0xf0, 0x1e, 0x00, 0xe0, 0x7c, 0x70, 0x3c, 0x38, 0x0f, 0x80, 0xe0, 0x01, 0xc0, 0xf0, 
	0x7f, 0xfc, 0x7f, 0xfc, 0x00, 0xff, 0xf8, 0x70, 0x1e, 0x38, 0x07, 0x00, 0xff, 0xfd, 0xc0, 0x3c, 
	0x3f, 0xfc, 0x3f, 0xf8, 0x00, 0xff, 0xe0, 0x70, 0x0f, 0x38, 0x07, 0x00, 0x7f, 0xfc, 0xc0, 0x1c, 
	0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char PROGMEM arrow[] = {B00000, B10000, B11000, B11100, B11000, B10000, B00000, B00000}; //setinha
const unsigned char PROGMEM saving[] = {B00000, B11110, B11111, B10001, B10001, B11111, B11111, B00000}; //card
const unsigned char PROGMEM lock[] = {B01110, B10001, B10001, B11111, B11011, B11011, B11111, B00000}; //cadeado
const unsigned char PROGMEM done[] = {B00000, B00000, B00001, B00011, B10110, B11100, B01000, B00000}; //ok
const unsigned char PROGMEM full_block[] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111}; //referencia pro load
const unsigned char PROGMEM dynamic_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};

//menu options (limited to given size)
enum menu_option{
    DEC_           = 0,
    RA             = 1,
    Hemisphere     = 2,
    automatic_mode = 3,
    brilho_tela    = 4,
    tempo_tela     = 5,
    save_configs   = 6
};
const uint8_t menu_size = 7;
const char* menu_str[] = {
    "DEC",
    "R.A",
    "Hemisferio", 
    "Modo automatico", 
    "Brilho da tela", 
    "Luz da tela",
    "Salvar configs"
};
unsigned short int menu_op_value[menu_size] = {0};

unsigned short int value_preview = 0;
unsigned short int lock_value;

bool on_menu = 1;
bool auto_mode = 0;
uint8_t arrow_row = 0;
uint8_t menu_top_row = 0;
uint8_t current_selection;
uint8_t current_text_row;

unsigned long int action_last_time = 0;
unsigned long int ra_last_time = 0;
unsigned long int time_now = 0;

void IRAM_ATTR rotary_IRQ();

void setup()    {
    delay(250); //estabilizar a tensão

    //data recover
    EEPROM.begin(menu_size);
    int stored_brightness = EEPROM.read(0);
    int stored_light_time = EEPROM.read(1);
    int stored_hemisphere = EEPROM.read(2);
    EEPROM.end();
    menu_op_value[brilho_tela] = stored_brightness; 
    menu_op_value[Hemisphere]  = stored_hemisphere;
    menu_op_value[tempo_tela]  = stored_light_time;


    display.begin(i2c_display_Address, true);
    display.clearDisplay();
    display.drawBitmap(0, 0, bitmap_eqmount_logo, 128, 64, WHITE); //logo show-off
    display.display();
    delay(2500);

    display.clearDisplay();
    display.setContrast(127); // dim display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(21, 5);
    display.println("Iniciando Menu");
    display.display();

    
    //rotary encoder
    pinMode(ROT_A, INPUT);
    pinMode(ROT_B, INPUT);
    pinMode(SELECT, INPUT);
    attachInterrupt(digitalPinToInterrupt(ROT_A), rotary_IRQ, RISING);
    

    //fake load suave 
    display.drawRoundRect(7, 32, 120, 8, 3, 1);
    for (uint8_t p = 7; p < 120; p++)    {
        display.fillRoundRect(7, 32, p, 8, 3, 1);
        display.display(); 
        delay(1300/(120));    
    }
    display.clearDisplay();

    ra_last_time = millis();
    action_last_time = millis();
}


bool sleeping = false;
bool wake     = false;
void loop()     {

    time_now = millis();
    
    //wait screen 
    if (wake)  {
        display.setContrast((menu_op_value[brilho_tela]*127)/100);
        action_last_time = time_now;
        wake     = false;
        sleeping = false;
    }
    else if (!sleeping && (time_now - action_last_time) >= (menu_op_value[tempo_tela]*1000))  { //sec to ms
        display.setContrast(0);
        sleeping = true;
    }

    //automode settings
    if (auto_mode) {
        int tmp = menu_op_value[RA];
        if ((time_now - ra_last_time >= RATE_ROTATION_MS))    {
            menu_op_value[RA]++; //R.A. increment over time
            ra_last_time = time_now;
        }

    }

    
    //menu movement
    current_selection = menu_top_row+arrow_row; //qual option ta selecionada agora
    // if (on_menu)    {
        
    //     //especific configs
    //     switch (current_selection) {
    //         case DEC_: { //declination
    //             if (auto_mode)  (value_preview==lock_value)?:value_preview=lock_value; //lock
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.write(byte(223)); lcd.print(value_preview%60); lcd.print("'    ");
    //             lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
    //             if (auto_mode) lcd.write(byte(4)); 
    //             else lcd.print("    ");
    //         } break;
    //         case RA: { //right ascendence
    //             if (auto_mode)  (value_preview==menu_op_value[1])?:value_preview=menu_op_value[1]; //lock on auto ra
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.print("h"); lcd.print(value_preview%60); lcd.print("m    ");
    //             lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
    //             if (auto_mode) lcd.write(byte(4)); 
    //             else lcd.print("    ");
    //         } break;
    //         case Hemisphere: { //hemisphere
    //             (value_preview>0)?value_preview=1:value_preview; //boolify
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); value_preview?lcd.print("Norte         "):lcd.print(" Sul        ");
    //         } break;
    //         case automatic_mode: { //auto-mode
    //             (value_preview>0)?value_preview=1:value_preview; //boolify
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); value_preview?lcd.print("ON        "):lcd.print("OFF       ");
    //         } break;
    //         case brilho_tela: { //brightness
    //             (value_preview>100)?value_preview=100:(value_preview<0)?value_preview=0:false;
    //             analogWrite(lcdBrightness_pin, ((value_preview*255)/100));
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("%        "); //linha 2 print
    //         } break;  
    //         case tempo_tela: { //brightness
    //             (value_preview>240)?value_preview=240:(value_preview<1)?value_preview=1:false;
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("s       "); //linha 2 print
    //         } break;   
    //         case (save_configs):     { //save (ultima op)
    //             lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(3)); lcd.print("  Salvando..."); lcd.print("         "); //linha 2 print
    //             uint8_t eeprom_brightness, eeprom_light_time, eeprom_Hemisphere;
    //             EEPROM.begin(menu_size);
    //             eeprom_brightness = EEPROM.read(0);
    //             eeprom_light_time = EEPROM.read(1);
    //             eeprom_Hemisphere = EEPROM.read(2);

    //             uint8_t brightness = menu_op_value[brilho_tela];
    //             uint8_t light_time = menu_op_value[tempo_tela];
    //             uint8_t hemisphere = menu_op_value[Hemisphere];
    //             //selective save
    //             if (eeprom_brightness != brightness)   {
    //                 EEPROM.write(0, brightness);
    //                 delay(800); //simular espera do save
    //             }
    //             if (eeprom_light_time != light_time) {
    //                 EEPROM.write(1, light_time);
    //                 delay(800);
    //             }
    //             if (eeprom_Hemisphere != hemisphere) {
    //                 EEPROM.write(2, hemisphere);
    //                 delay(800);
    //             }

    //             EEPROM.end();
    //             lcd.setCursor(0, 1); lcd.write(byte(2)); //ok
    //             delay(500);
    //             on_menu = !on_menu; //sai automaticamente da tela de salvamento
    //         } break;  
    //         default:    { //submenu generico
    //             for (uint8_t row = 1; row < LCD_ROWS; row++)  {
    //                 lcd.setCursor(0, row);
    //                 if (row == LCD_ROWS-1){ 
    //                     lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("        ");
    //                 }
    //                 else lcd.print("                  ");
    //             }
    //         }
    //     }
        
    //     if (digitalRead(SELECT) && (current_selection!=save_configs))    { //press sem ser no save
    //         menu_op_value[current_selection] = value_preview; //salva o setting
    //         lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(2)); //da o simbolo de ok;
    //         while (digitalRead(SELECT)) delay(800);
    //         wake         = true;
    //         ra_last_time = millis();
    //         on_menu      = !on_menu; //sai do sub-menu
    //         if (menu_op_value[automatic_mode])   {auto_mode = true; ra_last_time = millis();} else auto_mode = false; //inicia o modo automatico
    //     }
            
    // }

};

void shift_bits(byte* source, byte* dest, uint8_t shift, bool right)  {
    if (!right) {
        for (uint8_t i = 0; i < 8; i++)   {
           dest[i] = source[i] << shift; 
        }
    }
    else    {
        for (uint8_t i = 0; i < 8; i++)   {
           dest[i] = source[i] >> shift; 
        }
    }
    return;
}

void IRAM_ATTR rotary_IRQ()   {
    static unsigned long last_Interrupt = 0;
    unsigned long interrupt_now = millis();
    // if (on_menu)    {
    //     if ((interrupt_now - last_Interrupt) > DEBOUNCE_ROTARY_MS)  { //debounce sem delay()
    //         if (!digitalRead(ROT_B))    { //anti horario
    //             if (arrow_row) arrow_row--;
    //             else !menu_top_row?:menu_top_row--; //guard pra signed assignment e limite superior do menu
    //         }
    //         else    { //horario
    //             if (arrow_row<LCD_ROWS-1) arrow_row++;
    //             else (menu_top_row==(menu_size-LCD_ROWS))?:menu_top_row++;
    //         }
    //     }

    //     last_Interrupt = interrupt_now; //remember the change (if there was one)
    // }

    // else    {
    //     if ((interrupt_now - last_Interrupt) > DEBOUNCE_ROTARY_MS)    { //debounce sem delay()
    //         if (!digitalRead(ROT_B))   {
    //             (value_preview<1)?:value_preview--; //guard pra signed assignment
    //         }
    //         else {
    //             value_preview+=5;
    //         }
    //     }

    //     last_Interrupt = interrupt_now;
    // }
    wake = true;
    while(digitalRead(ROT_A)); //espara cair
}