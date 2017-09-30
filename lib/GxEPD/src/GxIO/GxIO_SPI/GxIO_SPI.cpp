// created by Jean-Marc Zingg to be the GxIO_SPI io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//

#include "GxIO_SPI.h"

GxIO_SPI::GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst, int8_t bl) : IOSPI(spi)
{
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _bl   = bl;
}

void GxIO_SPI::reset()
{
  if (_rst >= 0)
  {
    delay(20);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(200);
  }
}

void GxIO_SPI::init()
{
  if (_cs >= 0)
  {
    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);
  }
  if (_dc >= 0)
  {
    digitalWrite(_dc, HIGH);
    pinMode(_dc, OUTPUT);
  }
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  if (_bl >= 0)
  {
    digitalWrite(_bl, HIGH);
    pinMode(_bl, OUTPUT);
  }
  reset();
  IOSPI.begin();
  IOSPI.setDataMode(SPI_MODE0);
  IOSPI.setBitOrder(MSBFIRST);
  setFrequency(GxIO_SPI_defaultFrequency);
}

void GxIO_SPI::setFrequency(uint32_t freq)
{
#if defined(ESP8266) || defined(ESP32)
  IOSPI.setFrequency(freq);
#elif defined(SPI_HAS_TRANSACTION)
  // true also for STM32F1xx Boards
  SPISettings settings(freq, MSBFIRST, SPI_MODE0);
  IOSPI.beginTransaction(settings);
  IOSPI.endTransaction();
#elif defined(ARDUINO_ARCH_STM32F1)|| defined(ARDUINO_ARCH_STM32F4)
#if defined(SPI_SPEED_CLOCK_DIV2_MHZ)
  // STM32F1xx Boards
  if (freq >= SPI_SPEED_CLOCK_DIV2_MHZ) setClockDivider(SPI_CLOCK_DIV2);
  else if (freq >= SPI_SPEED_CLOCK_DIV4_MHZ) setClockDivider(SPI_CLOCK_DIV4);
  else if (freq >= SPI_SPEED_CLOCK_DIV8_MHZ) setClockDivider(SPI_CLOCK_DIV8);
  else if (freq >= SPI_SPEED_CLOCK_DIV16_MHZ) setClockDivider(SPI_CLOCK_DIV16);
  else if (freq >= SPI_SPEED_CLOCK_DIV32_MHZ) setClockDivider(SPI_CLOCK_DIV32);
  else if (freq >= SPI_SPEED_CLOCK_DIV64_MHZ) setClockDivider(SPI_CLOCK_DIV64);
  else if (freq >= SPI_SPEED_CLOCK_DIV128_MHZ) setClockDivider(SPI_CLOCK_DIV128);
  else setClockDivider(SPI_CLOCK_DIV128);
#elif defined(__STM32F1__) || defined(__STM32F4__)
  // STM32 Boards (STM32duino.com)
  static const spi_baud_rate baud_rates[8] __FLASH__ = {
    SPI_BAUD_PCLK_DIV_2,
    SPI_BAUD_PCLK_DIV_4,
    SPI_BAUD_PCLK_DIV_8,
    SPI_BAUD_PCLK_DIV_16,
    SPI_BAUD_PCLK_DIV_32,
    SPI_BAUD_PCLK_DIV_64,
    SPI_BAUD_PCLK_DIV_128,
    SPI_BAUD_PCLK_DIV_256,
  };
  uint32_t clock = STM32_PCLK1 / 2;
  uint32_t i = 0;
  while (i < 7 && freq < clock) {
    clock /= 2;
    i++;
  };
  setClockDivider(baud_rates[i]);
#endif
#elif defined(__AVR)
  uint8_t clockDiv;
  if (freq >= F_CPU / 2) {
    clockDiv = SPI_CLOCK_DIV4;
  } else if (freq >= F_CPU / 4) {
    clockDiv = SPI_CLOCK_DIV16;
  } else if (freq >= F_CPU / 8) {
    clockDiv = SPI_CLOCK_DIV64;
  } else if (freq >= F_CPU / 16) {
    clockDiv = SPI_CLOCK_DIV128;
  } else if (freq >= F_CPU / 32) {
    clockDiv = SPI_CLOCK_DIV2;
  } else if (freq >= F_CPU / 64) {
    clockDiv = SPI_CLOCK_DIV8;
  } else {
    clockDiv = SPI_CLOCK_DIV32;
  }
  setClockDivider(clockDiv);
#else
  // keep the SPI default (should be 4MHz)
#endif
}

