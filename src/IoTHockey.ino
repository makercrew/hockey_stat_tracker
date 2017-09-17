/*
 * Project IoTHockey
 * Description: IoT Hockey stat tracker
 * Author: Kevin Sidwar
 * Date: September 14, 2017
 * License: MIT
 */

#define CLEAR_COMMAND   0x76
#define CURSOR_COMMAND  0x79

void initializeScreen()
{
  Serial1.begin(9600);
}

void writeToScreen(byte b)
{
  Serial1.write(b);
}

void writeToScreen(String s)
{
  for (int i = 0; i < s.length(); i++)
  {
    Serial1.write(s.charAt(i));
  }
}

void writeToScreen(byte bytes[], int len)
{
  Serial1.write(bytes, len);
}

void clearDisplay()
{
  writeToScreen(CLEAR_COMMAND);
}

void setCursorPosition(int position)
{
  byte commands[] = {CURSOR_COMMAND, position};
  writeToScreen(commands, 2);
}

int updateTrackedStat(String statData)
{
  clearDisplay();
  if(statData.length() > 4)
    statData = statData.substring(0, 4);

  setCursorPosition(4 - statData.length());
  writeToScreen(statData);
  return 0;
}

void setup() {
  initializeScreen();
  clearDisplay();
  Particle.function("updateStat", updateTrackedStat);
}

void loop() {
}
