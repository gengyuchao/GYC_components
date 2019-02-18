#include "oled_display.h"

#include "OLEDDisplayFonts.h"


#define OLED_BACK_IMAGE_ENABLE 


//OLED Settings

// Header Values
#define JUMPTABLE_BYTES 4

#define JUMPTABLE_LSB   1
#define JUMPTABLE_SIZE  2
#define JUMPTABLE_WIDTH 3
#define JUMPTABLE_START 4

#define WIDTH_POS 0
#define HEIGHT_POS 1
#define FIRST_CHAR_POS 2
#define CHAR_NUM_POS 3

#define byte char

enum OLEDDISPLAY_TEXT_ALIGNMENT {
  TEXT_ALIGN_LEFT = 0,           // 输入坐标为左起点
  TEXT_ALIGN_RIGHT = 1,          // 输入坐标为右起点
  TEXT_ALIGN_CENTER = 2,         // 输入坐标为水平中心
  TEXT_ALIGN_CENTER_BOTH = 3     // 输入坐标为整个字符串的水平和垂直中心
};

// #define OLED_FILL_TEXT_BAKCGROUND

uint16_t  displayWidth                     = 128;
uint16_t  displayHeight                    = 64;
uint16_t  displayBufferSize                = 1024;
uint8_t   *dispaly_image_buffer            = NULL;
uint8_t   *dispaly_image_buffer_back       = NULL;

uint8_t   oled_pen_color                   = WHITE;
uint8_t* fontData = (uint8_t*)ArialMT_Plain_10;
uint8_t textAlignment = TEXT_ALIGN_CENTER_BOTH;




#define memset(dst, c, n) xmemset(dst, c, n)

static void *
xmemset(void *dst, int c, size_t n)
{
    unsigned char *     dst_ = (unsigned char *) dst;
    const unsigned char c_   = (unsigned char) c;
    size_t              i;

    for (i = 0; i < n; i++) {
        dst_[i] = c_;
    }
    return dst;
}

 



esp_err_t oled_init()
{
    oled_gpio_init();
    oled_write_cmd(0xAE);    // Set Display ON/OFF (AEh/AFh)
    oled_write_cmd(0x00);    // Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
    oled_write_cmd(0x10);    // Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
    oled_write_cmd(0x40);    // Set Display Start Line (40h~7Fh)
    oled_write_cmd(0x81);    // Set Contrast Control for BANK0 (81h)
    oled_write_cmd(0xCF);    //
    oled_write_cmd(0xA1);    // Set Segment Re-map (A0h/A1h)
    oled_write_cmd(0xC8);    // Set COM Output Scan Direction (C0h/C8h)
    oled_write_cmd(0xA6);    // Set Normal/Inverse Display (A6h/A7h)
    oled_write_cmd(0xA8);    // Set Multiplex Ratio (A8h)
    oled_write_cmd(0x3F);    // Set Heigh (0-63 0x00-0x3F)
    oled_write_cmd(0xD3);    // Set Display Offset (D3h)
    oled_write_cmd(0x00);    // Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
    oled_write_cmd(0xD5);    // Set Display Clock Divide Ratio/ Oscillator Frequency (D5h)
    oled_write_cmd(0x80);    //
    oled_write_cmd(0xD9);    // Set Pre-charge Period (D9h)
    oled_write_cmd(0xF1);    //
    oled_write_cmd(0xDA);    // Set COM Pins Hardware Configuration (DAh)
    oled_write_cmd(0x12);    // Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
    oled_write_cmd(0xDB);    // Set VCOMH  Deselect Level (DBh)
    oled_write_cmd(0x40);    // Set Display Start Line (40h~7Fh)
    oled_write_cmd(0x20);    // Set Memory Addressing Mode (20h)
    oled_write_cmd(0x00);    // Set 页地址模式(A[1:0]=10b) 水平地址模式(A[1:0]=00b) 垂直地址模式(A[1:0]=01b)
    oled_write_cmd(0x8D);    //
    oled_write_cmd(0x14);    // Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
    oled_write_cmd(0xA4);    // Entire Display ON (A4h/A5h)
    oled_write_cmd(0xA6);    // Set Normal/Inverse Display (A6h/A7h)
    oled_write_cmd(0xAF);    // Set Display ON/OFF (AEh/AFh)
    return ESP_OK;
}

