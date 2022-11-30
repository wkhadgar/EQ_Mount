#include <sh1106.h>
#include <string.h>

// Screen dimensions
uint16_t scr_width = SCR_W;
uint16_t scr_height = SCR_H;

// Pixel drawing mode
// Whereas in most drawing operations pixels are set, use global variable to select drawing mode
// instead of passing set/reset/invert mode in each call of drawing functions
uint8_t SH1106_PixelMode = SH1106_Pset;

// Display image orientation
static uint8_t scr_orientation = SH1106_ORIENT_NORMAL;

// Video RAM buffer
static uint8_t vRAM[(SCR_W * SCR_H) >> 3] __attribute__((aligned(4)));

// Vertical line drawing look up table for first byte
static const uint8_t LUT_FB[] =
		{0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};

// Vertical line drawing look up table for last byte
static const uint8_t LUT_LB[] =
		{0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};

// Send single byte command to display
// input:
//   cmd - display command
static void SH1106_cmd(uint8_t cmd) {
	// Send command to display
	uint8_t command[] = {0x00, cmd};
	HAL_I2C_Master_Transmit(&SH1106_I2C_PORT, SH1106_ADDR << 1, command,
							sizeof(command), 10);
}

// Send double byte command to display
// input:
//   data - page buffer data with 0x40 blocks
//   num_bytes - number of bytes to display in the page
static void SH1106_data(uint8_t* data, uint8_t num_bytes) {
	// Send double byte command to display
	HAL_I2C_Master_Transmit(&SH1106_I2C_PORT, SH1106_ADDR << 1, data,
							num_bytes, 10);
}

// Initialize SDA peripheral and SH1106 display
// note: SDA peripheral must be initialized before
void SH1106_Init(void) {
	// Initial display configuration
	HAL_Delay(100);
	
	SH1106_cmd(SH1106_CMD_DISP_OFF);
	
	SH1106_cmd(SH1106_CMD_COL_LOW);
	SH1106_cmd(SH1106_CMD_COL_HIGH);
	
	SH1106_cmd(SH1106_CMD_STARTLINE);
	
	SH1106_cmd(SH1106_CMD_PAGE_ADDR);
	
	SH1106_cmd(SH1106_CMD_CONTRAST);
	SH1106_cmd(0x88); // 0x00...0xff
	
	SH1106_cmd(SH1106_CMD_SEG_NORM + 1);
	
	SH1106_cmd(SH1106_CMD_EDOFF); // Display follows RAM content
	
	SH1106_cmd(SH1106_CMD_INV_OFF); // Normal display mode
	
	SH1106_cmd(SH1106_CMD_SETMUX);
	SH1106_cmd(0x3F); // 63 -> 64mux
	
	SH1106_cmd(SH1106_CMD_DC_DC_SET);
	SH1106_cmd(0x8B); // on
	SH1106_cmd(0x32); // 8v
	
	
	SH1106_cmd(SH1106_CMD_COM_INV);
	
	SH1106_cmd(SH1106_CMD_SETOFFS);
	SH1106_cmd(0x00); // 0px
	
	SH1106_cmd(SH1106_CMD_CLOCKDIV);
	SH1106_cmd(0xF0); // ~100Hz
	
	SH1106_cmd(SH1106_CMD_SET_CHARGE);
	SH1106_cmd(0x2A); //4MSb are discharge ticks and 4LSb are pre charge ticks
	
	// Set COM pins hardware configuration
	// bit[4]: reset - sequential COM pin configuration
	//         set   - alternative COM pin configuration (reset value)
	// bit[5]: reset - disable COM left/right remap (reset value)
	//         set   - enable COM left/right remap
	SH1106_cmd(SH1106_CMD_COM_HW);
	SH1106_cmd(0x12);
	
	SH1106_cmd(0xDB);
	SH1106_cmd(0x40);
	
	HAL_Delay(100);
	
	SH1106_cmd(SH1106_CMD_DISP_ON); // Display enabled
	
	HAL_Delay(100);
}

// set display contrast
// input:
//   contrast - new contrast value (0x00..0xff)
void SH1106_setContrast(uint8_t contrast) {
	SH1106_cmd(SH1106_CMD_CONTRAST);
	SH1106_cmd(contrast);
}

// set entire LCD pixels on or off
// input:
//   eon_state - new pixels state (one of SH1106_ENTIRE_PIXELS_XXX values)
// note: SH1106_ENTIRE_PIXELS_ON means what all pixels on display will be on
//       without regard of display memory contents
void SH1106_setAllPixelsOn(uint8_t eon_state) {
	SH1106_cmd(eon_state ? SH1106_CMD_EDON : SH1106_CMD_EDOFF);
}

