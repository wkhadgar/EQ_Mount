#ifndef __SH1106_H
#define __SH1106_H

#include "stm32f1xx_hal.h"

// SH1106 display connection:
//   PB6 --> SCL
//   PB7 --> SDA

// Use bit-banding to draw pixel
//   0 - use logic operations to set pixel color
//   1 - use bit-banding to set pixel color
#define SH1106_USE_BITBAND   0

// Pixel set function definition
//   0 - call pixel function (less code size in cost of speed)
//   1 - inline pixel function (higher speed in cost of code size)
#define SH1106_OPT_PIXEL     0

// DMA usage
//   0 - DMA is not used
//   1 - compile functions for DMA transfer VRAM to display
#define SH1106_USE_DMA       0

// SH1106 HAL

// I2C port 
#define SH1106_I2C_PORT	hi2c1
#define SH1106_ADDR 0x3C //standard address for sh1106 displays (can be 0x3D)

// GPIO peripherals
#define SH1106_GPIO_PERIPH   (RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN)

// SH1106 SCL and SDA pins are set by default as PB6 and PB7
#define SH1106_SCL_PORT       GPIOB
#define SH1106_SCL_PIN        GPIO_PIN_6
#define SH1106_SDA_PORT       GPIOB
#define SH1106_SDA_PIN        GPIO_PIN_7

// Screen dimensions
#define SCR_W                 (uint8_t)128 // width
#define SCR_H                 (uint8_t)64  // height

// SH1106 command definitions
#define SH1106_CMD_setMUX    (uint8_t)0xA8 // set multiplex ratio (N, number of lines active on display)
#define SH1106_CMD_setOFFS   (uint8_t)0xD3 // set display offset
#define SH1106_CMD_STARTLINE (uint8_t)0x40 // set display start line
#define SH1106_CMD_SEG_NORM  (uint8_t)0xA0 // Column 0 is mapped to SEG0 (x coordinate normal)
#define SH1106_CMD_SEG_INV   (uint8_t)0xA1 // Column 127 is mapped to SEG0 (x coordinate inverted)
#define SH1106_CMD_COM_NORM  (uint8_t)0xC0 // Scan from COM0 to COM[N-1] (N - mux ratio, y coordinate normal)
#define SH1106_CMD_COM_INV   (uint8_t)0xC8 // Scan from COM[N-1] to COM0 (N - mux ratio, y coordinate inverted)
#define SH1106_CMD_COM_HW    (uint8_t)0xDA // set COM pins hardware configuration
#define SH1106_CMD_CONTRAST  (uint8_t)0x81 // Contrast control
#define SH1106_CMD_EDON      (uint8_t)0xA5 // Entire display ON enabled (all pixels on, RAM content ignored)
#define SH1106_CMD_EDOFF     (uint8_t)0xA4 // Entire display ON disabled (outprint follows RAM content)
#define SH1106_CMD_INV_OFF   (uint8_t)0xA6 // Entire display inversion OFF (normal display)
#define SH1106_CMD_INV_ON    (uint8_t)0xA7 // Entire display inversion ON (all pixels inverted)
#define SH1106_CMD_CLOCKDIV  (uint8_t)0xD5 // set display clock divide ratio/oscillator frequency
#define SH1106_CMD_DISP_ON   (uint8_t)0xAF // Display ON
#define SH1106_CMD_DISP_OFF  (uint8_t)0xAE // Display OFF (sleep mode)

#define SH1106_CMD_COL_LOW   (uint8_t)0x00 // set Lower Column Address
#define SH1106_CMD_COL_HIGH  (uint8_t)0x10 // set Higher Column Address
#define SH1106_CMD_PAGE_ADDR (uint8_t)0xB0 // set Page Address

#define SH1106_CMD_CHARGE    (uint8_t)0x22 //  Dis-charge / Pre-charge Period
#define SH1106_CMD_SCRL_HR   (uint8_t)0x26 // setup continuous horizontal scroll right
#define SH1106_CMD_SCRL_HL   (uint8_t)0x27 // setup continuous horizontal scroll left
#define SH1106_CMD_SCRL_VHR  (uint8_t)0x29 // setup continuous vertical and horizontal scroll right
#define SH1106_CMD_SCRL_VHL  (uint8_t)0x2A // setup continuous vertical and horizontal scroll left
#define SH1106_CMD_SCRL_STOP (uint8_t)0x2E // Deactivate scroll
#define SH1106_CMD_SCRL_ACT  (uint8_t)0x2F // Activate scroll

// Entire display on/off enumeration
enum {
	SH1106_ENTIRE_PIXELS_OFF = 0,
	SH1106_ENTIRE_PIXELS_ON = !SH1106_ENTIRE_PIXELS_OFF
};

// Display pixels inversion enumeration
enum {
	SH1106_INVERT_OFF = 0, SH1106_INVERT_ON = !SH1106_INVERT_OFF
};

// Display ON/OFF enumeration
enum {
	SH1106_OFF = 0, SH1106_ON = !SH1106_OFF
};

