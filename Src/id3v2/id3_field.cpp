//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//  
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//  
//  Mon Nov 23 18:34:01 1998

// improved/optimized/whatever 10/30/00 JF

#include <windows.h>
#include <string.h>
#include "id3_field.h"

static ID3_FieldDef ID3FD_URL[] =
{
	{ID3FN_URL, ID3FTY_ASCIISTRING,  -1, 2, 0, ID3VC_HIGHER, NULL, ID3FN_NOFIELD},
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_UserURL[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION,    ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_ADJUSTENC,            ID3FN_NOFIELD    },
	{ID3FN_URL,        ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_Text[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_TEXT,        ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTENC,                  ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_UserText[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION,    ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_ADJUSTENC,            ID3FN_NOFIELD    },
	{ID3FN_TEXT,        ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTENC,                  ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_GeneralText[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,    ID3FTY_INTEGER,      1,  2, 0, ID3VC_HIGHER, NULL,                         ID3FN_NOFIELD    },
	{ID3FN_LANGUAGE,    ID3FTY_ASCIISTRING, 3,  2, 0, ID3VC_HIGHER, NULL,                         ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION, ID3FTY_ASCIISTRING, -1, 2, 0, ID3VC_HIGHER, ID3FF_NULL | ID3FF_ADJUSTENC, ID3FN_NOFIELD    },
	{ID3FN_TEXT,        ID3FTY_ASCIISTRING, -1, 2, 0, ID3VC_HIGHER, ID3FF_ADJUSTENC,              ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_Picture[] =
{
	//  FIELD          FIELD                   FIXED  RENDER IF  OR
	//  NAME          TYPE                     LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,         1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_IMAGEFORMAT,    ID3FTY_ASCIISTRING,   3,    2,  0,    ID3VC_LOWER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_MIMETYPE,      ID3FTY_ASCIISTRING,   -1,    3,  0,    ID3VC_HIGHER,  ID3FF_NULL,                      ID3FN_NOFIELD    },
	{ID3FN_PICTURETYPE,    ID3FTY_INTEGER,       1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION,    ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_ADJUSTENC,            ID3FN_NOFIELD    },
	{ID3FN_DATA,        ID3FTY_BINARY,          -1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_GEO[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_MIMETYPE,      ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL,                      ID3FN_NOFIELD    },
	{ID3FN_FILENAME,      ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_ADJUSTENC,            ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION,    ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_ADJUSTENC,            ID3FN_NOFIELD    },
	{ID3FN_DATA,        ID3FTY_BINARY,    -1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_UFI[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_OWNER,      ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL,  ID3FN_NOFIELD    },
	{ID3FN_DATA,       ID3FTY_BINARY,  -1,    2,  0,    ID3VC_HIGHER,  NULL,        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};

static  ID3_FieldDef  ID3FD_PRIVATE[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_OWNER,      ID3FTY_ASCIISTRING,  -1,    3,  0,    ID3VC_HIGHER,  ID3FF_NULL,  ID3FN_NOFIELD    },
	{ID3FN_DATA,       ID3FTY_BINARY,  -1,    3,  0,    ID3VC_HIGHER,  NULL,        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_PlayCounter[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_COUNTER,      ID3FTY_INTEGER,     4,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_Popularimeter[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_EMAIL,      ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL,                      ID3FN_NOFIELD    },
	{ID3FN_RATING,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_COUNTER,      ID3FTY_INTEGER,     4,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_Registration[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_OWNER,      ID3FTY_ASCIISTRING,  -1,    3,  0,    ID3VC_HIGHER,  ID3FF_NULL,                      ID3FN_NOFIELD    },
	{ID3FN_SYMBOL,      ID3FTY_INTEGER,     1,    3,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_DATA,        ID3FTY_BINARY,    -1,    3,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_InvolvedPeople[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_TEXT,        ID3FTY_ASCIISTRING,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_NULL | ID3FF_NULLDIVIDE | ID3FF_ADJUSTENC,  ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};


static  ID3_FieldDef  ID3FD_Volume[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_VOLUMEADJ,    ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_NUMBITS,      ID3FTY_INTEGER,     1,    2,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_VOLCHGRIGHT,    ID3FTY_BITFIELD,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTEDBY,                  ID3FN_NUMBITS    },
	{ID3FN_VOLCHGLEFT,    ID3FTY_BITFIELD,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTEDBY,                  ID3FN_NUMBITS    },
	{ID3FN_PEAKVOLRIGHT,    ID3FTY_BITFIELD,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTEDBY,                  ID3FN_NUMBITS    },
	{ID3FN_PEAKVOLLEFT,    ID3FTY_BITFIELD,  -1,    2,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTEDBY,                  ID3FN_NUMBITS    },
	{ID3FN_NOFIELD    }
};

static  ID3_FieldDef  ID3FD_MCDI[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_CD_TOC,    ID3FTY_BINARY,     -1,    3,  0,    ID3VC_HIGHER,  NULL, ID3FN_NOFIELD},
	{ID3FN_NOFIELD    }
};

static  ID3_FieldDef  ID3FD_Text_v2_4[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,      ID3FTY_INTEGER,     1,    4,  0,    ID3VC_HIGHER,  NULL,                        ID3FN_NOFIELD    },
	{ID3FN_TEXT,        ID3FTY_ASCIISTRING,  -1,    4,  0,    ID3VC_HIGHER,  ID3FF_ADJUSTENC,                  ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};

static  ID3_FieldDef  ID3FD_Timestamp1[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TIMESTAMP,   ID3FTY_INTEGER,   1,  2, 0,     ID3VC_HIGHER, NULL,                      ID3FN_NOFIELD    },
	{ID3FN_DATA,       ID3FTY_BINARY,    -1,  3,  0,    ID3VC_HIGHER, NULL,                      ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};

static  ID3_FieldDef  ID3FD_Timestamp2[] =
{
	//  FIELD          FIELD        FIXED  RENDER IF  OR
	//  NAME          TYPE        LEN    VER  REV    WHAT?      FLAGS                        LINKED FIELD
	{ID3FN_TEXTENC,    ID3FTY_INTEGER,      1,  2, 0, ID3VC_HIGHER, NULL,                         ID3FN_NOFIELD    },
	{ID3FN_LANGUAGE,    ID3FTY_ASCIISTRING, 3,  2, 0, ID3VC_HIGHER, NULL,                         ID3FN_NOFIELD    },
	{ID3FN_TIMESTAMP,   ID3FTY_INTEGER,      1,  2, 0, ID3VC_HIGHER, NULL,                        ID3FN_NOFIELD    },
	{ID3FN_CONTENTTYPE, ID3FTY_INTEGER,      1,  2, 0, ID3VC_HIGHER, NULL,                        ID3FN_NOFIELD    },
	{ID3FN_DESCRIPTION, ID3FTY_ASCIISTRING, -1, 2, 0, ID3VC_HIGHER, ID3FF_NULL | ID3FF_ADJUSTENC, ID3FN_NOFIELD    },
	{ID3FN_NOFIELD    }
};

static  ID3_FrameDef  ID3_FrameDefs[] =
{
	//FRAME ID                SHORTID LONGID   PRI TAGDISCARD FILEDISCARD HANDLER        FIELDDEFS
	{ID3FID_ENCODEDBY,         "TEN",  "TENC",    false,    true,        NULL,        ID3FD_Text        },
	{ID3FID_ORIGALBUM,         "TOT",  "TOAL",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PUBLISHER,         "TPB",  "TPUB",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ENCODERSETTINGS,   "TSS",  "TSSE",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ORIGFILENAME,      "TOF",  "TOFN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_LANGUAGE,          "TLA",  "TLAN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PARTINSET,         "TPA",  "TPOS",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_DATE,              "TDA",  "TDAT",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_TIME,              "TIM",  "TIME",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_RECORDINGDATES,    "TRD",  "TRDA",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_MEDIATYPE,         "TMT",  "TMED",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_FILETYPE,          "TFT",  "TFLT",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_NETRADIOSTATION,   "TRN",  "TRSN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_NETRADIOOWNER,     "TRO",  "TRSO",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_LYRICIST,          "TXT",  "TEXT",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ORIGARTIST,        "TOA",  "TOPE",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ORIGLYRICIST,      "TOL",  "TOLY",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_CONTENTGROUP,      "TT1",  "TIT1",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_TITLE,             "TT2",  "TIT2",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_SUBTITLE,          "TT3",  "TIT3",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_LEADARTIST,        "TP1",  "TPE1",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_BAND,              "TP2",  "TPE2",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_CONDUCTOR,         "TP3",  "TPE3",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_MIXARTIST,         "TP4",  "TPE4",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ALBUM,             "TAL",  "TALB",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_YEAR,              "TYE",  "TYER",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_COMPOSER,          "TCM",  "TCOM",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_COPYRIGHT,         "TCR",  "TCOP",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PRODUCEDNOTICE,    "   ",  "TPRO",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_CONTENTTYPE,       "TCO",  "TCON",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_TRACKNUM,          "TRK",  "TRCK",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_USERTEXT,          "TXX",  "TXXX",    false,    false,       NULL,        ID3FD_UserText    },
	{ID3FID_COMMENT,           "COM",  "COMM",    false,    false,       NULL,        ID3FD_GeneralText },
	{ID3FID_TERMSOFUSE,        "   ",  "USER",    false,    false,       NULL,        ID3FD_GeneralText },
	{ID3FID_UNSYNCEDLYRICS,    "ULT",  "USLT",    false,    false,       NULL,        ID3FD_GeneralText },
	{ID3FID_SYNCEDLYRICS,      "SLT",  "SYLT",    false,    false,       NULL,        ID3FD_Timestamp2  },
	{ID3FID_SYNCEDTEMPOCODE,   "STC",  "SYTC",    false,    false,       NULL,        ID3FD_Timestamp1  },
	// URL Frames
	{ID3FID_WWWAUDIOFILE,      "WAF",  "WOAF",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWARTIST,         "WAR",  "WOAR",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWAUDIOSOURCE,    "WAS",  "WOAS",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWCOMMERCIALINFO, "WCM",  "WCOM",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWCOPYRIGHT,      "WCP",  "WCOP",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWPUBLISHER,      "WPB",  "WPUB",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWPAYMENT,        "WPY",  "WPAY",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWRADIOPAGE,      "WRA",  "WORS",    false,    false,       NULL,        ID3FD_URL        },
	{ID3FID_WWWUSER,           "WXX",  "WXXX",    false,    false,       NULL,        ID3FD_UserURL    },
	// misc frames
	{ID3FID_INVOLVEDPEOPLE,    "IPL",  "IPLS",    false,    false,       NULL,        ID3FD_InvolvedPeople },
	{ID3FID_PICTURE,           "PIC",  "APIC",    false,    false,       NULL,        ID3FD_Picture        },
	{ID3FID_GENERALOBJECT,     "GEO",  "GEOB",    false,    false,       NULL,        ID3FD_GEO            },
	{ID3FID_UNIQUEFILEID,      "UFI",  "UFID",    false,    false,       NULL,        ID3FD_UFI            },
	{ID3FID_PRIVATE,           "   ",  "PRIV",    false,    false,       NULL,        ID3FD_PRIVATE        },
	{ID3FID_PLAYCOUNTER,       "CNT",  "PCNT",    false,    false,       NULL,        ID3FD_PlayCounter    },
	{ID3FID_POPULARIMETER,     "POP",  "POPM",    false,    false,       NULL,        ID3FD_Popularimeter  },
	{ID3FID_CRYPTOREG,         "   ",  "ENCR",    false,    false,       NULL,        ID3FD_Registration   },
	{ID3FID_GROUPINGREG,       "   ",  "GRID",    false,    false,       NULL,        ID3FD_Registration   },
	{ID3FID_SIGNATURE,         "   ",  "SIGN",    false,    false,       NULL,        ID3FD_Registration   },
	{ID3FID_MCDI,              "MCI",  "MCDI",    false,    false,       NULL,        ID3FD_MCDI},
	{ID3FID_BPM,               "TBP",  "TBPM",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_KEY,               "TKE",  "TKEY",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_MOOD,              "   ",  "TMOO",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_ISRC,              "TRC",  "TSRC",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_RECORDINGTIME,     "   ",  "TDRC",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_COMPILATION,       "TCP",  "TCMP",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_ALBUMSORT,         "TSA",  "TSOA",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_ALBUMARTISTSORT,   "TS2",  "TSO2",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_PERFORMERSORT,     "TSP",  "TSOP",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_COMPOSERSORT,      "TSC",  "TSOC",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_TITLESORT,         "TST",  "TSOT",    false,    false,       NULL,        ID3FD_Text},
	{ID3FID_REPLAYGAIN,        "   ",  "RGAD",    false,    false,       NULL,        ID3FD_UserText    },
	{ID3FID_VOLUMEADJ,         "RVA",  "RVAD",    false,    false,       NULL,        ID3FD_Volume      },
	{ID3FID_INVOLVEDPEOPLE2,   "   ",  "TIPL",    false,    false,       NULL,        ID3FD_InvolvedPeople },
	{ID3FID_CREDITS,           "   ",  "TMCL",    false,    false,       NULL,        ID3FD_InvolvedPeople },
	{ID3FID_ENCODINGTIME,      "   ",  "TDEN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_FILEOWNER,         "   ",  "TOWN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_LENGTH,            "TLE",  "TLEN",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ORIGYEAR,          "TOR",  "TORY",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_ORIGRELEASETIME,   "   ",  "TDOR",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_RELEASETIME,       "   ",  "TDRL",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_SETSUBTITLE,       "   ",  "TSST",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_TAGGINGTIME,       "   ",  "TDTG",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PLAYLISTDELAY,     "TDY",  "TDLY",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PODCAST,           "   ",  "PCST",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PODCASTCATEGORY,   "   ",  "TCAT",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PODCASTDESC,       "   ",  "TDES",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PODCASTID,         "   ",  "TGID",    false,    false,       NULL,        ID3FD_Text        },
	{ID3FID_PODCASTURL,        "   ",  "WFED",    false,    false,       NULL,        ID3FD_URL         },
	{ID3FID_NOFRAME    }
};

ID3_Field::ID3_Field(void)
{
	name	= ID3FN_NOFIELD;
	type	= ID3FTY_INTEGER;
	data	= 0;
	size	= 0;
	flags	= 0;
	//SetVersion(3,0);
	version = 3;
	revision = 0;
	fixedLength = -1;
  	ioVersion = 3;
	ioRevision = 0;
	control = ID3VC_HIGHER;

	Clear();
}

ID3_Field::~ID3_Field(void)
{
	Clear();
}

void ID3_Field::Clear(void)
{
	if	(data && type != ID3FTY_INTEGER) free(data);

	type		= ID3FTY_INTEGER;
	data		= 0;
	size		= sizeof (luint);
	hasChanged	= true;

	return;
}

void ID3_Field::SetVersion (uchar ver, uchar rev)
{
	if (version != ver || revision != rev) hasChanged = true;

	version	= ver;
	revision = rev;

	return;
}


bool ID3_Field::HasChanged (void)
{
	return hasChanged;
}


luint ID3_Field::Size(void)
{
	return BinSize (false);
}


luint ID3_Field::BinSize(bool withExtras)
{
	luint bytes	= 0;

	if (control == ID3VC_HIGHER)
	{
		if (version < ioVersion || revision < ioRevision)
			return 0;
	}
	else
	{
		if (version > ioVersion || revision > ioRevision)
			return 0;
	}

	bytes = size;

	if (withExtras)
	{
		if (!data && size)
		{
			if (flags & ID3FF_NULL)
				bytes = 2;
			else
				bytes = 0;
		}

		// if we are a Unicode string, add 2 bytes for the BOM (but
		// only if there is a string to render - regardless of NULL)
		if	(type == ID3FTY_UNICODESTRING && data && size) bytes += 2;

		// if we are an ASCII string, divide by sizeof (wchar_t)
		// because internally we store the string as Unicode, so
		// the ASCII version will only be half as long				
		if (type == ID3FTY_UTF8STRING)
		{
			if (data && size)
				bytes = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t *)data, (int)(size/sizeof(wchar_t)), 0, 0, 0, 0);
			else
				bytes /= sizeof(wchar_t);
		}

		if	(type == ID3FTY_ASCIISTRING)
		{
			// TODO: this statement isn't correct (especially for double byte)	
			// we could use WideCharToMultiByte to determine an exact byte count
			bytes /= sizeof (wchar_t);
		}
	}
	else
	{
		// because it seems that the application called us via ID3_Field::Size()
		// we are going to return the number of characters, not bytes, so if
		// the string is Unicode, we will half the 'bytes' variable because
		// Unicode strings have twice as many bytes as they do characters
		if (type == ID3FTY_UNICODESTRING)
			bytes /= sizeof(wchar_t);
	}

	// check to see if we are within the legal limit for this field
	// -1 means arbitrary length field
	if	(fixedLength != -1)
		bytes = fixedLength;

	return bytes;
}


luint ID3_Field::Parse(uchar *buffer, luint posn, luint buffSize)
{
	if (control == ID3VC_HIGHER)
	{
		if (version < ioVersion || revision < ioRevision) 
			return 0;
	}
	else
	{
		if (version > ioVersion || revision > ioRevision) 
			return 0;
	}

	switch(type)
	{
		case ID3FTY_INTEGER:
			return ParseInteger(buffer, posn, buffSize);
		case ID3FTY_BINARY:
			return ParseBinary(buffer, posn, buffSize);
		case ID3FTY_ASCIISTRING:
			return ParseASCIIString(buffer, posn, buffSize);
		case ID3FTY_UNICODESTRING:
			return ParseUnicodeString(buffer, posn, buffSize);
		case ID3FTY_UTF8STRING:
			return ParseUTF8String(buffer, posn, buffSize);
		default:
			//ID3_THROW (ID3E_UnknownFieldType);
			break;
	}

	return 0;
}

ID3_FrameDef *ID3_FindFrameDef(ID3_FrameID id)
{
	luint cur = 0;
	while (1)
	{
		if (ID3_FrameDefs[cur].id == id)
			return &ID3_FrameDefs[cur];
		if (ID3_FrameDefs[cur].id == ID3FID_NOFRAME)
			return NULL;
		cur++;
	}
}

ID3_FrameID ID3_FindFrameID(const char *id)
{
	luint cur = 0;
	while (1)
	{
		if (ID3_FrameDefs[cur].id == ID3FID_NOFRAME)
			return ID3FID_NOFRAME;
		if (((strncmp(ID3_FrameDefs[cur].shortTextID, id, 3) == 0) && strlen (id) == 3) 
			|| ((strncmp(ID3_FrameDefs[cur].longTextID, id, 4) == 0) && strlen (id) == 4))
			return ID3_FrameDefs[cur].id;
		cur++;
	}
}

luint ID3_Field::Render(uchar *buffer)
{
	if	(control == ID3VC_HIGHER)
	{
		if (version < ioVersion || revision < ioRevision) 
			return 0;
	}
	else
	{
		if (version > ioVersion || revision > ioRevision) 
			return 0;
	}

	switch(type)
	{
	case ID3FTY_INTEGER:
		return RenderInteger(buffer);
	case ID3FTY_BINARY:
		return RenderBinary(buffer);
	case ID3FTY_ASCIISTRING:
		return RenderLatinString(buffer); 
	case ID3FTY_UNICODESTRING:
		return RenderUnicodeString(buffer);
	case ID3FTY_UTF8STRING:
		return RenderUTF8String(buffer);
	default:
		//			ID3_THROW (ID3E_UnknownFieldType);
		break;
	}
	return 0;
}