// set display pixels inversion
// input:
//   inv_state - new state of display inversion (one of SH1106_INVERT_XXX values)
// note: SH1106_INVERT_ON means what all pixels on display will be inverted
void SH1106_setInvert(uint8_t inv_state) {
	SH1106_cmd(inv_state ? SH1106_CMD_INV_ON : SH1106_CMD_INV_OFF);
}

// Toggle display on/off
// input:
//   disp_state - new display state (SH1106_ON or SH1106_OFF)
// note: doesn't affect the display memory
void SH1106_setDisplayState(uint8_t disp_state) {
	SH1106_cmd(disp_state ? SH1106_CMD_DISP_ON : SH1106_CMD_DISP_OFF);
}

// set x coordinate mapping (normal or mirrored)
// input:
//   x_map - new mapping of x coordinate (one of SH1106_INVERT_XXX values)
// note: SH1106_INVERT_OFF means normal SEG scan direction
// note: new setting will only affect subsequent data output
void SH1106_setXDir(uint8_t x_map) {
	SH1106_cmd(x_map ? SH1106_CMD_SEG_INV : SH1106_CMD_SEG_NORM);
}

// set y coordinate mapping (normal or mirrored)
// input:
//   y_map - new mapping of y coordinate (one of SH1106_INVERT_XXX values)
// note: SH1106_INVERT_OFF means normal COM scan direction
// note: new setting flip screen image immediately
void SH1106_setYDir(uint8_t y_map) {
	SH1106_cmd(y_map ? SH1106_CMD_COM_INV : SH1106_CMD_COM_NORM);
}

// set display orientation
// input:
//   orientation - new display orientation (one of SH1106_ORIENT_XXX values)
// note: normal orientation is FPC on top of COG
// note: this setting specifies an orientation of display, not orientation of image
void SH1106_orientation(uint8_t orientation) {
	// Configure display SEG/COM scan direction
	switch (orientation) {
		case SH1106_ORIENT_CW:
			// Clockwise rotation
			scr_width = SCR_H;
			scr_height = SCR_W;
			SH1106_setXDir(SH1106_INVERT_ON);
			SH1106_setYDir(SH1106_INVERT_OFF);
			break;
		case SH1106_ORIENT_CCW:
			// Counter-clockwise rotation
			scr_width = SCR_H;
			scr_height = SCR_W;
			SH1106_setXDir(SH1106_INVERT_OFF);
			SH1106_setYDir(SH1106_INVERT_ON);
			break;
		case SH1106_ORIENT_180:
			// 180 degree rotation
			scr_width = SCR_W;
			scr_height = SCR_H;
			SH1106_setXDir(SH1106_INVERT_OFF);
			SH1106_setYDir(SH1106_INVERT_OFF);
			break;
		default:
			// Normal orientation
			scr_width = SCR_W;
			scr_height = SCR_H;
			SH1106_setXDir(SH1106_INVERT_ON);
			SH1106_setYDir(SH1106_INVERT_ON);
			break;
	}
	
	// Store orientation
	scr_orientation = orientation;
}

// clear vRAM buffer
void SH1106_clear(void) {
	uint8_t page;
	static uint8_t null_buff[SCR_W] = {0x00};
	for (page = 0; page < 8; page++) {
		memcpy(vRAM + (page << 7), null_buff, SCR_W);
	}
}

// Send vRAM buffer into display
void SH1106_flush(void) {
	uint8_t page;
	static uint8_t screen_buffer[SCR_W + 1] = {0x40};
	for (page = 0; page < 8; page++) {
		
		uint8_t* page_content = vRAM + (page << 7);
		memcpy(screen_buffer + 1, page_content, SCR_W);
		
		SH1106_cmd(SH1106_CMD_PAGE_ADDR + page);
		SH1106_cmd(SH1106_CMD_COL_LOW);
		SH1106_cmd(SH1106_CMD_COL_HIGH);
		
		SH1106_data(screen_buffer, sizeof(screen_buffer));
	}
}

// Fill vRAM memory with specified pattern
// input:
//   pattern - byte to fill vRAM buffer
void SH1106_fill(uint8_t pattern) {
	uint8_t page;
	static uint8_t fill_buff[SCR_W] = {0x00};
	memset(fill_buff, pattern, sizeof(fill_buff));
	
	for (page = 0; page < 8; page++) {
		memcpy(vRAM + (page << 7), fill_buff, SCR_W);
	}
}

//init routine grouped in one function
void SH1106_cleanInit(void) {
	SH1106_Init();
	SH1106_setAllPixelsOn(0);
	SH1106_clear();
	SH1106_flush();
}

