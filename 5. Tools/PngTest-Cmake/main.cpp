#include <iostream>
#include <string>
#include "./pnglib/include/png.h"

#include "./io.h"
#include "./image.h"
#include "./array_types.h"

int main(int argc, char *argv[])
{

	std::string image_path = "D:\\CmakeProject\\OpticalFlow\\PngTest-Cmake\\MPI-Sintel\\frame_0029.png";
	color_image_t *image = color_image_load(image_path.c_str());
	if (image != nullptr)
	{
		std::cout << "Load image successed.." << std::endl;
	}

	std::cout << "Width: " << image->width << std::endl;
	std::cout << "Height: " << image->height << std::endl;
	std::cout << "Stride: " << image->stride << std::endl;

	return 0;
}
