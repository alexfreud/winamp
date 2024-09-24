/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 IntegerField Class
 
 --------------------------------------------------------------------------- */

#include "nde.h"
#include "Query.h"
#include <time.h>
#ifdef _WIN32
#include <malloc.h> // for alloca
#endif
//---------------------------------------------------------------------------
IntegerField::IntegerField(int Val)
{
	InitField();
	Type = FIELD_INTEGER;
	Value = Val;
}

//---------------------------------------------------------------------------
void IntegerField::InitField(void)
{
	Type = FIELD_INTEGER;
	Value=0;
}

//---------------------------------------------------------------------------
IntegerField::IntegerField()
{
	InitField();
}

//---------------------------------------------------------------------------
IntegerField::~IntegerField()
{
}

//---------------------------------------------------------------------------
void IntegerField::ReadTypedData(const uint8_t *data, size_t len)
{
	CHECK_INT(len);
	Value = *((int *)data);
}

//---------------------------------------------------------------------------
void IntegerField::WriteTypedData(uint8_t *data, size_t len)
{
	CHECK_INT(len);
	*((int *)data) = Value;
}

//---------------------------------------------------------------------------
int IntegerField::GetValue(void)
{
	return Value;
}

//---------------------------------------------------------------------------
void IntegerField::SetValue(int Val)
{
	Value = Val;
}


#include <limits.h>
//---------------------------------------------------------------------------
size_t IntegerField::GetDataSize(void)
{
	return 4;
}

//---------------------------------------------------------------------------
int IntegerField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	return GetValue() < ((IntegerField*)Entry)->GetValue() ? -1 : (GetValue() > ((IntegerField*)Entry)->GetValue() ? 1 : 0);
}

