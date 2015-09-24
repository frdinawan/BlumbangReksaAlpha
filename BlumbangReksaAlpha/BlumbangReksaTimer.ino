//Call this function to include '0' before random number between 0-10

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

unsigned long deltaTime()
{
  unsigned long nowInSec, savedInSec, interval;
  nowInSec = DNow.d*24*3600 + TNow.h*3600 + TNow.m*60 + TNow.s;
  savedInSec = DSaved.d*24*3600 + TSaved.h*3600 + TSaved.m*60 + TSaved.s;
  interval = nowInSec - savedInSec;
  return interval;
}

unsigned long minToSec(unsigned long inMin)
{
  return inMin*60;
}

void writeTime(const char *str)
{
  int Hour, Min, Sec;

//  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec);
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
}

void writeDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

//  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  sscanf(str, "%s %d %d", Month, &Day, &Year);
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
//  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
}

void printTwoDigits(int number)
{
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
