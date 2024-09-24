#include "regexp.h"
// TODO: make a little more multi-byte safe



// regexp match functions

// A match means the entire string TEXT is used up in matching.
// In the pattern string:
//      `*' matches any sequence of characters (zero or more)
//      `?' matches any character
//      [SET] matches any character in the specified set,
//      [!SET] or [^SET] matches any character not in the specified set.

// A set is composed of characters or ranges; a range looks like
// character hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the
// minimal set of characters allowed in the [..] pattern construct.
// Other characters are allowed (ie. 8 bit characters) if your system
// will support them.

// To suppress the special syntactic significance of any of `[]*?!^-\',
// and match the character exactly, precede it with a `\'.

enum {
    MATCH_VALID = 1,    /* valid match */
    MATCH_END,        /* premature end of pattern string */
    MATCH_ABORT,      /* premature end of text string */
    MATCH_RANGE,      /* match failure on [..] construct */
    MATCH_LITERAL,    /* match failure on literal match */
    MATCH_PATTERN,    /* bad pattern */
};

enum {
    PATTERN_VALID = 0,    /* valid pattern */
    PATTERN_ESC = -1,     /* literal escape at end of pattern */
    PATTERN_RANGE = -2,   /* malformed range in [..] construct */
    PATTERN_CLOSE = -3,   /* no end bracket in [..] construct */
    PATTERN_EMPTY = -4,   /* [..] contstruct is empty */
};

int Matche(const regchar_t *p, const regchar_t *t);

// TODO: make this multi-byte aware
int matche_after_star(const regchar_t *p, const regchar_t *t)
{
	register int match = 0;
	register regchar_t nextp;
	/* pass over existing ? and * in pattern */
	while ( *p == '?' || *p == '*' )
	{
		/* take one char for each ? and + */
		if (*p == '?')
		{
			/* if end of text then no match */
			if (!*t++) return MATCH_ABORT;
		}
		/* move to next char in pattern */
		p++;
	}
	/* if end of pattern we have matched regardless of text left */
	if (!*p) return MATCH_VALID;
	/* get the next character to match which must be a literal or '[' */
	nextp = *p;
	if (nextp == '\\')
	{
		nextp = p[1];
		/* if end of text then we have a bad pattern */
		if (!nextp) return MATCH_PATTERN;
	}
	/* Continue until we run out of text or definite result seen */
	do
	{
		/* a precondition for matching is that the next character
		   in the pattern match the next character in the text or that
		   the next pattern char is the beginning of a range.  Increment
		   text pointer as we go here */
		if (nextp == *t || nextp == '[') match = Matche(p, t);
		/* if the end of text is reached then no match */
		if (!*t++) match = MATCH_ABORT;
	}
	while ( match != MATCH_VALID && match != MATCH_ABORT && match != MATCH_PATTERN);
	/* return result */
	return match;
}


int Matche(const regchar_t *p, const regchar_t *t)
{
	regchar_t range_start, range_end;  /* start and end in range */

	bool invert;             /* is this [..] or [!..] */
	bool member_match;       /* have I matched the [..] construct? */
	bool loop;               /* should I terminate? */

	for ( ; *p; p++, t++)
	{
		/* if this is the end of the text then this is the end of the match */
		if (!*t)
		{
			return (*p == '*' && *++p == '\0') ? MATCH_VALID : MATCH_ABORT;
		}
		/* determine and react to pattern type */
		switch (*p)
		{
		case '?':  /* single any character match */
			break;
		case '*':  /* multiple any character match */
			return matche_after_star (p, t);

			/* [..] construct, single member/exclusion character match */
		case '[':
			{
				/* move to beginning of range */
				p++;
				/* check if this is a member match or exclusion match */
				invert = false;
				if (*p == '!' || *p == '^')
				{
					invert = true;
					p++;
				}
				/* if closing bracket here or at range start then we have a malformed pattern */
				if (*p == ']')
					return MATCH_PATTERN;

				member_match = false;
				loop = true;
				while (loop)
				{
					/* if end of construct then loop is done */
					if (*p == ']')
					{
						loop = false;
						continue;
					}
					/* matching a '!', '^', '-', '\' or a ']' */
					if (*p == '\\')
						range_start = range_end = *++p;
					else
						range_start = range_end = *p;
					/* if end of pattern then bad pattern (Missing ']') */
					if (!*p)
						return MATCH_PATTERN;
					/* check for range bar */
					if (*++p == '-')
					{
						/* get the range end */
						range_end = *++p;
						/* if end of pattern or construct then bad pattern */
						if (range_end == '\0' || range_end == ']') return MATCH_PATTERN;
						/* special character range end */
						if (range_end == '\\')
						{
							range_end = *++p;
							/* if end of text then we have a bad pattern */
							if (!range_end) return MATCH_PATTERN;
						}
						/* move just beyond this range */
						p++;
					}
					/* if the text character is in range then match found.
					   make sure the range letters have the proper
					   relationship to one another before comparison */
					if (range_start < range_end)
					{
						if (*t >= range_start && *t <= range_end)
						{
							member_match = true;
							loop = false;
						}
					}
					else
					{
						if (*t >= range_end && *t <= range_start)
						{
							member_match = true;
							loop = false;
						}
					}
				}
				/* if there was a match in an exclusion set then no match */
				/* if there was no match in a member set then no match */
				if ((invert && member_match) || !(invert || member_match))
					return MATCH_RANGE;
				/* if this is not an exclusion then skip the rest of the [...] construct that already matched. */
				if (member_match)
				{
					while (p && *p != ']')
					{
						/* bad pattern (Missing ']') */
						if (!*p)
							return MATCH_PATTERN;
						/* skip exact match */
						if (*p == '\\')
						{
							p++;
							/* if end of text then we have a bad pattern */
							if (!*p)
								return MATCH_PATTERN;
						}
						/* move to next pattern char */
						p++;
					}
				}
				break;
			}
		case '\\':   /* next character is quoted and must match exactly */
			/* move pattern pointer to quoted char and fall through */
			p++;
			/* if end of text then we have a bad pattern */
			if (!*p)
				return MATCH_PATTERN;
			/* must match this character exactly */
		default:
			if (*p != *t)
				return MATCH_LITERAL;
		}
	}
	/* if end of text not reached then the pattern fails */
	if (*t)
		return MATCH_END;
	else return MATCH_VALID;
}

bool Match(const regchar_t *match, const regchar_t *string)
{
	if (!match)
		return true;
	int error_type;
	
	error_type = Matche(match, string);
	return (error_type == MATCH_VALID);
}
