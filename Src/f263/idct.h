#pragma once
	class IDCT
	{
	public:
		void init();
		void idct(short *block);
	private:
		void idctcol(short *blk);
		void idctrow(short *blk);

		static bool initted;
		static short iclip[1024]; /* clipping table */
		static short *iclp;
	};