#ifndef _YAIL_H_
#define _YAIL_H_

// yet another image library. Because everything else SUCKS. Fact.

typedef unsigned short RGB565;

class Image {
public:
	Image(const ARGB32 * data, int w, int h);
	Image(const RGB565 * data, int w, int h, int format, int alignRowBytes, int alignImageBytes);
	~Image();
	void exportToRGB565(RGB565* data, int format, int alignRowBytes, int alignImageBytes) const;
	void exportToARGB32(ARGB32* data) const;
	ARGB32 * getData() {return data;}
	int getWidth() const {return width;}
	int getHeight() const {return height;}
	int get16BitSize(int alignRowBytes, int alignImageBytes) { return get16BitSize(width,height,alignRowBytes, alignImageBytes); }
	static int get16BitSize(int width, int height, int alignRowBytes, int alignImageBytes);
protected:
	ARGB32 *data;
	int width,height;
};

#endif //_YAIL_H_