// Horizontal scroll setup
// input:
//   dir - scroll direction (one of SH1106_scroll_XXX values)
//   start - start page address [0..7]
//   end - end page address [0..7], must be great or equal to start value
//   interval - time interval between scroll steps (one of SH1106_scroll_IFXXX values)
void SH1106_scrollHSetup(uint8_t dir, uint8_t start, uint8_t end,
						 uint8_t interval) {
	SH1106_cmd((dir == SH1106_scroll_RIGHT) ?
			   SH1106_CMD_SCRL_HR :
			   SH1106_CMD_SCRL_HL);
	SH1106_cmd(0x00); // dummy byte
	SH1106_cmd(start); // Start page address
	SH1106_cmd(interval); // Time interval between each scroll stop in terms of frame frequency
	SH1106_cmd(end); // End page address
	SH1106_cmd(0x00); // dummy byte
	SH1106_cmd(0xFF); // dummy byte
}

// Diagonal (vertical and horizontal) scroll setup
// input:
//   dir - horizontal scroll direction (one of SH1106_scroll_XXX values)
//   start - start page address [0..7]
//   end - end page address [0..7], must be great or equal to start value
//   interval - time interval between scroll steps (one of SH1106_scroll_IFXXX values)
//   voffs - vertical scrolling offset, this value specifies how many lines will
//           be scrolled vertically per one scroll half_step [1..63]
void SH1106_scrollDSetup(uint8_t dir, uint8_t start, uint8_t end,
						 uint8_t interval, uint8_t voffs) {
	SH1106_cmd((dir == SH1106_scroll_RIGHT) ?
			   SH1106_CMD_SCRL_VHR :
			   SH1106_CMD_SCRL_VHL);
	SH1106_cmd(0x00); // dummy byte
	SH1106_cmd(start); // Start page address
	SH1106_cmd(interval); // Time interval between each scroll stop in terms of frame frequency
	SH1106_cmd(end); // End page address
	SH1106_cmd(voffs); // Vertical scrolling offset
	
}

// Activate scrolling
// note: this function must be called only after scroll setup
// note: changing of video RAM contents and scroll parameters are prohibited
//       after the scrolling is activated
void SH1106_scrollStart(void) {
	SH1106_cmd(SH1106_CMD_SCRL_ACT);
	
}

// Deactivate scrolling
// note: after calling this function the graphics RAM data needs to be rewritten
void SH1106_scrollStop(void) {
	SH1106_cmd(SH1106_CMD_SCRL_STOP);
	
}

