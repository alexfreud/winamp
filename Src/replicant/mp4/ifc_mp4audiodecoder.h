#pragma once
#include "foundation/dispatch.h"
#include "ifc_mp4file.h"
#include "audio/ifc_audioout.h"

class ifc_mp4audiodecoder : public Wasabi2::Dispatchable
{
	protected:
	ifc_mp4audiodecoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_mp4audiodecoder() {}
public:
	int FillAudioParameters(ifc_audioout::Parameters *parameters) { return MP4AudioDecoder_FillAudioParameters(parameters); }
	int Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position) { return MP4AudioDecoder_Decode(output_buffer, output_buffer_bytes, start_position, end_position); }
	int Seek(ifc_mp4file::SampleID sample_number) { return MP4AudioDecoder_Seek(sample_number); }
	int SeekSeconds(double *seconds) { return MP4AudioDecoder_SeekSeconds(seconds); }
	int ConnectFile(ifc_mp4file *new_file) { return MP4AudioDecoder_ConnectFile(new_file); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
private:
	/* sizeof_parameters will already be filled out for you */
	virtual int WASABICALL MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)=0;
	virtual int WASABICALL MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position)=0;
	virtual int WASABICALL MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number) = 0;
	/* fill in with the actual seconds you'll resume playback at */
	virtual int WASABICALL MP4AudioDecoder_SeekSeconds(double *seconds) = 0;
	/* this is an unfortunate wart in the API.  In order to support editing metadata on an actively playing file, we have to re-open the file which will generate a new ifc_mp4file object.  
	do _not_ reset the decoder or change the sample number.  You should assume the file is identical from a playback point-of-view, just release your old object and retain/assign the new one */
	virtual int WASABICALL MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file)=0;
};