void GxIO_SPI::setClockDivider(uint32_t clockDiv)
{
  IOSPI.setClockDivider(clockDiv);
}

uint8_t GxIO_SPI::transferTransaction(uint8_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint8_t rv = IOSPI.transfer(d);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint16_t GxIO_SPI::transfer16Transaction(uint16_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  //uint16_t rv = IOSPI.transfer16(d);
  uint16_t rv = IOSPI.transfer((byte)(d & 0xFF));
  rv &= IOSPI.transfer((byte)((d >> 8) & 0xFF)) << 8;
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_SPI::readDataTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint8_t rv = IOSPI.transfer(0xFF);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint16_t GxIO_SPI::readData16Transaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  //uint16_t rv = IOSPI.transfer16(0xFFFF);
  uint16_t rv = IOSPI.transfer(0xFF);
  rv &= IOSPI.transfer(0xFF) << 8;
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_SPI::readData()
{
  return IOSPI.transfer(0xFF);
}

uint16_t GxIO_SPI::readData16()
{
  //return IOSPI.transfer16(0xFFFF);
  uint16_t rv = IOSPI.transfer(0xFF);
  rv &= IOSPI.transfer(0xFF) << 8;
  return rv;
}

void GxIO_SPI::writeCommandTransaction(uint8_t c)
{
  if (_dc >= 0) digitalWrite(_dc, LOW);
  if (_cs >= 0) digitalWrite(_cs, LOW);
  IOSPI.transfer(c);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  if (_dc >= 0) digitalWrite(_dc, HIGH);
}

void GxIO_SPI::writeDataTransaction(uint8_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  IOSPI.transfer(d);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI::writeData16Transaction(uint16_t d, uint32_t num)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  writeData16(d, num);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI::writeCommand(uint8_t c)
{
  if (_dc >= 0) digitalWrite(_dc, LOW);
  IOSPI.transfer(c);
  if (_dc >= 0) digitalWrite(_dc, HIGH);
}

void GxIO_SPI::writeData(uint8_t d)
{
  IOSPI.transfer(d);
}

void GxIO_SPI::writeData(uint8_t* d, uint32_t num)
{
#if defined(ESP8266) || defined(ESP32)
  IOSPI.writeBytes(d, num);
#else
  while (num > 0)
  {
    IOSPI.transfer(*d);
    d++;
    num--;
  }
#endif
}

void GxIO_SPI::writeData16(uint16_t d, uint32_t num)
{
#if defined(ESP8266) || defined(ESP32)
  uint8_t b[2] = {d >> 8 , d};
  IOSPI.writePattern(b, 2, num);
#else
  while (num > 0)
  {
    //IOSPI.transfer16(d);
    IOSPI.transfer((byte)(d & 0xFF));
    IOSPI.transfer((byte)((d >> 8) & 0xFF));
    num--;
  }
#endif
}

void GxIO_SPI::writeAddrMSBfirst(uint16_t d)
{
  IOSPI.transfer(d >> 8);
  IOSPI.transfer(d & 0xFF);
}

void GxIO_SPI::startTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
}

void GxIO_SPI::endTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI::selectRegister(bool rs_low)
{
  if (_dc >= 0) digitalWrite(_dc, (rs_low ? LOW : HIGH));
}

void GxIO_SPI::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}


//#endif