// set pixel in vRAM buffer
// input:
//   x, y - pixel coordinates
//   Mode - pixel mode (one of SH1106_PXXX values)
#if (SH1106_OPT_PIXEL)
__attribute__((always_inline)) void SH1106_pixel(uint8_t x, uint8_t y, uint8_t Mode) {
#else

void SH1106_pixel(uint8_t x, uint8_t y, uint8_t Mode) {
#endif // SH1106_OPT_PIXEL
	register uint32_t offset;
	register uint32_t bpos;
	
	// Offset of pixel in the vRAM array must be computed by formula ((y >> 3) * SCR_W) + x
	// Since screen is 128 pixel width the formula can be simplified to ((y >> 3) << 7) + x
	// For 90 degree rotation x and y must be swapped
	if (scr_orientation == SH1106_ORIENT_CW
		|| scr_orientation == SH1106_ORIENT_CCW) {
		offset = ((x >> 3) << 7) + y;
		bpos = x & 0x07;
	} else {
		offset = ((y >> 3) << 7) + x;
		bpos = y & 0x07;
	}
	
	// Return if offset went out outside of vRAM
	if (offset > ((SCR_W * SCR_H) >> 3)) {
		return;
	}

#if (SH1106_USE_BITBAND)
	switch (Mode) {
	case SH1106_PRES:
		*(uint32_t*) (SRAM_BB_BASE
				+ (((uint32_t) ((void*) (&vRAM[offset])) - SRAM_BASE) << 5)
				+ (bpos << 2)) = 0;
		break;
	case SH1106_PINV:
		*(uint32_t*) (SRAM_BB_BASE
				+ (((uint32_t) ((void*) (&vRAM[offset])) - SRAM_BASE) << 5)
				+ (bpos << 2)) ^= 1;
		break;
	default:
		*(uint32_t*) (SRAM_BB_BASE
				+ (((uint32_t) ((void*) (&vRAM[offset])) - SRAM_BASE) << 5)
				+ (bpos << 2)) = 1;
		break;
	}
#else // (SH1106_USE_BITBAND)
	switch (Mode) {
		case SH1106_PRES:
			vRAM[offset] &= ~(1 << bpos);
			break;
		case SH1106_PINV:
			vRAM[offset] ^= (1 << bpos);
			break;
		default:
			vRAM[offset] |= (1 << bpos);
			break;
	}
#endif // SH1106_USE_BITBAND
}

// Optimized draw horizontal line (without regard of screen rotation)
// input:
//   x - horizontal coordinate of line start
//   y - vertical coordinate
//   w - line width
static void SH1106_drawHLineInt(uint8_t x, uint8_t y, uint8_t w) {
	uint8_t* ptr;
	uint8_t mask;
	
	// Pointer to the first byte of line in video buffer
	// This is optimized formula, original is "((y >> 3) * SCR_W) + x"
	ptr = &vRAM[((y >> 3) << 7)] + x;
	
	// Mask bit for pixel in byte
	mask = 1 << (y & 0x07);
	
	// Draw line
	switch (SH1106_PixelMode) {
		case SH1106_PRES:
			mask = ~mask;
			while (w--)
				*ptr++ &= mask;
			break;
		case SH1106_PINV:
			while (w--)
				*ptr++ ^= mask;
			break;
		default:
			while (w--)
				*ptr++ |= mask;
			break;
	}
}

// Optimized draw vertical line (without regard of screen rotation)
// input:
//   x - horizontal coordinate
//   y - vertical coordinate
//   h - line length
static void SH1106_drawVLineInt(uint8_t x, uint8_t y, uint8_t h) {
	uint8_t* ptr;
	uint8_t mask;
	uint8_t modulo;
	
	// Pointer to the first byte of line in video buffer
	// This is optimized formula, original is "((y >> 3) * SCR_W) + x"
	ptr = &vRAM[((y >> 3) << 7)] + x;
	
	// First partial byte?
	modulo = (y & 0x07);
	if (modulo) {
		// Get bit mask for first partial byte from lookup table
		modulo = 8 - modulo;
		mask = LUT_FB[modulo];
		
		// Trim mask if line is will not go out from a current byte
		if (modulo > h)
			mask &= (0xFF >> (modulo - h));
		
		// Modify bits in first byte of line
		switch (SH1106_PixelMode) {
			case SH1106_PRES:
				*ptr &= ~mask;
				break;
			case SH1106_PINV:
				*ptr ^= mask;
				break;
			default:
				*ptr |= mask;
				break;
		}
		
		// Return if line is over
		if (modulo > h)
			return;
		
		// Shift pointer to the next byte in line and decrease line height counter
		ptr += SCR_W;
		h -= modulo;
	}
	
	// Fill solid bytes
	if (h > 7) {
		// Separate cycle for each case of pixel mode (to improve performance)
		switch (SH1106_PixelMode) {
			case SH1106_PRES:
				do {
					*ptr = 0x00;
					ptr += SCR_W;
					h -= 8;
				} while (h > 7);
				break;
			case SH1106_PINV:
				do {
					*ptr = ~(*ptr);
					ptr += SCR_W;
					h -= 8;
				} while (h > 7);
				break;
			default:
				do {
					*ptr = 0xFF;
					ptr += SCR_W;
					h -= 8;
				} while (h > 7);
				break;
		}
	}
	
	// Last partial byte?
	if (h) {
		// Get bit mask for last partial byte from lookup table
		modulo = (h & 0x07);
		mask = LUT_LB[modulo];
		
		// Modify bits in last byte of line
		switch (SH1106_PixelMode) {
			case SH1106_PRES:
				*ptr &= ~mask;
				break;
			case SH1106_PINV:
				*ptr ^= mask;
				break;
			default:
				*ptr |= mask;
				break;
		}
	}
}

// Draw horizontal line
// input:
//   x1, x2 - left and right horizontal coordinates
//   y - vertical coordinate
void SH1106_drawHLine(uint8_t x1, uint8_t x2, uint8_t y) {
	uint8_t x, w;
	
	if (x1 > x2) {
		x = x2;
		w = x1 - x2;
	} else {
		x = x1;
		w = x2 - x1;
	}
	w++;
	
	if (scr_orientation == SH1106_ORIENT_CW
		|| scr_orientation == SH1106_ORIENT_CCW) {
		SH1106_drawVLineInt(y, x, w);
	} else {
		SH1106_drawHLineInt(x, y, w);
	}
}

// Draw vertical line
// input:
//   x - horizontal coordinate
//   y1,y2 - top and bottom vertical coordinates (y1 must be less than y2)
void SH1106_drawVLine(uint8_t x, uint8_t y1, uint8_t y2) {
	uint8_t y, h;
	
	if (y1 > y2) {
		y = y2;
		h = y1 - y2;
	} else {
		y = y1;
		h = y2 - y1;
	}
	h++;
	
	if (scr_orientation == SH1106_ORIENT_CW
		|| scr_orientation == SH1106_ORIENT_CCW) {
		SH1106_drawHLineInt(y, x, h);
	} else {
		SH1106_drawVLineInt(x, y, h);
	}
}

// Draw rectangle
// input:
//   x1,y1 - top left coordinates
//   x2,y2 - bottom right coordinates
void SH1106_drawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	SH1106_drawHLine(x1, x2, y1);
	SH1106_drawHLine(x1, x2, y2);
	SH1106_drawVLine(x1, y1 + 1, y2 - 1);
	SH1106_drawVLine(x2, y1 + 1, y2 - 1);
}