// Screen orientation enumeration
enum {
	SH1106_ORIENT_NORMAL = 0, // No rotation
	SH1106_ORIENT_CW = 1, // Clockwise rotation
	SH1106_ORIENT_CCW = 2, // Counter-clockwise rotation
	SH1106_ORIENT_180 = 3  // 180 degrees rotation
};

// Screen horizontal scroll direction enumeration
enum {
	SH1106_scroll_RIGHT = 0, // scroll right
	SH1106_scroll_LEFT = 1  // scroll left
};

// Screen scroll interval enumeration
enum {
	SH1106_scroll_IF2 = 0x07, // 2 frames
	SH1106_scroll_IF3 = 0x04, // 3 frames
	SH1106_scroll_IF4 = 0x05, // 4 frames
	SH1106_scroll_IF5 = 0x00, // 5 frames
	SH1106_scroll_IF25 = 0x06, // 25 frames
	SH1106_scroll_IF64 = 0x01, // 64 frames
	SH1106_scroll_IF128 = 0x02, // 128 frames
	SH1106_scroll_IF256 = 0x03  // 256 frames
};

// Pixel draw mode
enum {
	SH1106_Pset = 0x00, // set pixel
	SH1106_PRES = 0x01, // Reset pixel
	SH1106_PINV = 0x02  // Invert pixel
};

// Font structure scan lines enumeration
enum {
	FONT_V = (uint8_t) 0,        // Vertical font scan lines
	FONT_H = (uint8_t) (!FONT_V) // Horizontal font scan lines
};

// Font descriptor
typedef struct {
	uint8_t font_Width;       // Width of character
	uint8_t font_Height;      // Height of character
	uint8_t font_BPC;         // Bytes for one character
	uint8_t font_Scan;        // Font scan lines behavior
	uint8_t font_MinChar;     // Code of the first known symbol
	uint8_t font_MaxChar;     // Code of the last known symbol
	uint8_t font_UnknownChar; // Code of the unknown symbol
	uint8_t font_Data[];      // Font data
} Font_TypeDef;

// Public variables
extern uint16_t scr_width;
extern uint16_t scr_height;
extern uint8_t SH1106_PixelMode;

extern I2C_HandleTypeDef hi2c1;

// Function prototypes
//void SH1106_InitGPIO(void);
void SH1106_Init(void);

void SH1106_setContrast(uint8_t contrast);
void SH1106_setAllPixelsOn(uint8_t eon_state);
void SH1106_setInvert(uint8_t inv_state);
void SH1106_setDisplayState(uint8_t disp_state);
void SH1106_setXDir(uint8_t x_map);
void SH1106_setYDir(uint8_t y_map);
void SH1106_orientation(uint8_t orientation);

void SH1106_flush(void);
#if (SH1106_USE_DMA)
void SH1106_flush_DMA(void);
#endif // SH1106_USE_DMA

void SH1106_fill(uint8_t pattern);

void SH1106_scrollHSetup(uint8_t dir, uint8_t start, uint8_t end,
		uint8_t interval);
void SH1106_scrollDSetup(uint8_t dir, uint8_t start, uint8_t end,
		uint8_t interval, uint8_t voffs);
void SH1106_scrollStart(void);
void SH1106_scrollStop(void);

#if (SH1106_OPT_PIXEL)
inline void SH1106_pixel(uint8_t x, uint8_t y, uint8_t Mode);
#else
void SH1106_pixel(uint8_t x, uint8_t y, uint8_t Mode);
#endif // SH1106_OPT_PIXEL

void SH1106_drawHLine(uint8_t x1, uint8_t x2, uint8_t y);
void SH1106_drawVLine(uint8_t x, uint8_t y1, uint8_t y2);
void SH1106_drawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void SH1106_drawFillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void SH1106_drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void SH1106_drawCircle(int16_t x_c, int16_t y_c, uint8_t radius);
void SH1106_drawEllipse(uint16_t x_, uint16_t y_, uint16_t Ra, uint16_t Rb);

uint8_t SH1106_printChar(uint8_t x, uint8_t y, uint8_t ch,
		const Font_TypeDef *Font);
uint16_t SH1106_printStr(uint8_t x, uint8_t y, const char *str,
		const Font_TypeDef *Font);
uint16_t SH1106_printStrLF(uint8_t x, uint8_t y, const char *str,
		const Font_TypeDef *Font);
uint8_t SH1106_printInt(uint8_t x, uint8_t y, int32_t num,
		const Font_TypeDef *Font);
uint8_t SH1106_printIntU(uint8_t x, uint8_t y, uint32_t num,
		const Font_TypeDef *Font);
uint8_t SH1106_printIntF(uint8_t x, uint8_t y, int32_t num, uint8_t decimals,
		const Font_TypeDef *Font);
uint8_t SH1106_printIntLZ(uint8_t x, uint8_t y, int32_t num, uint8_t digits,
		const Font_TypeDef *Font);
uint8_t SH1106_printHex(uint8_t x, uint8_t y, uint32_t num,
		const Font_TypeDef *Font);

void SH1106_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
		const uint8_t *pBMP);
void SH1106_drawBitmapFullscreen(const uint8_t *pBMP);

#endif // __SH1106_H
