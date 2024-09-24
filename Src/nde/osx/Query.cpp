#include "nde.h"
#include "NDEString.h"
#include "Query.h"

//---------------------------------------------------------------------------

BOOL Scanner::Query(const char *query)
{
	if (!query) return FALSE;
	if (last_query) CFRelease(last_query);
	last_query = CFStringCreateWithBytes(NULL, (const uint8_t *)query, strlen(query), kCFStringEncodingUTF32, false);
	RemoveFilters();
	in_query_parser = 1;
	BOOL r = Query_Parse(query);
	
	if (r == FALSE)
	{
		if (!disable_date_resolution) RemoveFilters();
		last_query_failed = TRUE;
	}
	in_query_parser = 0;
	Query_CleanUp();
	return r & CheckFilters();
}

CFStringRef Scanner::GetLastQuery()
{
	return last_query;
}

typedef struct
{
	CFStringRef token;
	int tid;
} tokenstruct;

tokenstruct Tokens[] =   // Feel free to add more...
{
	{CFSTR("AND"), TOKEN_AND },
	{CFSTR("OR"), TOKEN_OR },
	{CFSTR("HAS"), TOKEN_CONTAINS },
	{CFSTR("NOTHAS"),TOKEN_NOTCONTAINS},
	{CFSTR("BEGINS"), TOKEN_BEGINS },
	{CFSTR("ENDS"), TOKEN_ENDS },
	{CFSTR("ISEMPTY"), TOKEN_ISEMPTY},
	{CFSTR("ISNOTEMPTY"),TOKEN_ISNOTEMPTY},
	{CFSTR("LIKE"), TOKEN_LIKE},
	{CFSTR("BEGINSLIKE"), TOKEN_BEGINSLIKE},
};


typedef struct
{
	int Op;
	int Level;
} OpLevel;


static int Query_ParseLength(CFStringRef str)
{
	int x;
	CFArrayRef array = CFStringCreateArrayBySeparatingStrings(NULL, str, CFSTR(":"));
	if (array)
	{
		CFIndex count = CFArrayGetCount(array);
		if (count == 2)
		{
			CFStringRef t1 = (CFStringRef)CFArrayGetValueAtIndex(array, 0);
			CFStringRef t2 = (CFStringRef)CFArrayGetValueAtIndex(array, 1);
			x = CFStringGetIntValue(t1) * 60 + CFStringGetIntValue(t2);
			CFRelease(array);
			return x;
		}
		
		if (count == 3)
		{
			CFStringRef t1 = (CFStringRef)CFArrayGetValueAtIndex(array, 0);
			CFStringRef t2 = (CFStringRef)CFArrayGetValueAtIndex(array, 1);
			CFStringRef t3 = (CFStringRef)CFArrayGetValueAtIndex(array, 2);
			x = CFStringGetIntValue(t1) * 60 * 60 + CFStringGetIntValue(t2) * 60 + CFStringGetIntValue(t3);
			CFRelease(array);
			return x;
		}
		CFRelease(array);
	}
	return CFStringGetIntValue(str);;
}

/*
    our state machine

                               &, |
            ----------<-------------------------<----------------------<-----------
           |                                                                       |
           v    ID (Col)        =, >, <...       ID / Data / [f]          )        |
     ---->(0) ----->-----> (1) ------>-----> (2) ------>------> (3) ------>-----> (4) <--
    |     |^                \isempty------------->------------/  |^                |     |
    | !(  ||                                                     ||----            |  )  |
     --<--  ---------<---------------------------<-------------<-|     |            -->--
                               &, |                              v [f] |
                                                                  -->--

*/