// Draw filled rectangle
// input:
//   x1,y1 - top left coordinates
//   x2,y2 - bottom right coordinates
void SH1106_fillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	uint8_t z, e, t, l;
	
	// Fill rectangle by vertical lines is most optimal, therefore calculate coordinates
	// with regard of screen rotation
	if (scr_orientation == SH1106_ORIENT_CW
		|| scr_orientation == SH1106_ORIENT_CCW) {
		if (x1 > x2) {
			t = x2;
			l = x1 - x2;
		} else {
			t = x1;
			l = x2 - x1;
		}
		
		if (y1 > y2) {
			z = y1;
			e = y2;
		} else {
			z = y2;
			e = y1;
		}
	} else {
		if (y1 > y2) {
			t = y2;
			l = y1 - y2;
		} else {
			t = y1;
			l = y2 - y1;
		}
		
		if (x1 > x2) {
			z = x1;
			e = x2;
		} else {
			z = x2;
			e = x1;
		}
	}
	l++;
	
	// Fill a rectangle
	do {
		SH1106_drawVLineInt(z, t, l);
	} while (z-- > e);
}

// Draw line
// input:
//   x1,y1 - top left coordinates
//   x2,y2 - bottom right coordinates
void SH1106_drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	int16_t dx = x2 - x1;
	int16_t dy = y2 - y1;
	int16_t dx_sym = (dx > 0) ? 1 : -1;
	int16_t dy_sym = (dy > 0) ? 1 : -1;
	
	if (dx == 0) {
		SH1106_drawVLine(x1, y1, y2);
		
		return;
	}
	if (dy == 0) {
		SH1106_drawHLine(x1, x2, y1);
		
		return;
	}
	
	dx *= dx_sym;
	dy *= dy_sym;
	int16_t dx2 = dx << 1;
	int16_t dy2 = dy << 1;
	int16_t di;
	
	if (dx >= dy) {
		di = dy2 - dx;
		while (x1 != x2) {
			SH1106_pixel(x1, y1, SH1106_PixelMode);
			x1 += dx_sym;
			if (di < 0) {
				di += dy2;
			} else {
				di += dy2 - dx2;
				y1 += dy_sym;
			}
		}
	} else {
		di = dx2 - dy;
		while (y1 != y2) {
			SH1106_pixel(x1, y1, SH1106_PixelMode);
			y1 += dy_sym;
			if (di < 0) {
				di += dx2;
			} else {
				di += dx2 - dy2;
				x1 += dx_sym;
			}
		}
	}
	SH1106_pixel(x1, y1, SH1106_PixelMode);
}

// Draw circle
// input:
//   x_,x_ - circle center coordinates
//   radius - circle radius
void SH1106_drawCircle(int16_t x_, int16_t y_, uint8_t radius) {
	int16_t err = 1 - radius;
	int16_t dx = 0;
	int16_t dy = -2 * radius;
	int16_t x = 0;
	int16_t y = radius;
	// Screen width and height for less calculations
	int16_t sh = scr_height - 1;
	int16_t sw = scr_width - 1;
	
	while (x < y) {
		if (err >= 0) {
			dy += 2;
			err += dy;
			y--;
		}
		dx += 2;
		err += dx + 1;
		x++;
		
		// Draw eight pixels of each octant
		if (x_ + x < sw) {
			if (y_ + y < sh)
				SH1106_pixel(x_ + x, y_ + y, SH1106_PixelMode);
			if (y_ - y > -1)
				SH1106_pixel(x_ + x, y_ - y, SH1106_PixelMode);
		}
		if (x_ - x > -1) {
			if (y_ + y < sh)
				SH1106_pixel(x_ - x, y_ + y, SH1106_PixelMode);
			if (y_ - y > -1)
				SH1106_pixel(x_ - x, y_ - y, SH1106_PixelMode);
		}
		if (x_ + y < sw) {
			if (y_ + x < sh)
				SH1106_pixel(x_ + y, y_ + x, SH1106_PixelMode);
			if (y_ - x > -1)
				SH1106_pixel(x_ + y, y_ - x, SH1106_PixelMode);
		}
		if (x_ - y > -1) {
			if (y_ + x < sh)
				SH1106_pixel(x_ - y, y_ + x, SH1106_PixelMode);
			if (y_ - x > -1)
				SH1106_pixel(x_ - y, y_ - x, SH1106_PixelMode);
		}
	}
	
	// Vertical and horizontal points
	if (x_ + radius < sw)
		SH1106_pixel(x_ + radius, y_, SH1106_PixelMode);
	if (x_ - radius > -1)
		SH1106_pixel(x_ - radius, y_, SH1106_PixelMode);
	if (y_ + radius < sh)
		SH1106_pixel(x_, y_ + radius, SH1106_PixelMode);
	if (y_ - radius > -1)
		SH1106_pixel(x_, y_ - radius, SH1106_PixelMode);
}

