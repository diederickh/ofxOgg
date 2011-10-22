#ifndef OFXOGGH
#define OFXOGGH

//#include <stdio.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>
//
//#include <stdio.h>
//#include <errno.h>
//#include <ctype.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/time.h>
//#include <halfduplex.h> /* For name of the named-pipe */

extern "C" {
//	#include <ogg/ogg.h>
//	#include <theora/theoraenc.h>
//	#include <libswscale/swscale.h>
//	#include "vpx_image.h"
	
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
//	#include "external/vorbis/codec.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}

class ofxOgg {
public:
	ofxOgg();
	~ofxOgg();
	void setup(int w, int h, int bytesPerPixel);
	void addFrame(unsigned char* pixels);
private:
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
};

#endif