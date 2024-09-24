class Shaper;

class Decoder
{
private:
	void process_rg();
	void setup_mc();
	float* bptr;
	float** pcmbuf;
	Shaper * shaper;
	UINT data,pos;
	float scale;
	int cur_link;
	int cur_preamp;
	int clipmin,clipmax;
public:
	VorbisFile * file;

	UINT nch,sr,kbps,bps,src_nch;

	Decoder()
	{
		memset(this,0,sizeof(*this));
	}

	~Decoder();
	
	int Seek(double p);
	int Read(UINT bytes,void * buf);
	void Flush();
	void Init(VorbisFile * f, UINT _bits=0, UINT _nch=0, bool _useFloat=false, bool allowRG=true);
	void wa2_setinfo(UINT cur_bitrate);

	UINT DataAvailable();
	int DoFrame();
	bool need_reopen;
	int play_init();
	bool play_inited;
	bool dither;
	bool useFloat;
};