esp_err_t oled_set_pos(uint8_t x_start, uint8_t y_start)
{
    oled_write_cmd(0xb0 + y_start);
    oled_write_cmd(((x_start & 0xf0) >> 4) | 0x10);
    //oled_write_cmd((x_start & 0x0f) | 0x01);
    oled_write_cmd(x_start & 0x0f);

    return ESP_OK;
}

 esp_err_t oled_set_pos_range(uint8_t minBoundX, uint8_t minBoundY,uint8_t maxBoundX, uint8_t maxBoundY)
{
    oled_write_cmd(0x21);//COLUMNADDR
    oled_write_cmd(minBoundX);
    oled_write_cmd(maxBoundX);

    oled_write_cmd(0x22);//PAGEADDR
    oled_write_cmd(minBoundY);
    oled_write_cmd(maxBoundY);

    return ESP_OK;
}

esp_err_t oled_Fully_fill(uint8_t data)//Fully filled
{
	unsigned char i;
  uint8_t fill_buf[128];
  memset(fill_buf,data,128);
  oled_set_pos_range(0, 0,127, 7);
	for(i=0;i<8;i++)
	{
    // 页地址模式
		// oled_write_cmd(0xb0+i); //设置页地址（0~7）
		// oled_write_cmd(0x00); //设置显示位置—列低地址
		// oled_write_cmd(0x10); //设置显示位置—列高地址
		// for(n=0;n<128;n++)	oled_write_byte(data); //写0x00到屏幕寄存器上
    
    // 水平地址模式
    oled_write_data(fill_buf,128);//为加快刷新速率 选择使用IIC连续写入的方式
	}
  return ESP_OK;
}


esp_err_t oled_display_init()
{
    if(dispaly_image_buffer==NULL) 
    {
        dispaly_image_buffer = (uint8_t*) malloc(sizeof(uint8_t) * displayBufferSize);

        if(!dispaly_image_buffer) 
        {
            DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create display\n");
            return ESP_FAIL;
        }
        memset(dispaly_image_buffer, 0x00, displayBufferSize);
    }

 #ifdef OLED_BACK_IMAGE_ENABLE
    if(dispaly_image_buffer_back==NULL) 
    {
        dispaly_image_buffer_back = (uint8_t*) malloc(sizeof(uint8_t) * displayBufferSize);

        if(!dispaly_image_buffer_back) 
        {
            DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create dispaly_image_buffer_back display\n");
            return ESP_FAIL;
        }
        memset(dispaly_image_buffer_back, 0xFF, displayBufferSize);
    }
 #endif
    return ESP_OK;
}

/*------OLED Draw Function------*/

void setPixel(int16_t x, int16_t y) {
  if (x >= 0 && x < displayWidth && y >= 0 && y < displayHeight) {
    switch (oled_pen_color) {
      case WHITE:   dispaly_image_buffer[x + (y / 8) * displayWidth] |=  (1 << (y & 7)); break;
      case BLACK:   dispaly_image_buffer[x + (y / 8) * displayWidth] &= ~(1 << (y & 7)); break;
      case INVERSE: dispaly_image_buffer[x + (y / 8) * displayWidth] ^=  (1 << (y & 7)); break;
    }
  }
}

// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      setPixel(y0, x0);
    } else {
      setPixel(x0, y0);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void drawRect(int16_t x, int16_t y, int16_t width, int16_t height) {
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width - 1, y, height);
  drawHorizontalLine(x, y + height - 1, width);
}

void fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height) {
  for (int16_t x = xMove; x < xMove + width; x++) {
    drawVerticalLine(x, yMove, height);
  }
}

void drawCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

		setPixel(x0 + x, y0 + y);     //For the 8 octants
		setPixel(x0 - x, y0 + y);
		setPixel(x0 + x, y0 - y);
		setPixel(x0 - x, y0 - y);
		setPixel(x0 + y, y0 + x);
		setPixel(x0 - y, y0 + x);
		setPixel(x0 + y, y0 - x);
		setPixel(x0 - y, y0 - x);

	} while (x < y);

  setPixel(x0 + radius, y0);
  setPixel(x0, y0 + radius);
  setPixel(x0 - radius, y0);
  setPixel(x0, y0 - radius);
}

