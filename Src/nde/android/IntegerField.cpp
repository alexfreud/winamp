/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 IntegerField Class
 Windows implementation

Field data layout:
[4 bytes] value
 --------------------------------------------------------------------------- */

#include "../nde.h"
#include "Query.h"
#include <time.h>
#include <malloc.h> // for alloca

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
  const char *token;
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
  {"ago", TOKEN_AGO},
  {"now", TOKEN_NOW},
  {"am", TOKEN_AM},
  {"pm", TOKEN_PM},
  {"this", TOKEN_THIS},
  {"date", TOKEN_DATE},
  {"time", TOKEN_TIME},
  {"of", TOKEN_OF},
  {"at", TOKEN_AT},
  {"the", TOKEN_THE},
  {"yesterday", TOKEN_YESTERDAY},
  {"tomorrow", TOKEN_TOMORROW},
  {"today", TOKEN_TODAY},
  {"from", TOKEN_FROM},
  {"before", TOKEN_BEFORE},
  {"after", TOKEN_AFTER},
  {"past", TOKEN_AFTER},
  {"monday", TOKEN_MONDAY},
  {"mon", TOKEN_MONDAY},
  {"tuesday", TOKEN_TUESDAY},
  {"tue", TOKEN_TUESDAY},
  {"wednesday", TOKEN_WEDNESDAY},
  {"wed", TOKEN_WEDNESDAY},
  {"thursday", TOKEN_THURSDAY},
  {"thu", TOKEN_THURSDAY},
  {"friday", TOKEN_FRIDAY},
  {"fri", TOKEN_FRIDAY},
  {"saturday", TOKEN_SATURDAY},
  {"sat", TOKEN_SATURDAY},
  {"sunday", TOKEN_SUNDAY},
  {"sun", TOKEN_SUNDAY},
  {"midnight", TOKEN_MIDNIGHT},
  {"noon", TOKEN_NOON},
  {"second", TOKEN_SECOND},
  {"seconds", TOKEN_SECOND},
  {"sec", TOKEN_SECOND},
  {"s", TOKEN_SECOND},
  {"minute", TOKEN_MINUTE},
  {"minutes", TOKEN_MINUTE},
  {"min", TOKEN_MINUTE},
  {"mn", TOKEN_MINUTE},
  {"m", TOKEN_MINUTE},
  {"hour", TOKEN_HOUR},
  {"hours", TOKEN_HOUR},
  {"h", TOKEN_HOUR},
  {"day", TOKEN_DAY},
  {"days", TOKEN_DAY},
  {"d", TOKEN_DAY},
  {"week", TOKEN_WEEK},
  {"weeks", TOKEN_WEEK},
  {"w", TOKEN_WEEK},
  {"month", TOKEN_MONTH},
  {"months", TOKEN_MONTH},
  {"year", TOKEN_YEAR},
  {"years", TOKEN_YEAR},
  {"y", TOKEN_YEAR},
  {"january", TOKEN_JANUARY},
  {"jan", TOKEN_JANUARY},
  {"february", TOKEN_FEBRUARY},
  {"feb", TOKEN_FEBRUARY},
  {"march", TOKEN_MARCH},
  {"mar", TOKEN_MARCH},
  {"april", TOKEN_APRIL},
  {"apr", TOKEN_APRIL},
  {"may", TOKEN_MAY},
  {"june", TOKEN_JUNE},
  {"jun", TOKEN_JUNE},
  {"july", TOKEN_JULY},
  {"jul", TOKEN_JULY},
  {"august", TOKEN_AUGUST},
  {"aug", TOKEN_AUGUST},
  {"september", TOKEN_SEPTEMBER},
  {"sep", TOKEN_SEPTEMBER},
  {"october", TOKEN_OCTOBER},
  {"oct", TOKEN_OCTOBER},
  {"november", TOKEN_NOVEMBER},
  {"nov", TOKEN_NOVEMBER},
  {"december", TOKEN_DECEMBER},
  {"dec", TOKEN_DECEMBER},
};


//---------------------------------------------------------------------------
int IntegerField::LookupToken(const char *t) {
  for (int i=0;i<sizeof(Int_Tokens)/sizeof(tokenstruct);i++) {
    if (!strcasecmp(Int_Tokens[i].token, t))
      return Int_Tokens[i].tid;
  }
  return TOKEN_IDENTIFIER;
}

static int myatoi(const char *p, int len) {
	char *w = (char *)alloca((len+1)*sizeof(char));
	strncpy(w, p, len);
	w[len] = 0;
	int a = strtol(w,0, 10);
	//free(w);
	return a;
}

static int isallnum(const char *p) 
{
  	while (p && *p) {
    	if (*p < '0' || *p > '9') return 0;
    	p++;
  	}
  	return 1;
}

//---------------------------------------------------------------------------
int IntegerField::ApplyConversion(const char *format, TimeParse *tp) {
  int size;
	
  int value = GetValue();
  char *token = 0;
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
        int i = strtol(token,0,10);
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
				char *z;
				size_t tokenLen=strlen(token);
				if (tokenLen>=2)
				{
					z = token+tokenLen-2;
					if (!_stricmp(z, "st") || !_stricmp(z, "nd") || !_stricmp(z, "rd") || !_stricmp(z, "th")) {
						int j = myatoi(token, z-token);
						if (j >= 1 && j <= 31) {
							origin.tm_mday = j;
							origin_flags.tm_mday = 1;
							if (tp) tp->relative_day = j;
							break;
						}
					}
				}
				
				// check for a time string (##:##:##)
        z = strchr(token, ':');
        if (z) 
				{
          if (tp) tp->absolute_hastime = 1;
          char *zz = strchr(z+1, ':');
          int a, b, c=0;
          a = myatoi(token, (int)(z-token));
          if (zz && *(zz+1) == 0) zz = NULL;
          if (zz && !isallnum(zz+1)) zz = NULL;
          if (zz) { b = myatoi(z+1, (int)(zz-(z+1))); c = strtol(zz+1,0,10); }
          else b = strtol(z+1,0,10);
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
				
				// check for a date string in the format ##/##/##
        z = strchr(token, '/');
        if (z) {
          if (tp) tp->absolute_hasdate = 1;
          char *zz = strchr(z+1, '/');
          int a, b, c=onow.tm_year;
          a = myatoi(token, (int)(z-token));
          if (zz && !isallnum(zz+1)) zz = NULL;
          if (zz && *(zz+1) == 0) zz = NULL;
          if (zz) { b = myatoi(z+1, (int)(zz-(z+1))); c = strtol(zz+1,0,10); }
          else b = atoi(z+1);
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
    ndestring_release(token);

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

     ndestring_release(token);

    if (tp) tp->absolute_datetime = GetValue();
    return 1;
  } else { // none of ago/from/before/after were specified, just make a date/time with what we got and ignore our old value
    time_t o = mktime(&origin);
    SetValue(o);

    ndestring_release(token);

    if (tp) tp->absolute_datetime = GetValue();
    return 1;
  }
	ndestring_release(token);

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
