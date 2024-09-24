#ifndef _IPOD_INFO_H_
#define _IPOD_INFO_H_

#define RGB_565 0
#define RGB_555 1
#define RGB_555_REC 2
typedef enum {
  THUMB_INVALID = -1,
  THUMB_COVER_SMALL,
	THUMB_COVER_MEDIUM1,
	THUMB_COVER_MEDIUM2,
	THUMB_COVER_MEDIUM3,
	THUMB_COVER_MEDIUM4,
  THUMB_COVER_LARGE,
  THUMB_PHOTO_SMALL,
  THUMB_PHOTO_LARGE,
  THUMB_PHOTO_FULL_SCREEN,
  THUMB_PHOTO_TV_SCREEN,
} ThumbType;

typedef enum {
	IPOD_COLOR_WHITE,
	IPOD_COLOR_BLACK,
	IPOD_COLOR_SILVER,
	IPOD_COLOR_BLUE,
	IPOD_COLOR_PINK,
	IPOD_COLOR_GREEN,
	IPOD_COLOR_ORANGE,
	IPOD_COLOR_GOLD,
	IPOD_COLOR_RED,
	IPOD_COLOR_U2,
} iPodColor;

typedef enum {
  IPOD_MODEL_INVALID=0,
  IPOD_MODEL_COLOR=1,
  IPOD_MODEL_REGULAR=2,
  IPOD_MODEL_MINI=3,
  IPOD_MODEL_SHUFFLE=4,
  IPOD_MODEL_VIDEO=5,
  IPOD_MODEL_NANO=6,
	IPOD_MODEL_CLASSIC=7,
	IPOD_MODEL_FATNANO=8,
	IPOD_MODEL_TOUCH=9,
} iPodModel;

typedef struct {
	ThumbType type;
	int width;
	int height;
	int correlation_id;
	int format;
	int row_align;
	int image_align;
} ArtworkFormat;

struct iPodModelInfo
{
	// model_number is abbreviated: if the first character is not numeric, it is ommited. e.g. "MA350 -> A350", "M9829 -> 9829"
	const wchar_t *model_number;
	iPodModel model;
	iPodColor color;
	int image16;
	int image160;
};

class iPodInfo
{
public:
	iPodInfo(const iPodModelInfo *model);
	~iPodInfo();
	void SetFWID(const uint8_t *new_fwid);
	int family_id;
	wchar_t *model_number;
	iPodModel model;
	iPodColor color;
	int image16;
	int image160;
	// Store the supported artwork formats if we
	// can dynamically read it from the extended sysinfo xml
	ArtworkFormat* supportedArtworkFormats;
	size_t numberOfSupportedFormats;
	unsigned char *fwid;
	unsigned int shadow_db_version;
};

struct _iPodSerialToModel {
    const wchar_t *serial;
    const wchar_t *model_number;
};
typedef struct _iPodSerialToModel iPodSerialToModel;

iPodInfo *GetiPodInfo(wchar_t drive);

const ArtworkFormat* GetArtworkFormats(const iPodInfo* info);

#endif //_IPOD_INFO_H_
