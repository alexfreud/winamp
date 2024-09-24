#include "../nde.h"
#include "../NDEString.h"
#include "Query.h"

//---------------------------------------------------------------------------

bool Scanner::Query(const char *query)
{
	if (!query) return false;
	ndestring_release(last_query);
	last_query = ndestring_wcsdup(query);
	RemoveFilters();
	in_query_parser = 1;
	bool r = Query_Parse(query);
	
	if (r == false)
	{
		if (!disable_date_resolution) RemoveFilters();
		last_query_failed = true;
	}
	in_query_parser = 0;
	Query_CleanUp();
	return r & CheckFilters();
}

const char *Scanner::GetLastQuery()
{
	return last_query;
}


typedef struct
{
	const char *token;
	int tid;
} tokenstruct;

tokenstruct Tokens[] =   // Feel free to add more...
{
	{"AND", TOKEN_AND },
	{"OR", TOKEN_OR },
	{"HAS", TOKEN_CONTAINS },
	{"NOTHAS",TOKEN_NOTCONTAINS},
	{"BEGINS", TOKEN_BEGINS },
	{"ENDS", TOKEN_ENDS },
	{"ISEMPTY", TOKEN_ISEMPTY},
	{"ISNOTEMPTY",TOKEN_ISNOTEMPTY},
	{"LIKE", TOKEN_LIKE},
	{"BEGINSLIKE", TOKEN_BEGINSLIKE},
};


typedef struct
{
	int Op;
	int Level;
} OpLevel;

static int Query_ParseLength(const char *str)
{
	int i = atoi(str);
	
	const char *p;
	if ((p=strstr(str,":")))
	{
		i*=60;
		i+=atoi(++p);
		if ((p=strstr(p,":")))
		{
			i*=60;
			i+=atoi(++p);
		}
	}
	return i;
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
bool Scanner::Query_Parse(const char *query)
{
	const char *p = query; // pointer on next token to read
	int size;
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
			return false;
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
					return false;
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
				pstack.AddEntry(entry, true);
				state = 0;
				break;
			case TOKEN_IDENTIFIER:
				state = 1;
				// create filter column

				if (AddFilterByName(token, NULL, FILTER_NONE) == ADDFILTER_FAILED)
				{
					Query_SyntaxError((int)(p-query));
					return false;
				}

				break;
			default:
				Query_SyntaxError((int)(p-query));
				return false;
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
				return false;
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
					return false;
				}
				p = Query_EatSpace(p);
				if (*p == '[') p++;

				char *format = ndestring_malloc((s-p+1)*sizeof(char));
				strncpy(format, p, s-p);
				format[s-p] = 0;

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
							f->SetData(new StringField(""));
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
						ndestring_release(format);
						p = s+1;
						continue;
					}


					ndestring_release(format);
					Query_SyntaxError((int)(p-query));
					return false;
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
				int r = field->ApplyConversion(format);
				ndestring_release(format);
				if (!r)
				{
					Query_SyntaxError((int)(p-query));
					return false;
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
					i = atoi(token);
					i_f->SetValue(i);
					f->SetData(i_f);
				}
				break;
				case FIELD_INT64:
				{
					int64_t i;
					Int64Field *i_f = new Int64Field();
					i = strtoull(token, 0, 10); // todo: Replace with own conversion and error checking
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
					return false;
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
			//        return false;
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
					return false;
				}
				p = Query_EatSpace(p);
				if (*p == '[') p++;
				char *format = ndestring_malloc((s-p+1)*sizeof(char));
				strncpy(format, p, s-p);
				format[s-p] = 0;
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
							f->SetData(new StringField(""));
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
						ndestring_release(format);
						p = s+1;
						continue;
					}
					ndestring_release(format);
					Query_SyntaxError((int)(p-query));
					return false;
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
				int r = field->ApplyConversion(format);
				ndestring_release(format);
				if (!r)
				{
					Query_SyntaxError((int)(p-query));
					return false;
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
					return false;
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
				pstack.AddEntry(entry, true);
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
				pstack.AddEntry(entry, true);
				break;
			}
			default:
				Query_SyntaxError((int)(p-query));
				return false;
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
				pstack.AddEntry(entry, true);
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
				pstack.AddEntry(entry, true);
				break;
			}
			case TOKEN_PARCLOSE:
				state = 4;
				// check parenthesis count
				if (pcount == 0)
				{
					Query_SyntaxError((int)(p-query));
					return false;
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
				return false;
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
		return false;
	}
	return true;
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
		return true;
	}
	return false;
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
int Scanner::Query_LookupToken(const char *t)
{
	for (int i=0;i<sizeof(Tokens)/sizeof(tokenstruct);i++)
	{
		if (!_stricmp(Tokens[i].token, t))
			return Tokens[i].tid;
	}
	return TOKEN_IDENTIFIER;
}

//---------------------------------------------------------------------------

int Scanner::Query_GetNextToken(const char *p, int *size, char **_token, int tokentable)
{

	int t = TOKEN_EOQ;
	const char *startptr = p;

	if (!*p) return TOKEN_EOQ;

	p = Query_EatSpace(p);

	const char *e = Query_ProbeNonAlphaNum(p);

	if (e != p)   // We have a word
	{
		size_t token_length = e-p;
		if (*_token) ndestring_release(*_token);
		*_token = ndestring_wcsndup(p, token_length);
		if (*(*_token) == '\"' && (*_token)[token_length-1] == '\"') // check for quoted string
		{
			size_t l=token_length-2;
			if (l>0)
			{
				memcpy(*_token,(*_token)+1,l*sizeof(char));
				(*_token)[l]=0;
				Query_Unescape(*_token); 
			}
			else
				(*_token)[0]=0;// we have an empty string
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

static uint8_t quickhex(char c)
{
	int hexvalue = c;
	if (hexvalue & 0x10)
		hexvalue &= ~0x30;
	else
	{
		hexvalue &= 0xF;
		hexvalue += 9;
	}
	return hexvalue;
}

static uint8_t DecodeEscape(const char *&str)
{
	uint8_t a = quickhex(*++str);
	uint8_t b = quickhex(*++str);
	str++;
	return a * 16 + b;
}

static void DecodeEscapedUTF8(char *&output, const char *&input)
{
	bool error=false;

	while (*input == '%')
	{
		if (isxdigit(input[1]) && isxdigit(input[2]))
		{
			*output++=DecodeEscape(input);
		}
		else if (input[1] == '%')
		{
			input+=2;
			*output++='%';
		}
		else
		{
			error = true;
			break;
		}
	}

	if (error)
	{
		*output++ = *input++;
	}
}

// benski> We have the luxury of knowing that decoding will ALWAYS produce smaller strings
// so we can do it in-place
void Query_Unescape(char *str)
{
	const char *itr = str;
	while (*itr)
	{
		switch (*itr)
		{
			case '%':
				DecodeEscapedUTF8(str, itr);
				break;
			default:
				*str++ = *itr++;
				break;
		}
	}
	*str = 0;
}

