/*
 * Project IoTHockey
 * Description: IoT Hockey stat tracker
 * Author: Kevin Sidwar
 * Date: September 14, 2017
 * License: MIT
 */
#include <GxEPD.h>
#include <GxGDEP015OC1/GxGDEP015OC1.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <Adafruit_mfGFX/fonts.h>
//#include "mcdavid.h"
#include "fleury.h"

GxIO_SPI io(SPI, SS, D2, D1); // abstracted comm layer to the display
GxGDEP015OC1 display(io, D1, D0); // subclass of Adafruit_GFX library

// The updatable box on the display. Depending on the bitmap template image
// you use you may need multiple update boxes. This prevents the entire screen
// from redrawing
#define STAT_BOX_X  0
#define STAT_BOX_Y  31
#define STAT_BOX_W  200
#define STAT_BOX_H  65

// Struct of the stats you wish to track. Change this to suite the type
// of player you are tracking stats for. You will also need to modify the
// updateStatsOnDisplay and processNewStats methods below to use the struct
// members to modify the display.
struct stats{
  double gaa;
  double save_percentage;
  uint8_t wins;
};

void updateStatsOnDisplay(const void* v)
{
  static char gaa_str[5];
  static char sv_percentage_str[6];
  static char wins_str[3];

  // Cast the passed void pointer back to a stats struct
  stats new_stats = *reinterpret_cast<const stats*>(v);

  // Turn the stats to strings for display
  sprintf(gaa_str, "%4.2f", new_stats.gaa);
  sprintf(sv_percentage_str, "%5.3f", new_stats.save_percentage);
  sprintf(wins_str, "%02d", new_stats.wins);

  // The setCursor values are found by trial and error to get the numbers
  // to show exactly where you want them to within the template bitmap
  display.setCursor(5, 55);
  display.print(gaa_str);
  display.setCursor(72, 55);
  if(sv_percentage_str[0] == '1')
    display.print("1.00");
  else
    display.print(&sv_percentage_str[1]);
  display.setCursor(151, 55);
  display.print(wins_str);
}

void initializeScreen()
{
  // Clear the screen and initialize it with the template for the player
  // stats to display on
  display.init();
  display.setFont(ARIAL_8);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(3);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.setCursor(0, 0);
  display.drawBitmap(fleury_bmp, sizeof(fleury_bmp), GxEPD::bm_flip_v | GxEPD::bm_partial_update);
}

void processNewStats(double _gaa, double _save_percentage, uint8_t _wins)
{
  stats new_stats;
  new_stats.gaa = _gaa;
  new_stats.save_percentage = _save_percentage;
  new_stats.wins = _wins;

  display.drawPagedToWindow(updateStatsOnDisplay, STAT_BOX_X, STAT_BOX_Y, STAT_BOX_W, STAT_BOX_H, &new_stats);
}

int updateTrackedStat(String statData)
{
  // Extract the stats from the statData string which needs to be in the
  // format [gaa],[save percentage],[wins]
  double gaa = (double)statData.substring(0,statData.indexOf(',')).toFloat();
  double sv_percentage = (double)statData.substring(statData.indexOf(',') + 1,statData.lastIndexOf(',')).toFloat();
  int wins = (double)statData.substring(statData.lastIndexOf(',') + 1).toInt();

  Serial.printlnf("%4.2f %5.3f %02d", gaa, sv_percentage, wins);
  processNewStats(gaa, sv_percentage, (uint8_t)wins);
  return 0;
}

void setup() {
  Serial.begin(9600);
  initializeScreen();
  Particle.function("updateStat", updateTrackedStat);
}

void loop() {
  /*processNewStats(0, 1.0, 0);
  delay(2000);
  processNewStats(1.23, 0.934, 0);
  delay(2000);
  processNewStats(2.34, 0.926, 5);
  delay(2000);
  processNewStats(3.33, 0.91, 13);
  delay(2000);*/
}