//---------------------------------------------------------------------------
bool IntegerField::ApplyFilter(Field *Data, int op)
{
	bool r;
	switch (op)
	{
		case FILTER_EQUALS:
			r = Value == ((IntegerField *)Data)->GetValue();
			break;
		case FILTER_NOTEQUALS:
			r = Value != ((IntegerField *)Data)->GetValue();
			break;
		case FILTER_NOTCONTAINS:
			r = (bool)!(Value & ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_CONTAINS:
			r = !!(Value & ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_ABOVE:
			r = (bool)(Value > ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_BELOW:
			r = (bool)(Value < ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_BELOWOREQUAL:
			r = (bool)(Value <= ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_ABOVEOREQUAL:
			r = (bool)(Value >= ((IntegerField *)Data)->GetValue());
			break;
		case FILTER_ISEMPTY:
			r = (Value == 0 || Value == -1);
			break;
		case FILTER_ISNOTEMPTY:
			r = !(Value == 0 || Value == -1);
			break;
		default:
			r = true;
			break;
	}
	return r;
}

//---------------------------------------------------------------------------
typedef struct {
  CFStringRef token;
  int tid;
} tokenstruct;

enum {
  TOKEN_AGO = 128,
  TOKEN_NOW,
  TOKEN_YESTERDAY,
  TOKEN_TOMORROW,
  TOKEN_TODAY,
  TOKEN_OF,
  TOKEN_THE,
  TOKEN_DATE,
  TOKEN_FROM,
  TOKEN_BEFORE,
  TOKEN_AFTER,
  TOKEN_THIS,
  TOKEN_SUNDAY,
  TOKEN_MONDAY,
  TOKEN_TUESDAY,
  TOKEN_WEDNESDAY,
  TOKEN_THURSDAY,
  TOKEN_FRIDAY,
  TOKEN_SATURDAY,
	
  TOKEN_MIDNIGHT,
  TOKEN_NOON,
	
  TOKEN_AM,
  TOKEN_PM,
	
  TOKEN_JANUARY,
  TOKEN_FEBRUARY,
  TOKEN_MARCH,
  TOKEN_APRIL,
  TOKEN_MAY,
  TOKEN_JUNE,
  TOKEN_JULY,
  TOKEN_AUGUST,
  TOKEN_SEPTEMBER,
  TOKEN_OCTOBER,
  TOKEN_NOVEMBER,
  TOKEN_DECEMBER,
	
  TOKEN_TIME,
  TOKEN_SECOND,
  TOKEN_MINUTE,
  TOKEN_HOUR,
  TOKEN_DAY,
  TOKEN_WEEK,
  TOKEN_MONTH,
  TOKEN_YEAR,
  TOKEN_AT,
};

tokenstruct Int_Tokens[] = { // Feel free to add more...
	{CFSTR("ago"), TOKEN_AGO},
	{CFSTR("now"), TOKEN_NOW},
	{CFSTR("am"), TOKEN_AM},
	{CFSTR("pm"), TOKEN_PM},
	{CFSTR("this"), TOKEN_THIS},
	{CFSTR("date"), TOKEN_DATE},
	{CFSTR("time"), TOKEN_TIME},
	{CFSTR("of"), TOKEN_OF},
	{CFSTR("at"), TOKEN_AT},
	{CFSTR("the"), TOKEN_THE},
	{CFSTR("yesterday"), TOKEN_YESTERDAY},
	{CFSTR("tomorrow"), TOKEN_TOMORROW},
	{CFSTR("today"), TOKEN_TODAY},
	{CFSTR("from"), TOKEN_FROM},
	{CFSTR("before"), TOKEN_BEFORE},
	{CFSTR("after"), TOKEN_AFTER},
	{CFSTR("past"), TOKEN_AFTER},
	{CFSTR("monday"), TOKEN_MONDAY},
	{CFSTR("mon"), TOKEN_MONDAY},
	{CFSTR("tuesday"), TOKEN_TUESDAY},
	{CFSTR("tue"), TOKEN_TUESDAY},
	{CFSTR("wednesday"), TOKEN_WEDNESDAY},
	{CFSTR("wed"), TOKEN_WEDNESDAY},
	{CFSTR("thursday"), TOKEN_THURSDAY},
	{CFSTR("thu"), TOKEN_THURSDAY},
	{CFSTR("friday"), TOKEN_FRIDAY},
	{CFSTR("fri"), TOKEN_FRIDAY},
	{CFSTR("saturday"), TOKEN_SATURDAY},
	{CFSTR("sat"), TOKEN_SATURDAY},
	{CFSTR("sunday"), TOKEN_SUNDAY},
	{CFSTR("sun"), TOKEN_SUNDAY},
	{CFSTR("midnight"), TOKEN_MIDNIGHT},
	{CFSTR("noon"), TOKEN_NOON},
	{CFSTR("second"), TOKEN_SECOND},
	{CFSTR("seconds"), TOKEN_SECOND},
	{CFSTR("sec"), TOKEN_SECOND},
	{CFSTR("s"), TOKEN_SECOND},
	{CFSTR("minute"), TOKEN_MINUTE},
	{CFSTR("minutes"), TOKEN_MINUTE},
	{CFSTR("min"), TOKEN_MINUTE},
	{CFSTR("mn"), TOKEN_MINUTE},
	{CFSTR("m"), TOKEN_MINUTE},
	{CFSTR("hour"), TOKEN_HOUR},
	{CFSTR("hours"), TOKEN_HOUR},
	{CFSTR("h"), TOKEN_HOUR},
	{CFSTR("day"), TOKEN_DAY},
	{CFSTR("days"), TOKEN_DAY},
	{CFSTR("d"), TOKEN_DAY},
	{CFSTR("week"), TOKEN_WEEK},
	{CFSTR("weeks"), TOKEN_WEEK},
	{CFSTR("w"), TOKEN_WEEK},
	{CFSTR("month"), TOKEN_MONTH},
	{CFSTR("months"), TOKEN_MONTH},
	{CFSTR("year"), TOKEN_YEAR},
	{CFSTR("years"), TOKEN_YEAR},
	{CFSTR("y"), TOKEN_YEAR},
	{CFSTR("january"), TOKEN_JANUARY},
	{CFSTR("jan"), TOKEN_JANUARY},
	{CFSTR("february"), TOKEN_FEBRUARY},
	{CFSTR("feb"), TOKEN_FEBRUARY},
	{CFSTR("march"), TOKEN_MARCH},
	{CFSTR("mar"), TOKEN_MARCH},
	{CFSTR("april"), TOKEN_APRIL},
	{CFSTR("apr"), TOKEN_APRIL},
	{CFSTR("may"), TOKEN_MAY},
	{CFSTR("june"), TOKEN_JUNE},
	{CFSTR("jun"), TOKEN_JUNE},
	{CFSTR("july"), TOKEN_JULY},
	{CFSTR("jul"), TOKEN_JULY},
	{CFSTR("august"), TOKEN_AUGUST},
	{CFSTR("aug"), TOKEN_AUGUST},
	{CFSTR("september"), TOKEN_SEPTEMBER},
	{CFSTR("sep"), TOKEN_SEPTEMBER},
	{CFSTR("october"), TOKEN_OCTOBER},
	{CFSTR("oct"), TOKEN_OCTOBER},
	{CFSTR("november"), TOKEN_NOVEMBER},
	{CFSTR("nov"), TOKEN_NOVEMBER},
	{CFSTR("december"), TOKEN_DECEMBER},
	{CFSTR("dec"), TOKEN_DECEMBER},
};

//---------------------------------------------------------------------------
int IntegerField::LookupToken(CFStringRef t) {
  for (int i=0;i<sizeof(Int_Tokens)/sizeof(tokenstruct);i++) {
	if (CFStringCompare(Int_Tokens[i].token, t, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
      return Int_Tokens[i].tid;
  }
  return TOKEN_IDENTIFIER;
}

static int isallnum(CFStringRef p) 
{
	// TODO: ideally we should cache this or create it during initialization (but need to b be careful about thread safety)
	CFCharacterSetRef set = CFCharacterSetCreateInvertedSet(NULL, CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit));

	CFRange range;
  Boolean ret = CFStringFindCharacterFromSet(p, set, CFRangeMake(0, CFStringGetLength(p)), 0, &range);
	CFRelease(set);
	return ret;
}

static bool Ends(CFStringRef str1, CFStringRef str2)
{
	
	CFRange findRange = CFStringFind(str1, str2, 	kCFCompareCaseInsensitive|kCFCompareAnchored|kCFCompareBackwards);
	if (findRange.location == kCFNotFound)
		return false;
	else
		return findRange.location != 0;
}

//---------------------------------------------------------------------------
int IntegerField::ApplyConversion(const char *format, TimeParse *tp) {
  size_t size;
	
  int value = GetValue();
  CFStringRef token = NULL;
  bool ago = false;
  bool from = false;
  bool kthis = false;
  int what = TOKEN_MINUTE;
	
  int lastnumber = value;
	
  if (tp) {
    tp->is_relative = 0; 
    tp->offset_value = 0;
    tp->offset_whence = -1;
    tp->offset_what = -1;
    tp->offset_used = 0;
    tp->relative_year = -1;
    tp->relative_month = -1;
    tp->relative_day = -1;
    tp->relative_hour = -1;
    tp->relative_min = -1;
    tp->relative_sec = -1;
    tp->relative_kwday = -1;
    tp->absolute_hastime = 0;
    tp->absolute_hasdate = 0;
  }
	
  time_t now;
  time(&now);
	
  struct tm *o = localtime(&now);
  struct tm origin = *o;
  struct tm origin_flags = {0,0,0,0,0,0,0,0,0};
	
  struct tm onow = *o;
	
  const char *p = format;
  int t = -1;
  int lastt = -1;
	
  origin.tm_isdst = -1;
	
  while (1) {
    int save_lastt = lastt;
    lastt = t;
    t = Scanner::Query_GetNextToken(p, &size, &token, 1);
    if (t == TOKEN_EOQ) break;
		
    switch (t) {
      case TOKEN_THIS:
        kthis = true;
        break;
      case TOKEN_AGO:
      case TOKEN_BEFORE: // before defaults to before now (= ago)
        ago = true;
        if (tp) {
          tp->is_relative = 1;
          tp->offset_whence = 1;
          tp->offset_used = 1;
        }
        break;
      case TOKEN_AFTER: // if after, ago is discarded, coz 5 mn ago after x has no meaning, so we get it as 5 mn after x
        ago = false;
        // no break
      case TOKEN_FROM:
        from = true;
        if (tp) { 
          tp->is_relative = 1;
          tp->offset_whence = 0;
          tp->offset_used = 1;
        }
        break;
      case TOKEN_DATE: {
        if (!kthis) break;
        kthis = false;
        origin.tm_year = onow.tm_year;
        origin_flags.tm_year = 1;
        origin.tm_mon = onow.tm_mon;
        origin_flags.tm_mon = 1;
        origin.tm_mday = onow.tm_mday - onow.tm_wday;
        origin_flags.tm_mday = 1;
        if (!origin_flags.tm_hour)
          origin.tm_hour = 0;
        if (!origin_flags.tm_min)
          origin.tm_min = 0;
        if (!origin_flags.tm_sec)
          origin.tm_sec = 0;
        if (tp) { 
          tp->relative_year = -1;
          tp->relative_month = -1;
          tp->relative_day = -1;
        }
        break;
      } 
      case TOKEN_TIME: {
        if (!kthis) break;
        kthis = false;
        origin.tm_hour = onow.tm_hour;
        origin_flags.tm_hour = 1;
        origin.tm_min = onow.tm_min;
        origin_flags.tm_min = 1;
        origin.tm_sec = onow.tm_sec;
        origin_flags.tm_sec = 1;
        if (tp) { 
          tp->relative_sec = -1;
          tp->relative_min = -1;
          tp->relative_hour = -1;
        }
        break;
      } 
      case TOKEN_SECOND: 
      case TOKEN_MINUTE: 
      case TOKEN_HOUR: 
      case TOKEN_DAY: 
      case TOKEN_WEEK: 
      case TOKEN_MONTH: 
      case TOKEN_YEAR: 
        if (kthis) {
          kthis = false;
          switch (t) {
            case TOKEN_SECOND: 
              origin.tm_sec = onow.tm_sec;
              origin_flags.tm_sec = 1;
              if (tp) tp->relative_sec = -1;
              break;
            case TOKEN_MINUTE: 
              origin.tm_min = onow.tm_min;
              origin_flags.tm_min = 1;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_min = -1;
              break;
            case TOKEN_HOUR: 
              origin.tm_hour = onow.tm_hour;
              origin_flags.tm_hour = 1;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_hour = -1;
              break;
            case TOKEN_DAY: 
              origin.tm_mday = onow.tm_mday;
              origin_flags.tm_mday = 1;
              if (!origin_flags.tm_hour)
                origin.tm_hour = 0;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_day = -1;
              break;
            case TOKEN_WEEK: 
              origin.tm_mday = onow.tm_mday - onow.tm_wday;
              origin_flags.tm_mday = 1;
              if (!origin_flags.tm_hour)
                origin.tm_hour = 0;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_day = -2;
              break;
            case TOKEN_MONTH: 
              origin.tm_mon = onow.tm_mon;
              origin_flags.tm_mon = 1;
              if (!origin_flags.tm_mday)
                origin.tm_mday = 1;
              if (!origin_flags.tm_hour)
                origin.tm_hour = 0;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_month = -1;
              break;
            case TOKEN_YEAR: 
              origin.tm_year = onow.tm_year;
              origin_flags.tm_year = 1;
              if (!origin_flags.tm_mon)
                origin.tm_mon = 0;
              if (!origin_flags.tm_mday)
                origin.tm_mday = 1;
              if (!origin_flags.tm_hour)
                origin.tm_hour = 0;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_year = -1;
              break;
          }
          break;
        } 
        if (lastnumber > 0) {
          value = lastnumber;
          lastnumber = 0;
          if (tp) tp->offset_value = value;
        }
        what = t;
        if (tp) { 
          switch (what) {
            case TOKEN_SECOND: 
              tp->offset_what = 6; break;
            case TOKEN_MINUTE: 
              tp->offset_what = 5; break;
            case TOKEN_HOUR: 
              tp->offset_what = 4; break;
            case TOKEN_DAY: 
              tp->offset_what = 3; break;
            case TOKEN_WEEK: 
              tp->offset_what = 2; break;
            case TOKEN_MONTH: 
              tp->offset_what = 1; break;
            case TOKEN_YEAR: 
              tp->offset_what = 0; break;
          }
        }
        break;
      case TOKEN_SUNDAY:
      case TOKEN_MONDAY:
      case TOKEN_TUESDAY:
      case TOKEN_WEDNESDAY:
      case TOKEN_THURSDAY:
      case TOKEN_FRIDAY:
      case TOKEN_SATURDAY: {
        kthis = false;
        int dow = t-TOKEN_MONDAY;
        if (dow > onow.tm_mday)
          origin.tm_mday = 7 - (dow - onow.tm_mday);
        else
          origin.tm_mday = dow;
        origin_flags.tm_mday = 1;
        if (!origin_flags.tm_hour)
          origin.tm_hour = 0;
        if (!origin_flags.tm_min)
          origin.tm_min = 0;
        if (!origin_flags.tm_sec)
          origin.tm_sec = 0;
			}
        if (tp) tp->relative_kwday = t-TOKEN_SUNDAY;
        break;
      case TOKEN_MIDNIGHT:
        kthis = false;
        origin.tm_hour = 0;
        origin_flags.tm_hour = 1;
        if (!origin_flags.tm_min) {
          if (tp) tp->relative_min = 0;
          origin.tm_min = 0;
          origin_flags.tm_min = 1;
        }
        if (!origin_flags.tm_sec) {
          if (tp) tp->relative_sec = 0;
          origin.tm_sec = 0;
          origin_flags.tm_sec = 1;
        }
        if (tp) tp->relative_hour = 0;
        break;
      case TOKEN_NOON:
        kthis = false;
        origin.tm_hour = 12;
        origin_flags.tm_hour = 1;
        if (!origin_flags.tm_min) {
          if (tp) tp->relative_min = 0;
          origin.tm_min = 0;
          origin_flags.tm_min = 1;
        }
        if (!origin_flags.tm_sec) {
          if (tp) tp->relative_sec = 0;
          origin.tm_sec = 0;
          origin_flags.tm_sec = 1;
        }
        if (tp) tp->relative_hour = 12;
        break;
      case TOKEN_AM:
        kthis = false;
        if (lastnumber > 0) {
          origin.tm_hour = lastnumber;
          if (!origin_flags.tm_min) {
            if (tp) tp->relative_min = 0;
            origin.tm_min = 0;
            origin_flags.tm_min = 1;
          }
          if (!origin_flags.tm_sec) {
            if (tp) tp->relative_sec = 0;
            origin.tm_sec = 0;
            origin_flags.tm_sec = 1;
          }
          if (tp) tp->relative_hour = lastnumber;
          lastnumber = 0;
        } else {
          if (origin.tm_hour > 12) origin.tm_hour -= 12;
          if (tp) tp->relative_hour = origin.tm_hour;
        }
        origin_flags.tm_hour = 1;
				break;
      case TOKEN_PM: 
        kthis = false;
        if (lastnumber > 0) {
          origin.tm_hour = lastnumber > 12 ? lastnumber : lastnumber + 12;
          if (!origin_flags.tm_min) {
            if (tp) tp->relative_min = 0;
            origin.tm_min = 0;
            origin_flags.tm_min = 1;
          }
          if (!origin_flags.tm_sec) {
            if (tp) tp->relative_sec = 0;
            origin.tm_sec = 0;
            origin_flags.tm_sec = 1;
          }
          if (tp) tp->relative_hour = lastnumber;
          lastnumber = 0;
        } else {
          if (origin.tm_hour <= 12) origin.tm_hour += 12;
          if (tp) tp->relative_hour = origin.tm_hour;
        }
        origin_flags.tm_hour = 1;
				break;
      case TOKEN_NOW:
        kthis = false;
        if (!origin_flags.tm_year) {
          if (tp) tp->relative_year = -1;
          origin.tm_year = onow.tm_year;
        }
        origin_flags.tm_year = 1;
        if (!origin_flags.tm_mon) {
          if (tp) tp->relative_month = -1;
          origin.tm_mon = onow.tm_mon;
        }
        origin_flags.tm_mon = 1;
        if (!origin_flags.tm_mday) {
          if (tp) tp->relative_day = -1;
          origin.tm_mday = onow.tm_mday;
        }
        origin_flags.tm_mday = 1;
        if (!origin_flags.tm_hour) {
          if (tp) tp->relative_hour = -1;
          origin.tm_hour = onow.tm_hour;
        }
        origin_flags.tm_hour = 1;
        if (!origin_flags.tm_min) {
          if (tp) tp->relative_min = -1;
          origin.tm_min = onow.tm_min;
        }
        origin_flags.tm_min = 1;
        if (!origin_flags.tm_sec) {
          if (tp) tp->relative_sec = -1;
          origin.tm_sec = onow.tm_sec;
        }
        break;
      case TOKEN_YESTERDAY:
        kthis = false;
        origin.tm_mday = onow.tm_mday - 1;
        origin_flags.tm_mday = 1;
        if (tp) tp->relative_kwday = 7;
        break;
      case TOKEN_TODAY:
        origin.tm_mday = onow.tm_mday;
        origin_flags.tm_mday = 1;
        if (tp) tp->relative_kwday = 8;
        break;
      case TOKEN_TOMORROW:
        kthis = false;
        origin.tm_mday = onow.tm_mday + 1;
        origin_flags.tm_mday = 1;
        if (tp) tp->relative_kwday = 9;
        break;
      case TOKEN_JANUARY:
      case TOKEN_FEBRUARY:
      case TOKEN_MARCH:
      case TOKEN_APRIL:
      case TOKEN_MAY:
      case TOKEN_JUNE:
      case TOKEN_JULY:
      case TOKEN_AUGUST:
      case TOKEN_SEPTEMBER:
      case TOKEN_OCTOBER:
      case TOKEN_NOVEMBER:
      case TOKEN_DECEMBER:
        kthis = false;
        if (lastnumber > 0) {
          origin.tm_mday = lastnumber;
          origin_flags.tm_mday = 1;
          lastnumber = 0;
        } 
        origin.tm_mon = t-TOKEN_JANUARY;
        if (!origin_flags.tm_mday)
          origin.tm_mday = 1;
        if (!origin_flags.tm_hour)
          origin.tm_hour = 0;
        if (!origin_flags.tm_min)
          origin.tm_min = 0;
        if (!origin_flags.tm_sec)
          origin.tm_sec = 0;
        origin_flags.tm_mon = 1;
        if (tp) tp->relative_month = t-TOKEN_JANUARY;
        break;
      case TOKEN_IDENTIFIER: 
			{
        kthis = false;
				
				// check for a year value
		int i = CFStringGetIntValue(token);
        if (i > 1970 && i < 2038 && isallnum(token)) { // max time_t range
          origin.tm_year = i-1900;
          if (!origin_flags.tm_mday)
            origin.tm_mday = 1;
          if (!origin_flags.tm_mon)
            origin.tm_mon = 0;
          if (!origin_flags.tm_hour)
            origin.tm_hour = 0;
          if (!origin_flags.tm_min)
            origin.tm_min = 0;
          if (!origin_flags.tm_sec)
            origin.tm_sec = 0;
          if (tp) tp->relative_year = i;
          break;
        }
				
				// check for 1st, 2nd, 3rd, 4th, etc.
				if (Ends(token, CFSTR("st")) | Ends(token, CFSTR("nd")) | Ends(token, CFSTR("rd")) | Ends(token, CFSTR("th")))
				{
					int j = CFStringGetIntValue(token);
					if (j >= 1 && j <= 31) 
					{
						origin.tm_mday = j;
						origin_flags.tm_mday = 1;
						if (tp) tp->relative_day = j;
						break;
					}
				}
				
				// check for a time string (##:##:##)
#ifdef _WIN32
        z = wcschr(token, L':');
        if (z) 
				{
          if (tp) tp->absolute_hastime = 1;
          wchar_t *zz = wcschr(z+1, L':');
          int a, b, c=0;
          a = myatoi(token, (int)(z-token));
          if (zz && *(zz+1) == 0) zz = NULL;
          if (zz && !isallnum(zz+1)) zz = NULL;
          if (zz) { b = myatoi(z+1, (int)(zz-(z+1))); c = wcstol(zz+1,0,10); }
          else b = wcstol(z+1,0,10);
          origin.tm_hour = a;
          origin.tm_min = b;
          if (tp) {
            tp->relative_hour = a;
            tp->relative_min = b;
          }
          if (zz && !origin_flags.tm_sec) {
            origin.tm_sec = c;
            if (tp) tp->relative_sec = c;
          } else if (!origin_flags.tm_sec) {
            origin.tm_sec = 0;
          }
          origin_flags.tm_sec = 1;
          origin_flags.tm_hour = 1;
          origin_flags.tm_min = 1;
          break;
        }
#else
		// TODO!!! maybe CFDateFormatterGetAbsoluteTimeFromString ?
#endif
				
				// check for a date string in the format ##/##/##
#ifdef _WIN32
        z = wcschr(token, L'/');
        if (z) {
          if (tp) tp->absolute_hasdate = 1;
          wchar_t *zz = wcschr(z+1, L'/');
          int a, b, c=onow.tm_year;
          a = myatoi(token, (int)(z-token));
          if (zz && !isallnum(zz+1)) zz = NULL;
          if (zz && *(zz+1) == 0) zz = NULL;
          if (zz) { b = myatoi(z+1, (int)(zz-(z+1))); c = wcstol(zz+1,0,10); }
          else b = _wtoi(z+1);
          if (b > 1969 && b < 2038) {
            // mm/yyyy
            origin.tm_year = b-1900;
            origin_flags.tm_year = 1;
            origin.tm_mon = a-1;
            origin_flags.tm_mon = 1;
            if (!origin_flags.tm_mday)
              origin.tm_mday = 1;
            if (!origin_flags.tm_hour)
              origin.tm_hour = 0;
            if (!origin_flags.tm_min)
              origin.tm_min = 0;
            if (!origin_flags.tm_sec)
              origin.tm_sec = 0;
            if (tp) {
              tp->relative_year = b;
              tp->relative_month = a-1;
            }
          } else {
            // mm/dd(/yy[yy])
            if (c < 70) c += 100;
            if (c > 138) c -= 1900;
            origin.tm_year = c;
            origin.tm_mon = a-1;
            origin.tm_mday = b == 0 ? 1 : b;
            origin_flags.tm_year = 1;
            origin_flags.tm_mon = 1;
            origin_flags.tm_mday = 1;
            if (!origin_flags.tm_hour)
              origin.tm_hour = 0;
            if (!origin_flags.tm_min)
              origin.tm_min = 0;
            if (!origin_flags.tm_sec)
              origin.tm_sec = 0;
            if (tp) {
              tp->relative_year = c+1900;
              tp->relative_month = a-1;
              tp->relative_day = b;
            }
          }
          origin_flags.tm_year = 1;
          origin_flags.tm_mon = 1;
          origin_flags.tm_mday = 1;
          break;
        }
#else
		// TODO!!! maybe CFDateFormatterCreateDateFromString ?
#endif
				
        if (isallnum(token))
		{
          lastnumber = i;
          switch (lastt) {
            case TOKEN_JANUARY:
            case TOKEN_FEBRUARY:
            case TOKEN_MARCH:
            case TOKEN_APRIL:
            case TOKEN_MAY:
            case TOKEN_JUNE:
            case TOKEN_JULY:
            case TOKEN_AUGUST:
            case TOKEN_SEPTEMBER:
            case TOKEN_OCTOBER:
            case TOKEN_NOVEMBER:
            case TOKEN_DECEMBER:
              origin.tm_mday = lastnumber;
              origin_flags.tm_mday = 1;
              lastnumber = 0;
              if (!origin_flags.tm_hour)
                origin.tm_hour = 0;
              if (!origin_flags.tm_min)
                origin.tm_min = 0;
              if (!origin_flags.tm_sec)
                origin.tm_sec = 0;
              if (tp) tp->relative_day = lastnumber;
              break;
            case TOKEN_AT: {
              origin.tm_hour = lastnumber;
              origin.tm_min = 0;
              origin.tm_sec = 0;
              origin_flags.tm_hour = 1;
              origin_flags.tm_min = 1;
              origin_flags.tm_sec = 1;
              if (tp) {
                tp->relative_hour = lastnumber;
                tp->relative_min = 0;
                tp->relative_sec = 0;
              }
              lastnumber = 0;
              break;
            }
          }
        }
				
        break;
      }
      default:
        lastt = save_lastt;
        break;
    }
    p += size;
  }
	
  if (lastnumber) {
    switch (lastt) {
      case TOKEN_JANUARY:
      case TOKEN_FEBRUARY:
      case TOKEN_MARCH:
      case TOKEN_APRIL:
      case TOKEN_MAY:
      case TOKEN_JUNE:
      case TOKEN_JULY:
      case TOKEN_AUGUST:
      case TOKEN_SEPTEMBER:
      case TOKEN_OCTOBER:
      case TOKEN_NOVEMBER:
      case TOKEN_DECEMBER:
        origin.tm_mday = lastnumber;
        lastnumber = 0;
        if (!origin_flags.tm_hour)
          origin.tm_hour = 0;
        if (!origin_flags.tm_min)
          origin.tm_min = 0;
        if (!origin_flags.tm_sec)
          origin.tm_sec = 0;
        if (tp) tp->relative_day = lastnumber;
        break;
    }
  }
	
  if (ago) { // if ago (or before), from is optional since if it wasn't specified we use now
    switch (what) {
      case TOKEN_SECOND:
        origin.tm_sec -= value;
        break;
      case TOKEN_MINUTE:
        origin.tm_min -= value;
        break;
      case TOKEN_HOUR:
        origin.tm_hour -= value;
        break;
      case TOKEN_DAY:
        origin.tm_mday -= value;
        break;
      case TOKEN_WEEK:
        origin.tm_mday -= value*7;
        break;
      case TOKEN_MONTH:
        origin.tm_mon -= value;
        break;
      case TOKEN_YEAR:
        origin.tm_year -= value;
        break;
    }
    time_t o = mktime(&origin);
    SetValue(o);
	if (token) CFRelease(token);

    if (tp) tp->absolute_datetime = GetValue();
    return 1;
  } else if (from) { // from (or after) was specified, but not ago, 5 mn from x is x + 5 mn
    switch (what) {
      case TOKEN_SECOND:
        origin.tm_sec += value;
        break;
      case TOKEN_MINUTE:
        origin.tm_min += value;
        break;
      case TOKEN_HOUR:
        origin.tm_hour += value;
        break;
      case TOKEN_DAY:
        origin.tm_mday += value;
        break;
      case TOKEN_WEEK:
        origin.tm_mday += value*7;
        break;
      case TOKEN_MONTH:
        origin.tm_mon += value;
        break;
      case TOKEN_YEAR:
        origin.tm_year += value;
        break;
    }
    time_t o = mktime(&origin);
    SetValue(o);

	if (token) CFRelease(token);

    if (tp) tp->absolute_datetime = GetValue();
    return 1;
  } else { // none of ago/from/before/after were specified, just make a date/time with what we got and ignore our old value
    time_t o = mktime(&origin);
    SetValue(o);

	if (token) CFRelease(token);

    if (tp) tp->absolute_datetime = GetValue();
    return 1;
  }

  if (token) CFRelease(token);
  if (tp) tp->absolute_datetime = GetValue();
	
  return 0;
}

//---------------------------------------------------------------------------
DateTimeField::DateTimeField(int Val) : IntegerField(Val)
{
	Type = FIELD_DATETIME;
}

//---------------------------------------------------------------------------
DateTimeField::DateTimeField()
{
	Type = FIELD_DATETIME;
}

//---------------------------------------------------------------------------
DateTimeField::~DateTimeField()
{
}

//---------------------------------------------------------------------------
LengthField::LengthField(int Val) : IntegerField(Val)
{
	Type = FIELD_LENGTH;
}

//---------------------------------------------------------------------------
LengthField::LengthField()
{
	Type = FIELD_LENGTH;
}

//---------------------------------------------------------------------------
LengthField::~LengthField()
{
}
