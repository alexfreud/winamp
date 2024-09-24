/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 IntegerField Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __INTEGERFIELD_H
#define __INTEGERFIELD_H

#include <bfc/platform/types.h>

class TimeParse {
public:
  int is_relative;          // ago/after/before used
  int absolute_datetime;    // if not is_relative, this is the date/time, otherwise it's the resulting date/time if the query was ran now
  int absolute_hasdate;     // if not, use only the time portion of absolute_datetime, the rest is now
  int absolute_hastime;     // if not, use only the date portion of absolute_datetime, the rest is now
  int relative_year;        // -1 = this year
  int relative_month;       // -1 = this month
  int relative_day;         // -1 = this day, -2 = this week
  int relative_hour;        // -1 = this hour
  int relative_min;         // -1 = this minute
  int relative_sec;         // -1 = this second
  int relative_kwday;       // not used(-1), 0 to 6 for sunday to saturday, yesterday(7), today(8), tomorrow(9)
  int offset_value;         // timeago/offsetby numerical edit field
  int offset_what;          // not used(-1) years(0), months(1), weeks(2), days(3), hours(4), minutes(5), seconds(6)
  int offset_whence;        // not used(-1), after(0), ago/before(1)
  int offset_used;          // if 1, 'time ago' / 'offset by' should be checked
};

class IntegerField : public Field
	{
	protected:

		virtual void ReadTypedData(const uint8_t *, size_t len);
		virtual void WriteTypedData(uint8_t *, size_t len);
		virtual size_t GetDataSize(void);
		virtual int Compare(Field *Entry);
    virtual bool ApplyFilter(Field *Data, int op);
		void InitField(void);
		int Value;

    enum {
      WHAT_YEARS,
      WHAT_MONTHS,
      WHAT_WEEKS,
      WHAT_DAYS,
      WHAT_HOURS,
      WHAT_MINUTES,
      WHAT_SECONDS,
    };

    enum {
      FROM_BARE,
      FROM_AGO,
      FROM_SINCE,
    };

	public:

		~IntegerField();
		IntegerField(int);
		IntegerField();
		int GetValue(void);
		void SetValue(int);
		int ApplyConversion(const char *format, TimeParse *tp=NULL);

		 static int LookupToken(CFStringRef t);

	};


class DateTimeField : public IntegerField {
  public:
    DateTimeField();
    DateTimeField(int Val);
    virtual ~DateTimeField();
};

class LengthField : public IntegerField {
  public:
    LengthField();
    LengthField(int Val);
    virtual ~LengthField();
};

#endif