void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads) {
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  while (x < y) {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    if (quads & 0x1) {
      setPixel(x0 + x, y0 - y);
      setPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2) {
      setPixel(x0 - y, y0 - x);
      setPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4) {
      setPixel(x0 - y, y0 + x);
      setPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8) {
      setPixel(x0 + x, y0 + y);
      setPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8) {
    setPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8) {
    setPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4) {
    setPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2) {
    setPixel(x0, y0 - radius);
  }
}

void fillCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

    drawHorizontalLine(x0 - x, y0 - y, 2*x);
    drawHorizontalLine(x0 - x, y0 + y, 2*x);
    drawHorizontalLine(x0 - y, y0 - x, 2*y);
    drawHorizontalLine(x0 - y, y0 + x, 2*y);


	} while (x < y);
  drawHorizontalLine(x0 - radius, y0, 2 * radius);

}

void drawHorizontalLine(int16_t x, int16_t y, int16_t length) {
  if (y < 0 || y >= displayHeight) { return; }

  if (x < 0) {
    length += x;
    x = 0;
  }

  if ( (x + length) > displayWidth) {
    length = (displayWidth - x);
  }

  if (length <= 0) { return; }

  uint8_t * bufferPtr = dispaly_image_buffer;
  bufferPtr += (y >> 3) * displayWidth;
  bufferPtr += x;

  uint8_t drawBit = 1 << (y & 7);

  switch (oled_pen_color) {
    case WHITE:   while (length--) {
        *bufferPtr++ |= drawBit;
      }; break;
    case BLACK:   drawBit = ~drawBit;   while (length--) {
        *bufferPtr++ &= drawBit;
      }; break;
    case INVERSE: while (length--) {
        *bufferPtr++ ^= drawBit;
      }; break;
  }
}

void drawVerticalLine(int16_t x, int16_t y, int16_t length) {
  if (x < 0 || x >= displayWidth) return;

  if (y < 0) {
    length += y;
    y = 0;
  }

  if ( (y + length) > displayHeight) {
    length = (displayHeight - y);
  }

  if (length <= 0) return;


  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = dispaly_image_buffer;

  bufferPtr += (y >> 3) * displayWidth;
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (oled_pen_color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += displayWidth;
  }

  if (length >= 8) {
    switch (oled_pen_color) {
      case WHITE:
      case BLACK:
        drawBit = (oled_pen_color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += displayWidth;
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += displayWidth;
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << (length & 7)) - 1;
    switch (oled_pen_color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }
  }
}

void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  uint8_t  oled_pen_color_back =oled_pen_color;
  oled_pen_color=(WHITE);
  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
  drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  uint16_t maxProgressWidth = (width - doubleRadius + 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);

  oled_pen_color = oled_pen_color_back;//recover the color back before
}

uint16_t getStringWidth(const char* text, uint16_t length) {
  uint16_t firstChar        = *(fontData + FIRST_CHAR_POS);

  uint16_t stringWidth = 0;
  uint16_t maxWidth = 0;

  while (length--) {
    stringWidth += *(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10) {
      maxWidth = _max(maxWidth, stringWidth);
      stringWidth = 0;
    }
  }

  return _max(maxWidth, stringWidth);
}

void oled_setTextAlignment(uint8_t _textAlignment/*OLEDDISPLAY_TEXT_ALIGNMENT*/) {
  textAlignment = _textAlignment;
}

void oled_setFontSize(uint8_t font_Size/*Plain_10 , Plain_16 , Plain_24*/) {
  switch(font_Size)
  {
    case 10 : fontData = (uint8_t *)ArialMT_Plain_10; break;
    case 16 : fontData = (uint8_t *)ArialMT_Plain_16; break;
    case 24 : fontData = (uint8_t *)ArialMT_Plain_24; break;
    default :
    {
      printf("Warring :No Such size,setting default size as 10.\n");
      fontData = (uint8_t *)ArialMT_Plain_10; break;
    }
  }
    
}

void oled_displayOn(void) {
  oled_write_cmd(DISPLAYON);
}

void oled_displayOff(void) {
  oled_write_cmd(DISPLAYOFF);
}

void oled_invertDisplay(void) {
  oled_write_cmd(INVERTDISPLAY);
}

void oled_normalDisplay(void) {
  oled_write_cmd(NORMALDISPLAY);
}