// Draw ellipse
// input:
//   x_, y_ - coordinates of center of the ellipse
//   Ra,Rb - horizontal and vertical radiuses
void SH1106_drawEllipse(uint16_t x_, uint16_t y_, uint16_t Ra, uint16_t Rb) {
	int16_t x = 0;
	int16_t y = Rb;
	int32_t A2 = Ra * Ra;
	int32_t B2 = Rb * Rb;
	int32_t C1 = -((A2 >> 2) + (Ra & 0x01) + B2);
	int32_t C2 = -((B2 >> 2) + (Rb & 0x01) + A2);
	int32_t C3 = -((B2 >> 2) + (Rb & 0x01));
	int32_t t = -A2 * y;
	int32_t dx = B2 * x * 2;
	int32_t dy = -A2 * y * 2;
	int32_t dXt2 = B2 * 2;
	int32_t dYt2 = A2 * 2;
	// Screen width and height for less calculations
	int16_t sh = scr_height - 1;
	int16_t sw = scr_width - 1;
	
	while ((y >= 0) && (x <= Ra)) {
		if ((x_ + x < sw) && (y_ + y < sh)) {
			SH1106_pixel(x_ + x, y_ + y, SH1106_PixelMode);
		}
		if (x || y) {
			if ((x_ - x > -1) && (y_ - y > -1)) {
				SH1106_pixel(x_ - x, y_ - y, SH1106_PixelMode);
			}
		}
		if (x && y) {
			if ((x_ + x < sw) && (y_ - y > -1)) {
				SH1106_pixel(x_ + x, y_ - y, SH1106_PixelMode);
			}
			if ((x_ - x > -1) && (y_ + y < sh)) {
				SH1106_pixel(x_ - x, y_ + y, SH1106_PixelMode);
			}
		}
		
		if ((t + x * B2 <= C1) || (t + y * A2 <= C3)) {
			dx += dXt2;
			t += dx;
			x++;
		} else if (t - y * A2 > C2) {
			dy += dYt2;
			t += dy;
			y--;
		} else {
			dx += dXt2;
			dy += dYt2;
			t += dx;
			t += dy;
			x++;
			y--;
		}
	}
}

/*
 * @brief: draws a simple 5x20 battery gauge at given position
 * @param fill_percentage: % of round rect to be displayed as filled on the gauge (left to right)
 * @param x: topleft x coordinate to draw gauge
 * @param y: topleft y coordinate to draw gauge
 * @param w: width of the gauge
 * @param h: height of the gauge
 */
void SH1106_drawRoundRectFill(uint8_t fill_percentage, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	SH1106_drawHLine(x + 1, x + (w - 1), y); //top
	SH1106_drawHLine(x + 1, x + (w - 1), y + (h - 1)); //bottom
	SH1106_drawVLine(x, y + 1, y + (h - 2)); //left
	SH1106_drawVLine(x + w, y + 1, y + (h - 2)); //right
	SH1106_fillRect(x, y + 1, x + (fill_percentage / (100 / (float) w)), y + (h - 2)); //percentage for filling
}

// Draw a single character
// input:
//   x,y - character top left corner coordinates
//   ch - character to be drawn
//   Font - pointer to font
// return: character width in pixels
uint8_t SH1106_printChar(uint8_t x, uint8_t y, uint8_t ch,
						 const Font_TypeDef* Font) {
	uint8_t p_x;
	uint8_t p_y;
	uint8_t tmpCh;
	uint8_t bL;
	const uint8_t* pCh;
	
	// If the specified character code is out of bounds should substitute the code of the "unknown" character
	if (ch < Font->font_MinChar || ch > Font->font_MaxChar)
		ch = Font->font_UnknownChar;
	
	// Pointer to the first byte of character in font data array
	pCh = &Font->font_Data[(ch - Font->font_MinChar) * Font->font_BPC];
	
	// Draw character
	if (Font->font_Scan == FONT_V) {
		// Vertical pixels order
		if (Font->font_Height < 9) {
			// Height is 8 pixels or less (one byte per column)
			p_x = x;
			while (p_x < x + Font->font_Width) {
				p_y = y;
				tmpCh = *pCh++;
				while (tmpCh) {
					if (tmpCh & 0x01)
						SH1106_pixel(p_x, p_y, SH1106_PixelMode);
					tmpCh >>= 1;
					p_y++;
				}
				p_x++;
			}
		} else {
			// Height is more than 8 pixels (several bytes per column)
			p_x = x;
			while (p_x < x + Font->font_Width) {
				p_y = y;
				while (p_y < y + Font->font_Height) {
					bL = 8;
					tmpCh = *pCh++;
					if (tmpCh) {
						while (bL) {
							if (tmpCh & 0x01)
								SH1106_pixel(p_x, p_y, SH1106_PixelMode);
							tmpCh >>= 1;
							if (tmpCh) {
								p_y++;
								bL--;
							} else {
								p_y += bL;
								break;
							}
						}
					} else {
						p_y += bL;
					}
				}
				p_x++;
			}
		}
	} else {
		// Horizontal pixels order
		if (Font->font_Width < 9) {
			// Width is 8 pixels or less (one byte per row)
			p_y = y;
			while (p_y < y + Font->font_Height) {
				p_x = x;
				tmpCh = *pCh++;
				while (tmpCh) {
					if (tmpCh & 0x01)
						SH1106_pixel(p_x, p_y, SH1106_PixelMode);
					tmpCh >>= 1;
					p_x++;
				}
				p_y++;
			}
		} else {
			// Width is more than 8 pixels (several bytes per row)
			p_y = y;
			while (p_y < y + Font->font_Height) {
				p_x = x;
				while (p_x < x + Font->font_Width) {
					bL = 8;
					tmpCh = *pCh++;
					if (tmpCh) {
						while (bL) {
							if (tmpCh & 0x01)
								SH1106_pixel(p_x, p_y, SH1106_PixelMode);
							tmpCh >>= 1;
							if (tmpCh) {
								p_x++;
								bL--;
							} else {
								p_x += bL;
								break;
							}
						}
					} else {
						p_x += bL;
					}
				}
				p_y++;
			}
		}
	}
	
	return Font->font_Width + 1;
}

