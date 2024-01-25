/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    time_utils.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
#include <stdio.h>
#include <stdlib.h>
#include "time_utils.h"

const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
const char WeekdayNames[]  = "ErrSunMonTueWedThuFriSat";
const uint8_t WeekdayNameLength = 3;


void breakTime(uint32_t time, tmelements_t * tm){  // break time_t into elements
// break the given time into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 2000 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time_temp;
  unsigned long days;

  time_temp = time;
  tm->Second = time_temp % 60;
  time_temp /= 60; // now it is minutes
  tm->Minute = time_temp % 60;
  time_temp /= 60; // now it is hours
  tm->Hour = time_temp % 24;
  time_temp /= 24; // now it is days
  tm->Wday = ((time_temp + 6) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time_temp) {
    year++;
  }
  tm->Year = year; // year is offset from 2000
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time_temp  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time_temp >= monthLength) {
      time_temp -= monthLength;
    } else {
        break;
    }
  }
  tm->Month = month + 1;  // jan is month 1  
  tm->Day = time_temp + 1;     // day of month
 
    
}



uint32_t makeTime(tmelements_t * tm){  // convert time elements into time_t
// assemble time elements into time_t 
// note year argument is offset from 2000 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 2000 till 1 jan 00:00:00 of the given year
  seconds= tm->Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm->Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm->Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm->Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm->Day-1) * SECS_PER_DAY;
  seconds+= tm->Hour * SECS_PER_HOUR;
  seconds+= tm->Minute * SECS_PER_MIN;
  seconds+= tm->Second;
  return seconds;     

}


uint8_t StringToUint8(const char* pString)
{
    uint8_t value = 0;

    // skip leading 0 and spaces
    while ('0' == *pString || *pString == ' ')
    {
        pString++;
    }

    // calculate number until we hit non-numeral char
    while ('0' <= *pString && *pString <= '9')
    {
        value *= 10;
        value += *pString - '0';
        pString++;
    }
    return value;
}

void RtcDateTime(tmelements_t * tm, const char * date, const char * time){
    //From RTC by Makuna
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    
    //first, calculate the year from 2000
    tm->Year = StringToUint8(date + 9);
    
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0])
    {
        case 'J':
        if ( date[1] == 'a' )
	        tm->Month = 1;
	    else if ( date[2] == 'n' )
	        tm->Month = 6;
	    else
	        tm->Month = 7;
        break;
    case 'F':
        tm->Month = 2;
        break;
    case 'A':
        tm->Month = date[1] == 'p' ? 4 : 8;
        break;
    case 'M':
        tm->Month = date[2] == 'r' ? 3 : 5;
        break;
    case 'S':
        tm->Month = 9;
        break;
    case 'O':
        tm->Month = 10;
        break;
    case 'N':
        tm->Month = 11;
        break;
    case 'D':
        tm->Month = 12;
        break;
    }
    
    tm->Day = StringToUint8(date + 4);
    tm->Hour = StringToUint8(time);
    tm->Minute = StringToUint8(time + 3);
    tm->Second = StringToUint8(time + 6);
    
    //Calculate day of week too
    breakTime(makeTime(tm),tm);
    
}


/* *****************************************************************************
 End of File
 */
