#pragma once
class IDCTRef
	{
	public:
		void init();
		void idct(short *block);
	private:

		static bool initted;
		static double c[8][8]; /* cosine transform matrix for 8x1 IDCT */
	}; 