//---------------------------------------------------------------------------
BOOL Scanner::Query_Parse(const char *query)
{
	const char *p = query; // pointer on next token to read
	size_t size;
	int state = 0;
	int pcount = 0;
	VListEntry<OpLevel> *entry;

	if (pstack.GetNElements() > 0)
		Query_CleanUp();

	while (1)
	{
		p = Query_EatSpace(p);
		int t = Query_GetNextToken(p, &size, &token);
		if (t == TOKEN_UNKNOWN)
		{
			Query_SyntaxError((int)(p-query));
			return FALSE;
		}
		if (t == TOKEN_EOQ)
			break;
		switch (state)
		{
		case 0:
			switch (t)
			{
			case TOKEN_PAROPEN:
				state = 0;
				// check too many parenthesis open
				if (pcount == 255)
				{
					Query_SyntaxError((int)(p-query)); // should not be _syntax_ error
					return FALSE;
				}
				// up one level
				pcount++;
				break;
			case TOKEN_NOT:
				// push not in this level
				OpLevel o;
				o.Op = FILTER_NOT;
				o.Level = pcount;
				entry = new VListEntry<OpLevel>;
				entry->SetVal(o);
				pstack.AddEntry(entry, TRUE);
				state = 0;
				break;
			case TOKEN_IDENTIFIER:
				state = 1;
				// create filter column

				if (AddFilterByName(token, NULL, FILTER_NONE) == ADDFILTER_FAILED)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}

				break;
			default:
				Query_SyntaxError((int)(p-query));
				return FALSE;
			}
			break;
		case 1:
			switch (t)
			{
			case TOKEN_EQUAL:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_EQUALS);
				break;
			}
			case TOKEN_ABOVE:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_ABOVE);
				break;
			}
			case TOKEN_BELOW:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_BELOW);
				break;
			}
			case TOKEN_CONTAINS:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_CONTAINS);
				break;
			}
			case TOKEN_NOTCONTAINS:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_NOTCONTAINS);
				break;
			}
			case TOKEN_AOREQUAL:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_ABOVEOREQUAL);
				break;
			}
			case TOKEN_BOREQUAL:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_BELOWOREQUAL);
				break;
			}
			case TOKEN_NOTEQUAL:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_NOTEQUALS);
				break;
			}
			case TOKEN_BEGINS:
			{
				state = 2;
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_BEGINS);
			}
			break;
			case TOKEN_ENDS:
			{
				state = 2;
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_ENDS);
			}
			break;
			case TOKEN_LIKE:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_LIKE);
				break;
			}
			case TOKEN_BEGINSLIKE:
			{
				state = 2;
				// set filter op
				Filter *f = GetLastFilter();
				f->SetOp(FILTER_BEGINSLIKE);
				break;
			}
			case TOKEN_ISNOTEMPTY:
			case TOKEN_ISEMPTY:
			{
				state = 3;
				Filter *f = GetLastFilter();
				f->SetOp(t==TOKEN_ISEMPTY ? FILTER_ISEMPTY : FILTER_ISNOTEMPTY);
				// pop all operators in this level beginning by the last inserted
				entry = (VListEntry<OpLevel> *)pstack.GetFoot();
				while (entry)
				{
					if (entry->GetVal().Level == pcount)
					{
						AddFilterOp(entry->GetVal().Op);
						VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
						pstack.RemoveEntry(entry);
						entry = _entry;
					}
					else
						break;
				}
			}
			break;
			default:
				Query_SyntaxError((int)(p-query));
				return FALSE;
			}
			break;
		case 2:
			if (t == TOKEN_SQBRACKETOPEN)
			{
				state = 3;
				const char *s = strchr(p, ']');
				if (!s)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				p = Query_EatSpace(p);
				if (*p == '[') p++;

				CFStringRef format = CFStringCreateWithBytes(NULL, (const uint8_t *)p, (s-p), kCFStringEncodingUTF32, false); 
				Filter *f = GetLastFilter();
				int id = f->GetId();
				ColumnField *c = GetColumnById(id);
				int tt = c ? c->GetDataType() : -1;
				if (disable_date_resolution || !c || (tt != FIELD_INTEGER && tt != FIELD_DATETIME && tt != FIELD_LENGTH && tt != FIELD_BOOLEAN))
				{

					if (disable_date_resolution)
					{
						StringField *field = (StringField *)f->Data();

						if (!field)
						{
							// format was used without a value, assume value is 0
							f->SetData(new StringField(CFSTR("")));
							entry = (VListEntry<OpLevel> *)pstack.GetFoot();
							while (entry)
							{
								if (entry->GetVal().Level == pcount)
								{
									AddFilterOp(entry->GetVal().Op);
									VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
									pstack.RemoveEntry(entry);
									entry = _entry;
								}
								else
									break;
							}
							field = (StringField*)f->Data();
						}

						field->SetNDEString(format);
						if (format) CFRelease(format);
						p = s+1;
						continue;
					}


					if (format) CFRelease(format);
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}

				IntegerField *field = (IntegerField *)f->Data();

				if (!field)
				{
					// format was used without a value, assume value is 0
					f->SetData(new IntegerField(0));
					entry = (VListEntry<OpLevel> *)pstack.GetFoot();
					while (entry)
					{
						if (entry->GetVal().Level == pcount)
						{
							AddFilterOp(entry->GetVal().Op);
							VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
							pstack.RemoveEntry(entry);
							entry = _entry;
						}
						else
							break;
					}
					field = (IntegerField *)f->Data();
				}

				// TODO: make sure this is safe (that p and s havn't been iterated)
				char *temp_format = (char *)malloc((s-p+1));
				strncpy(temp_format, p, s-p);
				temp_format[s-p] = 0;
				int r = field->ApplyConversion(temp_format);
				free(temp_format);

				ndestring_release(format);
				if (!r)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				p = s+1;
				continue;
			}
