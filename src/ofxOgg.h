#ifndef OFXOGGH
#define OFXOGGH

extern "C" {
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}

#include "ofxOggServer.h"
#include <set>
#include "ofMain.h"


class ofxOgg {
public:
	ofxOgg();
	~ofxOgg();
	void addFrame();
	void onSetup(ofEventArgs& ev);
	void enableRecording();
	void disableRecording();
	bool isRecordingEnabled();
private:
	void setup(int w, int h, int bytesPerPixel = 3);
	string getOutputFilePath();
	string output_file_path;

	int number_of_ogg_files;
	int bpp;
	int width;
	int height;
	FILE* out_file;
	th_info info;	
	th_enc_ctx* context;
	
	ogg_stream_state oggss;
	
	// used to write a frame
	th_ycbcr_buffer ycbcr;
	unsigned long yuv_w;
	unsigned long yuv_h;

	unsigned char *yuv_y; // @todo free this
	unsigned char *yuv_u; // @todo free this
	unsigned char *yuv_v; // @todo free this

	unsigned int x;
	unsigned int y;
	struct SwsContext* convert_context; // @todo free this
	vpx_image_t* in_image; // @todo free
	unsigned char* out_planes[3];
	int out_strides[3];
	int line_size ;
	ofImage grab_image;
	bool is_enabled;
};

#endif