// Draw string
// input:
//   x,y - top left coordinates of first character
//   str - pointer to zero-terminated string
//   Font - pointer to font
// return: string width in pixels
uint16_t SH1106_printStr(uint8_t x, uint8_t y, const char* str,
						 const Font_TypeDef* Font) {
	uint8_t p_x = x;
	uint8_t eX = scr_width - Font->font_Width - 1;
	
	while (*str) {
		p_x += SH1106_printChar(p_x, y, *str++, Font);
		if (p_x > eX)
			break;
	}
	
	return (p_x - x);
}

// Draw string with line feed at screen edge
// input:
//   x,y - top left coordinates of first character
//   str - pointer to zero-terminated string
//   Font - pointer to font
// return: string width in pixels
uint16_t SH1106_printStrLF(uint8_t x, uint8_t y, const char* str,
						   const Font_TypeDef* Font) {
	uint8_t strLen = 0;
	
	while (*str) {
		SH1106_printChar(x, y, *str++, Font);
		if (x < scr_width - Font->font_Width - 1) {
			x += Font->font_Width + 1;
		} else if (y < scr_height - Font->font_Height - 1) {
			x = 0;
			y += Font->font_Height;
		} else {
			x = 0;
			y = 0;
		}
		strLen++;
	};
	
	return strLen * (Font->font_Width + 1);
}

// Draw signed integer value
// input:
//   x,y - top left coordinates of first symbol
//   num - signed integer value
//   Font - pointer to font
// return: number width in pixels
uint8_t SH1106_printInt(uint8_t x, uint8_t y, int32_t num,
						const Font_TypeDef* Font) {
	uint8_t str[11]; // 10 chars max for INT32_MIN..INT32_MAX (without sign)
	uint8_t* pStr = str;
	uint8_t p_x = x;
	uint8_t neg = 0;
	
	// String termination character
	*pStr++ = '\0';
	
	// Convert number to characters
	if (num < 0) {
		neg = 1;
		num *= -1;
	}
	do {
		*pStr++ = (num % 10) + '0';
	} while (num /= 10);
	if (neg)
		*pStr++ = '-';
	
	// Draw a number
	while (*--pStr)
		p_x += SH1106_printChar(p_x, y, *pStr, Font);
	
	return (p_x - x);
}

// Draw unsigned integer value
// input:
//   x,y - top left coordinates of first symbol
//   num - unsigned integer value
//   Font - pointer to font
// return: number width in pixels
uint8_t SH1106_printIntU(uint8_t x, uint8_t y, uint32_t num,
						 const Font_TypeDef* Font) {
	uint8_t str[11]; // 10 chars max for UINT32_MAX
	uint8_t* pStr = str;
	uint8_t p_x = x;
	
	// String termination character
	*pStr++ = '\0';
	
	// Convert number to characters
	do {
		*pStr++ = (num % 10) + '0';
	} while (num /= 10);
	
	// Draw a number
	while (*--pStr)
		p_x += SH1106_printChar(p_x, y, *pStr, Font);
	
	return (p_x - x);
}

