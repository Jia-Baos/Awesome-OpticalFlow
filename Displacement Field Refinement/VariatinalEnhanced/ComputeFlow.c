#include <string.h>
#include "ComputeFlow.h"
#include "image.h"
#include "opticalflow.h"


void ComputeFlow(float* myOFParams, float* flowX, float* flowY, float* im1Data, float* im2Data, float* xMatches, float* yMatches, float* sMatches)
{
    // Init:
    int width, height, stride;
    color_image_t *inIm1 = NULL, *inIm2 = NULL;
    image_t *match_x = NULL, *match_y = NULL, *match_s = NULL;

    // Custom OF parameters:
    optical_flow_params_t* ofParams = (optical_flow_params_t*)malloc(sizeof(optical_flow_params_t));
    optical_flow_params_kitti(ofParams);

    width               = myOFParams[0];
    height              = myOFParams[1];
    ofParams->alpha     = myOFParams[2];
    ofParams->beta      = myOFParams[3];
    ofParams->gamma     = myOFParams[4];
    ofParams->delta     = myOFParams[5];
    ofParams->sigma     = myOFParams[6];
    ofParams->bk        = myOFParams[7];
    ofParams->eta       = myOFParams[8];
    ofParams->min_size  = myOFParams[9];

	// Input images:
    stride  = ((width + 3) / 4) * 4;
    inIm1   = color_image_new(width, height);
    inIm2   = color_image_new(width, height);

	memcpy(inIm1->c1, im1Data, stride*height*sizeof(float));
	memcpy(inIm1->c2, im1Data, stride*height*sizeof(float));
	memcpy(inIm1->c3, im1Data, stride*height*sizeof(float));

    memcpy(inIm2->c1, im2Data, stride*height*sizeof(float));
	memcpy(inIm2->c2, im2Data, stride*height*sizeof(float));
	memcpy(inIm2->c3, im2Data, stride*height*sizeof(float));

    // Flow fields:
    image_t *uX = image_new(width, height);
	image_t	*uY = image_new(width, height);

	// DeepMatching containers:
	match_x = image_new(width, height);
    match_y = image_new(width, height);
	match_s = image_new(width, height);

    memcpy(match_x->data, xMatches, stride*height*sizeof(float));
    memcpy(match_y->data, yMatches, stride*height*sizeof(float));
    memcpy(match_s->data, sMatches, stride*height*sizeof(float));

	// Compute LDOF
	optical_flow(uX, uY, inIm1, inIm2, ofParams, match_x, match_y, match_s);

	//Arrange flow fields for return:
    memcpy(flowX, uX->data, stride*height*sizeof(float));
    memcpy(flowY, uY->data, stride*height*sizeof(float));

	// Cleanup:
	free(ofParams);
	color_image_delete(inIm1); color_image_delete(inIm2);
	image_delete(uX); image_delete(uY);
	image_delete(match_x); image_delete(match_y); image_delete(match_s);

}
