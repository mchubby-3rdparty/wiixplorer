#include <time.h>
#include <math.h>

#include "tools.h"

bool TimePassed(int limit)
{
	time_t timer1 = 0;
	static time_t timer2;

	if (timer2 == 0) {
			time(&timer1);
		    timer2 = timer1;
    }

    if(timer1 != 0)	// timer l�uft
        time(&timer1);

    if(difftime(timer1, timer2) >= limit) {
        timer1 = 0;
        return true;
    }

    return false;
}

int ROUND2FOUR(double x)
{
    double tmp = 0.0;
    double rest = modf((double) x/4.0f, &tmp);
    int res = 0;

    if(rest >= 0.5)
    {
        res = ((int) x/4) +1;
    }
    else
    {
        res = (int) x/4;
    }


    return res*4;
}

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)
void ConvertNTFSDate(u64 ulNTFSDate,  TimeStruct * ptm)
{
    unsigned year, mon, day, hour, min, sec;
    u64 v64 = ulNTFSDate;
    u8 ms[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned temp;
    u32 v;
    v64 /= 10000000;
    sec = (unsigned)(v64 % 60);
    v64 /= 60;
    min = (unsigned)(v64 % 60);
    v64 /= 60;
    hour = (unsigned)(v64 % 24)+1;
    v64 /= 24;

    v = (u32)v64;

    year = (unsigned)(1601 + v / PERIOD_400 * 400);
    v %= PERIOD_400;

    temp = (unsigned)(v / PERIOD_100);
    if (temp == 4)
    temp = 3;
    year += temp * 100;
    v -= temp * PERIOD_100;

    temp = v / PERIOD_4;
    if (temp == 25)
    temp = 24;
    year += temp * 4;
    v -= temp * PERIOD_4;

    temp = v / 365;
    if (temp == 4)
    temp = 3;
    year += temp;
    v -= temp * 365;

    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
    ms[1] = 29;
    for (mon = 1; mon <= 12; mon++)
    {
    unsigned s = ms[mon - 1];
    if (v < s)
        break;
    v -= s;
    }
    day = (unsigned)v + 1;

    ptm->tm_mday = (u32)day;
    ptm->tm_mon =  (u32)mon;
    ptm->tm_year = (u32)year;

    ptm->tm_hour = (u32)hour;
    ptm->tm_min =  (u32)min;
    ptm->tm_sec =  (u32)sec;
}

void ConvertDosDate(u64 ulDosDate, TimeStruct * ptm)
{
    u32 uDate;
    uDate = (u32)(ulDosDate>>16);
    ptm->tm_mday = (u32)(uDate&0x1f) ;
    ptm->tm_mon =  (u32)((((uDate)&0x1E0)/0x20)) ;
    ptm->tm_year = (u32)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (u32) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (u32) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (u32) (2*(ulDosDate&0x1f)) ;
}
