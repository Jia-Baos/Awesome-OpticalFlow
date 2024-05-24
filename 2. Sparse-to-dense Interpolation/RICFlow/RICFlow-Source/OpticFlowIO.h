// flowIO.h

#ifndef _OpticFlowIO_H
#define _OpticFlowIO_H

#include <math.h>
#include <memory.h>
#include <string.h>

#include <stdint.h>
#include <opencv2/opencv.hpp> // for KITTI

// read and write our simple .flo flow file format

// ".flo" file format used for optical flow evaluation
//
// Stores 2-band float image for horizontal (u) and vertical (v) flow components.
// Floats are stored in little-endian order.
// A flow value is considered "unknown" if either |u| or |v| is greater than 1e9.
//
//  bytes  contents
//
//  0-3     tag: "PIEH" in ASCII, which in little endian happens to be the float 202021.25
//          (just a sanity check that floats are represented correctly)
//  4-7     width as an integer
//  8-11    height as an integer
//  12-end  data (width*height*2*4 bytes total)
//          the float values for u and v, interleaved, in row order, i.e.,
//          u[row0,col0], v[row0,col0], u[row0,col1], v[row0,col1], ...
//

// value to use to represent unknown flow
#define UNKNOWN_FLOW 1e10

typedef struct
{
	double aee; // Average Endpoint Error
	double aae; // Average Angular Error
}FlowErr;

class OpticFlowIO
{
public:
	// return whether flow vector is unknown
	template <class T>
	static bool unknown_flow(T u, T v);
	template <class T>
	static bool unknown_flow(T* f);

	// read a flow file into 2-band image
	template <class T>
	static int ReadFlowFile(T* U, T* V, int* w, int* h, const char* filename);

	// write a 2-band image into flow file
	template <class T>
	static int WriteFlowFile(T* U, T* V, int w, int h, const char* filename);

	// read a KITTI flow file into 2-band image
	template <class T>
	static int ReadKittiFlowFile(T* U, T* V, int* w, int* h, const char* filename);

	// write a 2-band image into KITTI flow file
	template <class T>
	static int WriteKittiFlowFile(T* U, T* V, int w, int h, const char* filename);

	// render the motion to a 4-band BGRA color image
	template <class T>
	static double MotionToColor(unsigned char* fillPix, T* U, T* V, int w, int h, float range = -1);

	template <class T>
	static float ShowFlow(const char* winname, T* U, T* V, int w, int h, float range = -1, int waittime = 1);
	template <class T>
	static void SaveFlowAsImage(const char* imgName, T* U, T* V, int w, int h, float range = -1);

	template <class T>
	static float ErrorImage(unsigned char* fillPix, T* u1, T* v1, T* u2, T* v2, int w, int h);
	template <class T>
	static float ErrorImage(unsigned char* fillPix, T* u1, T* v1, char* gtName, int w, int h);
	template <class T>
	static float ShowErrorImage(const char* winname, T* U, T* V, char* gtName, int w, int h, int waittime = 1);
	template <class T>
	static float SaveErrorImage(const char* imgName, T* U, T* V, char* gtName, int w, int h);

	template <class T1, class T2>
	static FlowErr CalcFlowError(T1* u1, T1* v1, T2* u2, T2* v2, int w, int h);

private:
	// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25  // check for this when READING the file
#define TAG_STRING "PIEH"    // use this when WRITING the file

#define M_PI       3.14159265358979323846

// the "official" threshold - if the absolute value of either
// flow component is greater, it's considered unknown
#define UNKNOWN_FLOW_THRESH 1e9

#define NUM_BANDS 2

// Color encoding of flow vectors
// adapted from the color circle idea described at
//   http://members.shaw.ca/quadibloc/other/colint.htm
//
// Daniel Scharstein, 4/2007
// added tick marks and out-of-range coding 6/05/07

#define MAXWHEELCOLS 60
	template <class T>
	static void setcols(T* colorwheel, int r, int g, int b, int k);
	template <class T>
	static int makecolorwheel(T* colorwheel);
	template <class T>
	static void computeColor(double fx, double fy, unsigned char* pix, T* colorwheel, int ncols);
};