//        switch (t) {
			//      case TOKEN_IDENTIFIER:
			else   // JF> we make this relaxed, so anything is valid as a value
			{
				state = 3;
				// set filter data
				Filter *f = GetLastFilter();
				int id = f->GetId();
				ColumnField *c = GetColumnById(id);
				switch (c ? c->GetDataType() : -1)
				{
				case FIELD_DATETIME:
					if (disable_date_resolution)
						goto field_string_override;
				case FIELD_LENGTH:
				{
					int i;
					IntegerField *i_f = new IntegerField();
					i = Query_ParseLength(token);
					i_f->SetValue(i);
					f->SetData(i_f);
				}
				break;

				case FIELD_BOOLEAN:
				case FIELD_INTEGER:
				{
					int i;
					IntegerField *i_f = new IntegerField();
					i = CFStringGetIntValue(token);
					i_f->SetValue(i);
					f->SetData(i_f);
				}
				break;
				case FIELD_INT64:
				{
					int64_t i;
					Int64Field *i_f = new Int64Field();
					#ifdef _WIN32
					i = _wtoi64(token); // todo: Replace with own conversion and error checking
					#else
					i = CFStringGetIntValue(token); // TODO!!! 64bit integer here ... maybe use CFNumberFormatterCreateNumberFromString but it's a lot of overhead
					#endif
					i_f->SetValue(i);
					f->SetData(i_f);
				}
				break;
				case FIELD_FILENAME:
					{
						FilenameField *s_f = new FilenameField();
						s_f->SetNDEString(token);
						f->SetData(s_f);
					}
					break;
				case FIELD_STRING:
field_string_override:
					{
						StringField *s_f = new StringField();
						s_f->SetNDEString(token);
						f->SetData(s_f);
					}
					break;
				default:
					Query_SyntaxError((int)(p-query));
					return FALSE;
					break;

				}
				// pop all operators in this level beginning by the last inserted
				entry = (VListEntry<OpLevel> *)pstack.GetFoot();
				while (entry)
				{
					if (entry->GetVal().Level == pcount)
					{
						AddFilterOp(entry->GetVal().Op);
						VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
						pstack.RemoveEntry(entry);
						entry = _entry;
					}
					else
						break;
				}
				break;
			}
//          default:
			//          Query_SyntaxError(p-query);
			//        return FALSE;
