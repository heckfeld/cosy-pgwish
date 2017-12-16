/*
 *  Sammlung von Routinen zum Thema Datum und Uhrzeit.
 *  Mike 3.10.88
 */

#include  <stdio.h>
#include  <time.h>

static int day_tab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

day_of_year( year, month, day)
int year, month, day;
{
    int i, leap;

    leap = year%4 == 0 && year%100 != 0 != 0 || year%400 == 0;
    for( i=1; i<month; i++)
        day += day_tab[leap][i];
    return( day);
}

char *day_of_week( n)
int n;
{
    static char *name[] = {
        "???",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
        "Sun"
    };
    return((n<1 || n>7) ? name[0] : name[n]);
}

char *month_name( n)
int n;
{
    static char *name[] = {
        "???",
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };
    return((n<1 || n>12) ? name[0] : name[n]);
}