template <class T>
bool OpticFlowIO::unknown_flow(T u, T v)
{
	return (abs(u) > UNKNOWN_FLOW_THRESH)
		|| (abs(v) > UNKNOWN_FLOW_THRESH)
		|| u != u || v != v;	// isnan()
}

template <class T>
int OpticFlowIO::ReadFlowFile(T* U, T* V, int* w, int* h, const char* filename)
{
	if (filename == NULL) {
		printf("ReadFlowFile: empty filename\n");
		return -1;
	}

	const char* dot = strrchr(filename, '.');
	if (strcmp(dot, ".flo") != 0) {
		printf("ReadFlowFile (%s): extension .flo expected\n", filename);
		return -1;
	}

	FILE* stream = fopen(filename, "rb");
	if (stream == 0) {
		printf("ReadFlowFile: could not open %s\n", filename);
		return -1;
	}

	int width, height;
	float tag;

	if ((int)fread(&tag, sizeof(float), 1, stream) != 1
		|| (int)fread(&width, sizeof(int), 1, stream) != 1
		|| (int)fread(&height, sizeof(int), 1, stream) != 1)
	{
		printf("ReadFlowFile: problem reading file %s\n", filename);
		return -1;
	}

	if (tag != TAG_FLOAT) // simple test for correct endian-ness
	{
		printf("ReadFlowFile(%s): wrong tag (possibly due to big-endian machine?)\n", filename);
		return -1;
	}

	// another sanity check to see that integers were read correctly (99999 should do the trick...)
	if (width < 1 || width > 99999) {
		printf("ReadFlowFile(%s): illegal width %d\n", filename, width);
		return -1;
	}

	if (height < 1 || height > 99999) {
		printf("ReadFlowFile(%s): illegal height %d\n", filename, height);
		return -1;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float tmp[NUM_BANDS];
			if ((int)fread(tmp, sizeof(float), NUM_BANDS, stream) != NUM_BANDS) {
				printf("ReadFlowFile(%s): file is too short\n", filename);
				return -1;
			}
			U[i * width + j] = tmp[0];
			V[i * width + j] = tmp[1];
		}
	}

	if (fgetc(stream) != EOF) {
		printf("ReadFlowFile(%s): file is too long\n", filename);
		return -1;
	}

	*w = width;
	*h = height;

	fclose(stream);
	return 0;
}

template <class T>
int OpticFlowIO::WriteFlowFile(T* U, T* V, int w, int h, const char* filename)
{
	if (filename == NULL) {
		printf("WriteFlowFile: empty filename\n");
		return -1;
	}

	const char* dot = strrchr(filename, '.');
	if (dot == NULL) {
		printf("WriteFlowFile: extension required in filename '%s'\n", filename);
		return -1;
	}

	if (strcmp(dot, ".flo") != 0) {
		printf("WriteFlowFile: filename '%s' should have extension '.flo'\n", filename);
		return -1;
	}

	int width = w, height = h;

	FILE* stream = fopen(filename, "wb");
	if (stream == 0) {
		printf("WriteFlowFile: could not open %s\n", filename);
		return -1;
	}

	// write the header
	fprintf(stream, TAG_STRING);
	if ((int)fwrite(&width, sizeof(int), 1, stream) != 1
		|| (int)fwrite(&height, sizeof(int), 1, stream) != 1)
	{
		printf("WriteFlowFile(%s): problem writing header\n", filename);
		return -1;
	}

	// write the rows
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float tmp[NUM_BANDS];
			tmp[0] = U[i * width + j];
			tmp[1] = V[i * width + j];
			if ((int)fwrite(tmp, sizeof(float), NUM_BANDS, stream) != NUM_BANDS) {
				printf("WriteFlowFile(%s): problem writing data\n", filename);
				return -1;
			}
		}
	}

	fclose(stream);
	return 0;
}
#endif //_OpticFlowIO_H