void oled_setContrast(uint8_t contrast, uint8_t precharge, uint8_t comdetect) {
  oled_write_cmd(SETPRECHARGE); //0xD9
  oled_write_cmd(precharge); //0xF1 default, to lower the contrast, put 1-1F
  oled_write_cmd(SETCONTRAST);
  oled_write_cmd(contrast); // 0-255
  oled_write_cmd(SETVCOMDETECT); //0xDB, (additionally needed to lower the contrast)
  oled_write_cmd(comdetect);	//0x40 default, to lower the contrast, put 0
  oled_write_cmd(DISPLAYALLON_RESUME);
  oled_write_cmd(NORMALDISPLAY);
  oled_write_cmd(DISPLAYON);
}

void oled_setBrightness(uint8_t brightness) {
  uint8_t contrast = brightness;
  if (brightness < 128) {
    // Magic values to get a smooth/ step-free transition
    contrast = brightness * 1.171;
  } else {
    contrast = brightness * 1.171 - 43;
  }

  uint8_t precharge = 241;
  if (brightness == 0) {
    precharge = 0;
  }
  uint8_t comdetect = brightness / 8;

  oled_setContrast(contrast, precharge, comdetect);
}

void oled_resetOrientation() {
  oled_write_cmd(SEGREMAP);
  oled_write_cmd(COMSCANINC);           //Reset screen rotation or mirroring
}

void oled_flipScreenVertically() {
  oled_write_cmd(SEGREMAP | 0x01);
  oled_write_cmd(COMSCANDEC);           //Rotate screen 180 Deg
}

void oled_mirrorScreen() {
  oled_write_cmd(SEGREMAP);
  oled_write_cmd(COMSCANDEC);           //Mirror screen
}

void oled_clear_dispaly_image(void) {
  memset(dispaly_image_buffer, 0, displayBufferSize);
}

void drawString(int16_t xMove, int16_t yMove, char* UserStr) {
  uint16_t lineHeight = *(fontData + HEIGHT_POS);

  uint8_t length = strlen(UserStr)+1;
  // char* text must be freed! 防止用户直接写入const char*的数组 
  char* text = (char*) malloc(length * sizeof(char));

  memcpy(text, UserStr, length);

  uint16_t yOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) {
    uint16_t lb = 0;
    // Find number of linebreaks in text
    for (uint16_t i=0;text[i] != 0; i++) {
      lb += (text[i] == 10);
    }
    // Calculate center
    yOffset = (lb * lineHeight) / 2;
  }

  uint16_t line = 0;
  char* textPart = strtok(text,"\n");
 //strtok 函数会破坏原数组 如果使用const型作为输入字符串会出现修改常量数组的情况 导致内存异常
  while (textPart != NULL) {
    uint16_t length = strlen(textPart);
    drawStringInternal(xMove, yMove - yOffset + (line++) * lineHeight, textPart, length, getStringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }

  free(text);

}

void drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth) {
  uint8_t textHeight       = *(fontData + HEIGHT_POS);
  uint8_t firstChar        = *(fontData + FIRST_CHAR_POS);
  uint16_t sizeOfJumpTable = *(fontData + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  uint8_t cursorX         = 0;
  uint8_t cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      yMove -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      xMove -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      xMove -= textWidth;
      break;
    case TEXT_ALIGN_LEFT:
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth  < 0 || xMove > displayWidth ) {return;}
  if (yMove + textHeight < 0 || yMove > displayWidth ) {return;}

  for (uint16_t j = 0; j < textLength; j++) {
    int16_t xPos = xMove + cursorX;
    int16_t yPos = yMove + cursorY;

    byte code = text[j];
    if (code >= firstChar) {
      byte charCode = code - firstChar;

      // 4 Bytes per char code
      byte msbJumpToChar    = *( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      byte lsbJumpToChar    = *( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      byte charByteSize     = *( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      byte currentCharWidth = *( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(msbJumpToChar == 255 && lsbJumpToChar == 255)) {
        // Get the position of the char data
        uint16_t charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        #ifdef OLED_FILL_TEXT_BAKCGROUND //Not Use,because clear the whole screen
          if((oled_pen_color&0x02)!=1)
            oled_pen_color = !oled_pen_color;
          fillRect(xPos, yPos-1, currentCharWidth, textHeight);
          if((oled_pen_color&0x02)!=1)
            oled_pen_color = !oled_pen_color;
          //drawRect(xPos, yPos, currentCharWidth, textHeight);
        #endif
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
      }

      cursorX += currentCharWidth;
    }
  }
}

void drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t *data, uint16_t offset, uint16_t bytesInData) {
  if (width < 0 || height < 0) return;
  if (yMove + height < 0 || yMove > displayHeight)  return;
  if (xMove + width  < 0 || xMove > displayWidth)   return;

  uint8_t  rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  int16_t initYMove   = yMove;
  int8_t  initYOffset = yOffset;


  for (uint16_t i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      yMove   = initYMove;
      yOffset = initYOffset;
    }

    char currentByte = *(data + offset + i);

    int16_t xPos = xMove + (i / rasterHeight);
    int16_t yPos = ((yMove >> 3) + (i % rasterHeight)) * displayWidth;

 //    int16_t yScreenPos = yMove + yOffset;
    int16_t dataPos    = xPos  + yPos;
    uint8_t * buffer = dispaly_image_buffer;

    if (dataPos >=  0  && dataPos < displayBufferSize &&
        xPos    >=  0  && xPos    < displayWidth ) {

      if (yOffset >= 0) {
        switch (oled_pen_color) {
          case WHITE:   buffer[dataPos] |= currentByte << yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte << yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte << yOffset; break;
        }

        if (dataPos < (displayBufferSize - displayWidth)) {
          switch (oled_pen_color) {
            case WHITE:   buffer[dataPos + displayWidth] |= currentByte >> (8 - yOffset); break;
            case BLACK:   buffer[dataPos + displayWidth] &= ~(currentByte >> (8 - yOffset)); break;
            case INVERSE: buffer[dataPos + displayWidth] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (oled_pen_color) {
          case WHITE:   buffer[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte >> yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }

    }
  }
}

// Set 水平地址模式(A[1:0]=00b)
void display(void) 
{

    if(dispaly_image_buffer==NULL)
    {
        DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not init display\n");
        return;
    }

 #ifdef OLED_BACK_IMAGE_ENABLE

    if(dispaly_image_buffer_back==NULL)
    {
        DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not BACK init display\n");
        return;
    }
        
    uint8_t minBoundY = UINT8_MAX;
    uint8_t maxBoundY = 0;

    uint8_t minBoundX = UINT8_MAX;
    uint8_t maxBoundX = 0;

    uint8_t x, y;

    // Calculate the Y bounding box of changes
    // and copy buffer[pos] to buffer_back[pos];
    for (y = 0; y < (displayHeight / 8); y++) {
        for (x = 0; x < displayWidth; x++) {
          uint16_t pos = x + y * displayWidth;
          if (dispaly_image_buffer[pos] != dispaly_image_buffer_back[pos]) {
            minBoundY = _min(minBoundY, y);
            maxBoundY = _max(maxBoundY, y);
            minBoundX = _min(minBoundX, x);
            maxBoundX = _max(maxBoundX, x);
          }
          dispaly_image_buffer_back[pos] = dispaly_image_buffer[pos];
        }
    }

    // If the minBoundY wasn't updated
    // we can savely assume that buffer_back[pos] == buffer[pos]
    // holdes true for all values of pos
    if (minBoundY == UINT8_MAX) return;

    oled_write_cmd(0x21);//COLUMNADDR
    oled_write_cmd(minBoundX);
    oled_write_cmd(maxBoundX);

    oled_write_cmd(0x22);//PAGEADDR
    oled_write_cmd(minBoundY);
    oled_write_cmd(maxBoundY);

    //printf("minx:%d maxx:%d miny:%d maxy:%d\n",minBoundX,maxBoundX,minBoundY,maxBoundY);
    for (y = minBoundY; y <= maxBoundY; y++) {
        // for (x = minBoundX; x <= maxBoundX; x++) {
        //    oled_write_byte(dispaly_image_buffer[x + y * displayWidth]);//x + y * displayWidth
        // }
         oled_write_data(&dispaly_image_buffer[minBoundX + y * displayWidth],(maxBoundX-minBoundX+1));
    }
    

 #else
    // No double buffering
    oled_write_cmd(0x21);//COLUMNADDR
    oled_write_cmd(0x0);
    oled_write_cmd(0x7F);

    oled_write_cmd(0x22);//PAGEADDR
    oled_write_cmd(0x0);
    oled_write_cmd(0x7);

    for (uint16_t i=0; i<displayBufferSize; i++) {
        oled_write_byte(dispaly_image_buffer[i]);//x + y * displayWidth
    }

 #endif
}

