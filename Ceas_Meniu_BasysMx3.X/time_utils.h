/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _TIME_UTILS_H    /* Guard against multiple inclusion */
#define _TIME_UTILS_H

#include <xc.h>

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif



// leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((2000+Y)>0) && !((2000+Y)%4) && ( ((2000+Y)%100) || !((2000+Y)%400) ) )
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k

typedef struct  { 
  uint8_t Second; 
  uint8_t Minute; 
  uint8_t Hour; 
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month; 
  uint8_t Year;   // offset from 2000; 
} tmelements_t;


/* low level functions to convert to and from system time                     */
void breakTime(uint32_t time, tmelements_t* tm);  // break time_t into elements
uint32_t makeTime(tmelements_t * tm);  // convert time elements into time_t

uint8_t StringToUint8(const char* pString);

void RtcDateTime(tmelements_t * tm, const char * date, const char * time);


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _TIME_UTILS_H */

/* *****************************************************************************
 End of File
 */