//        }
			break;
		case 3:
			switch (t)
			{
			case TOKEN_SQBRACKETOPEN:
			{
				const char *s = strchr(p, ']');
				if (!s)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				p = Query_EatSpace(p);
				if (*p == '[') p++;
				CFStringRef format = CFStringCreateWithBytes(NULL, (const uint8_t *)p, (s-p), kCFStringEncodingUTF32, false); 
				Filter *f = GetLastFilter();
				int id = f->GetId();
				ColumnField *c = GetColumnById(id);
				int tt = c ? c->GetDataType() : -1;
				if (disable_date_resolution || !c || (tt != FIELD_INTEGER && tt != FIELD_DATETIME && tt != FIELD_LENGTH && tt != FIELD_BOOLEAN))
				{
					if (disable_date_resolution)
					{
						StringField *field = (StringField *)f->Data();

						if (!field)
						{
							// format was used without a value, assume value is 0
							f->SetData(new StringField(CFSTR("")));
							entry = (VListEntry<OpLevel> *)pstack.GetFoot();
							while (entry)
							{
								if (entry->GetVal().Level == pcount)
								{
									AddFilterOp(entry->GetVal().Op);
									VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
									pstack.RemoveEntry(entry);
									entry = _entry;
								}
								else
									break;
							}
							field = (StringField *)f->Data();
						}

						field->SetNDEString(format);
						if (format) CFRelease(format);
						p = s+1;
						continue;
					}
					if (format) CFRelease(format);
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}

				IntegerField *field = (IntegerField *)f->Data();

				if (!field)
				{
					// format was used without a value, assume value is 0
					f->SetData(new IntegerField(0));
					entry = (VListEntry<OpLevel> *)pstack.GetFoot();
					while (entry)
					{
						if (entry->GetVal().Level == pcount)
						{
							AddFilterOp(entry->GetVal().Op);
							VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
							pstack.RemoveEntry(entry);
							entry = _entry;
						}
						else
							break;
					}
					field = (IntegerField *)f->Data();
				}

				// TODO: make sure this is safe (that p and s havn't been iterated)
				char *temp_format = (char *)malloc((s-p+1));
				strncpy(temp_format, p, s-p);
				temp_format[s-p] = 0;
				int r = field->ApplyConversion(temp_format);
				free(temp_format);

				ndestring_release(format);
				if (!r)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				p = s+1;
				continue;
			}
			break;

			case TOKEN_PARCLOSE:
				state = 4;
				// check parenthesis count
				if (pcount == 0)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				// down one level
				pcount--;
				// pop all operators in this level, beginning by the last inserted
				while (entry)
				{
					if (entry->GetVal().Level == pcount)
					{
						AddFilterOp(entry->GetVal().Op);
						VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
						pstack.RemoveEntry(entry);
						entry = _entry;
					}
					else
						break;
				}
				break;
			case TOKEN_AND:
			{
				state = 0;
				// push and
				OpLevel o;
				o.Op = FILTER_AND;
				o.Level = pcount;
				entry = new VListEntry<OpLevel>;
				entry->SetVal(o);
				pstack.AddEntry(entry, TRUE);
				break;
			}
			case TOKEN_OR:
			{
				state = 0;
				// push or
				OpLevel o;
				o.Op = FILTER_OR;
				o.Level = pcount;
				entry = new VListEntry<OpLevel>;
				entry->SetVal(o);
				pstack.AddEntry(entry, TRUE);
				break;
			}
			default:
				Query_SyntaxError((int)(p-query));
				return FALSE;
			}
			break;
		case 4:
			switch (t)
			{
			case TOKEN_AND:
			{
				state = 0;
				// push and
				OpLevel o;
				o.Op = FILTER_AND;
				o.Level = pcount;
				entry = new VListEntry<OpLevel>;
				entry->SetVal(o);
				pstack.AddEntry(entry, TRUE);
				break;
			}
			case TOKEN_OR:
			{
				state = 0;
				// push or
				OpLevel o;
				o.Op = FILTER_OR;
				o.Level = pcount;
				entry = new VListEntry<OpLevel>;
				entry->SetVal(o);
				pstack.AddEntry(entry, TRUE);
				break;
			}
			case TOKEN_PARCLOSE:
				state = 4;
				// check parenthesis count
				if (pcount == 0)
				{
					Query_SyntaxError((int)(p-query));
					return FALSE;
				}
				// down one level
				pcount--;
				// pop all operators in this level, beginning by the last inserted
				while (entry)
				{
					if (entry->GetVal().Level == pcount)
					{
						AddFilterOp(entry->GetVal().Op);
						VListEntry<OpLevel> *_entry = (VListEntry<OpLevel> *)entry->GetPrevious();
						pstack.RemoveEntry(entry);
						entry = _entry;
					}
					else
						break;
				}
				break;
			default:
				Query_SyntaxError((int)(p-query));
				return FALSE;
			}
			break;
		default:
			// Ahem... :/
			break;
		}
		p += size;
	}
	if (pcount > 0)
	{
		Query_SyntaxError((int)(p-query));
		return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------

void Scanner::Query_SyntaxError(int c)
{

}

//---------------------------------------------------------------------------
void Scanner::Query_CleanUp()
{
	while (pstack.GetNElements() > 0)
	{
		VListEntry<int> *e;
		e = (VListEntry<int> *)pstack.GetHead();
		pstack.RemoveEntry(e);
	}
}


//---------------------------------------------------------------------------
const char *Scanner::Query_EatSpace(const char *p)
{
	while (*p && *p == ' ') p++;
	return p;
}

//---------------------------------------------------------------------------
const char *Scanner::Query_ProbeNonAlphaNum(const char *p)
{
	int inquote=0;
	while (*p && (!Query_isControlChar(*p) || (inquote)))
	{
		if (*p == '\"')
		{
			if (!inquote)
				inquote = 1;
			else
				return p+1;
		}
		p++;
	}
	return p;
}

//---------------------------------------------------------------------------
int Scanner::Query_isControlChar(char p)
{
	switch (p)
	{
	case '&':
	case '|':
	case '!':
	case '(':
	case '[':
	case ')':
	case ']':
	case '>':
	case '<':
	case '=':
	case ',':
	case ' ':
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
char *Scanner::Query_ProbeAlphaNum(char *p)
{
	while (*p && Query_isControlChar(*p)) p++;
	return p;
}

//---------------------------------------------------------------------------
char *Scanner::Query_ProbeSpace(char *p)
{
	while (*p && *p != ' ') p++;
	return p;
}

//---------------------------------------------------------------------------
int Scanner::Query_LookupToken(CFStringRef t)
{
	for (int i=0;i<sizeof(Tokens)/sizeof(tokenstruct);i++)
	{
		if (CFStringCompare(Tokens[i].token, t, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			return Tokens[i].tid;
	}
	return TOKEN_IDENTIFIER;
}

//---------------------------------------------------------------------------
int Scanner::Query_GetNextToken(const char *p, size_t *size, CFStringRef *_token, int tokentable)
{
	
	int t = TOKEN_EOQ;
	const char *startptr = p;

	if (!*p) return TOKEN_EOQ;

	p = Query_EatSpace(p);

	const char *e = Query_ProbeNonAlphaNum(p);

	if (e != p)   // We have a word
	{
		if (*_token) CFRelease(*_token);
		
		// check for a quoted string
		if (*p == '\"' && e != p+1 && e[-1] == '\"')
		{
			CFStringRef inner_string = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)(p+1), (e-p-2), kCFStringEncodingUTF8, false);	

			// escape it (ugh)
			CFStringRef escaped_string = CFURLCreateStringByReplacingPercentEscapes(NULL, inner_string, CFSTR(""));
			if (escaped_string)
			{
				*_token = escaped_string;
				CFRelease(inner_string);
			}
			else 
			{
				*_token = inner_string;					
			}
		}
		else 
		{
			*_token = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)p, (e-p), kCFStringEncodingUTF8, false);
		}

		switch (tokentable)
		{
			case -1:
				t = TOKEN_IDENTIFIER;
				break;
			case 0:
				t = Query_LookupToken(*_token);
				break;
			case 1:
				t = IntegerField::LookupToken(*_token);
		}
		p = e;
	}
	else   // We have a symbol
	{
		switch (*p)
		{
			case '&':
				if (*(p+1) == '&') p++;
				t = TOKEN_AND;
				break;
			case '|':
				if (*(p+1) == '|') p++;
				t = TOKEN_OR;
				break;
			case '!':
				if (*(p+1) == '=')
				{
					p++;
					t = TOKEN_NOTEQUAL;
					break;
				}
				t = TOKEN_NOT;
				break;
			case '(':
				t = TOKEN_PAROPEN;
				break;
			case ')':
				t = TOKEN_PARCLOSE;
				break;
			case '[':
				t = TOKEN_SQBRACKETOPEN;
				break;
			case ']':
				t = TOKEN_SQBRACKETCLOSE;
				break;
			case ',':
				t = TOKEN_COMMA;
				break;
			case '>':
				if (*(p+1) == '=')
				{
					p++;
					t = TOKEN_AOREQUAL;
					break;
				}
				if (*(p+1) == '<')
				{
					p++;
					t = TOKEN_NOTEQUAL;
					break;
				}
				t = TOKEN_ABOVE;
				break;
			case '<':
				if (*(p+1) == '=')
				{
					p++;
					t = TOKEN_BOREQUAL;
					break;
				}
				if (*(p+1) == '>')
				{
					p++;
					t = TOKEN_NOTEQUAL;
					break;
				}
				t = TOKEN_BELOW;
				break;
			case '=':
				if (*(p+1) == '>')
				{
					p++;
					t = TOKEN_AOREQUAL;
					break;
				}
				if (*(p+1) == '<')
				{
					p++;
					t = TOKEN_BOREQUAL;
					break;
				}
				if (*(p+1) == '!')
				{
					p++;
					t = TOKEN_NOTEQUAL;
					break;
				}
				if (*(p+1) == '=') p++;
				t = TOKEN_EQUAL;
				break;
			default:
				t = TOKEN_UNKNOWN;
				break;
		}
		p++;
	}
	
	*size = (int)(p - startptr);
	return t;
}