// Draw signed integer value with decimal point
// input:
//   x,y - top left coordinates of first symbol
//   num - unsigned integer value
//   decimals - number of digits after decimal point
//   Font - pointer to font
// return: number width in pixels
uint8_t SH1106_printIntF(uint8_t x, uint8_t y, int32_t num, uint8_t decimals,
						 const Font_TypeDef* Font) {
	uint8_t str[11]; // 10 chars max for INT32_MIN..INT32_MAX (without sign)
	uint8_t* pStr = str;
	uint8_t p_x = x;
	uint8_t neg = 0;
	uint8_t strLen = 0;
	
	// String termination character
	*pStr++ = '\0';
	
	// Convert number to characters
	if (num < 0) {
		neg = 1;
		num *= -1;
	}
	do {
		*pStr++ = (num % 10) + '0';
		strLen++;
	} while (num /= 10);
	
	// Add leading zeroes
	if (strLen <= decimals) {
		while (strLen <= decimals) {
			*pStr++ = '0';
			strLen++;
		}
	}
	
	// Minus sign?
	if (neg) {
		*pStr++ = '-';
		strLen++;
	}
	
	// Draw a number
	while (*--pStr) {
		p_x += SH1106_printChar(p_x, y, *pStr, Font);
		if (decimals && (--strLen == decimals)) {
			// Draw decimal point
			SH1106_drawRect(p_x, y + Font->font_Height - 2, p_x + 1,
							y + Font->font_Height - 1);
			p_x += 3;
		}
	}
	
	return (p_x - x);
}

// Draw signed integer value with leading zeros
// input:
//   x,y - top left coordinates of first symbol
//   num - unsigned integer value
//   digits - minimal number of length (e.g. num=35, digits=5 --> 00035)
//   Font - pointer to font
// return: number width in pixels
uint8_t SH1106_printIntLZ(uint8_t x, uint8_t y, int32_t num, uint8_t digits,
						  const Font_TypeDef* Font) {
	uint8_t str[11]; // 10 chars max for INT32_MIN..INT32_MAX (without sign)
	uint8_t* pStr = str;
	uint8_t p_x = x;
	uint8_t neg = 0;
	uint8_t strLen = 0;
	
	// String termination character
	*pStr++ = '\0';
	
	// Convert number to characters
	if (num < 0) {
		neg = 1;
		num *= -1;
	}
	do {
		*pStr++ = (num % 10) + '0';
		strLen++;
	} while (num /= 10);
	
	// Add leading zeroes
	if (strLen < digits)
		while (strLen++ < digits)
			*pStr++ = '0';
	
	// Minus sign?
	if (neg)
		*pStr++ = '-';
	
	// Draw a number
	while (*--pStr)
		p_x += SH1106_printChar(p_x, y, *pStr, Font);
	
	return (p_x - x);
}

// Draw integer as hexadecimal
// input:
//   x,y - top left coordinates of first symbol
//   num - unsigned integer value
//   Font - pointer to font
// return: number width in pixels
uint8_t SH1106_printHex(uint8_t x, uint8_t y, uint32_t num,
						const Font_TypeDef* Font) {
	uint8_t str[11]; // 10 chars max for UINT32_MAX
	uint8_t* pStr = str;
	uint8_t p_x = x;
	
	// String termination character
	*pStr++ = '\0';
	
	// Convert number to characters
	do {
		*pStr = (num % SH1106_CMD_COL_HIGH) + '0';
		if (*pStr > '9')
			*pStr += 7;
		pStr++;
	} while (num /= SH1106_CMD_COL_HIGH);
	
	// Draw a number
	while (*--pStr)
		p_x += SH1106_printChar(p_x, y, *pStr, Font);
	
	return (p_x - x);
}

// Draw monochrome bitmap
// input:
//   x, y - top left corner coordinates of bitmap
//   w, h - width and height of bitmap in pixels
//   pBMP - pointer to array containing bitmap
// note: each '1' bit in the bitmap will be drawn as a pixel
//       each '0' bit in the will not be drawn (transparent bitmap)
// bitmap: one byte per 8 vertical pixels, LSB top, truncate bottom bits
void SH1106_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
					   const uint8_t* pBMP) {
	uint8_t p_x;
	uint8_t p_y;
	uint8_t tmpCh;
	uint8_t bL;
	
	p_y = y;
	while (p_y < y + h) {
		p_x = x;
		while (p_x < x + w) {
			bL = 0;
			tmpCh = *pBMP++;
			if (tmpCh) {
				while (bL < 8) {
					if (tmpCh & 0x01)
						SH1106_pixel(p_x, p_y + bL, SH1106_PixelMode);
					tmpCh >>= 1;
					if (tmpCh) {
						bL++;
					} else {
						p_x++;
						break;
					}
				}
			} else {
				p_x++;
			}
		}
		p_y += 8;
	}
}

void SH1106_drawBitmapFullscreen(const uint8_t* pBMP) {
	memcpy(vRAM, pBMP, (SCR_W * SCR_H) >> 3);
}