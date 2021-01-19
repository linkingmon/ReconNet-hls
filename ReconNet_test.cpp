#include <stdio.h>
#include <stdlib.h>
#include <hls_video.h>
#include <iostream>
#include "hls_opencv.h"

#include "ap_fixed.h"
#include "hls_half.h"
typedef half data_type;
using namespace std;

// typedef ap_fixed<16, 4> data_type;
// typedef float data_type;


#define IMAGE_SIZE 33
#define IMAGE_CHANNELS 1

template<int SIZE>
void load_img(hls::stream<data_type>& stream_in, string filename);

// void ReconNet(hls::stream<data_type> &img_in_stream,
//             hls::stream<data_type> &img_out_stream
//          );
		 
void ReconNet(hls::stream<ap_axiu<32,1,1,1> >& AXI_video_stream_in,
            hls::stream<ap_axiu<32,1,1,1> >& AXI_video_stream_out
         );

int main()
{
	hls::stream<data_type> img_in_stream;
	data_type kernel1_ary[11*11];
	data_type kernel2_ary[1*1];
	data_type kernel3_ary[7*7];
	data_type kernel4_ary[11*11];
	data_type kernel5_ary[1*1];
	data_type kernel6_ary[7*7];
	hls::stream<data_type> img_out_stream;

	data_type img_out[IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS];
	float img_out_gt[IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS];

    char INPUT_IMAGE[30] = "barbara.tif";
    char OUTPUT_IMAGE[30] = "barbara_recon.png";
    char OUTPUT_IMAGE_GOLDEN[30] = "barbara_recon.png";

    // load_img<IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS>(img_in_stream, "img_in.txt");

	hls::stream<ap_axiu<32,1,1,1> > src_axi, dst_axi;
	IplImage* src = cvLoadImage(INPUT_IMAGE);
    cout << "Load image success " << endl;
    IplImage* dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
    IplImage2AXIvideo(src, src_axi);

    // pass through kernel
    cout << "Start Kernel " << endl;
    ReconNet(src_axi, dst_axi);
    cout << "Done Kernel " << endl;

	AXIvideo2IplImage(dst_axi, dst);
	cvSaveImage(OUTPUT_IMAGE, dst);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);

    // char tempbuf[2000];
    // sprintf(tempbuf, "diff --brief -w %s %s", OUTPUT_IMAGE, OUTPUT_IMAGE_GOLDEN);

    // int ret = system(tempbuf);
    // if (ret != 0) {
    //     printf("????????????? Test Failed!\n");
    //     ret = 1;
    // } else {
    //     printf("------------- Test Passed!\n");
    // }

	return 0;

    // // stream out the output image
	// for(int i = 0 ; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS ; i++)
	// 	img_out_stream >> img_out[i];
    // cout << "Loading Output stream " << endl;

    // // read GT of teh output image
	// FILE* img_out_head = fopen("img_out.txt","r");
	// if(img_out_head == NULL) return -1;
	// for(int i = 0; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS; i++)
	// 	fscanf(img_out_head, "%f", &img_out_gt[i]);
    // cout << "Read Output GT " << endl;

    // compare the output results
	printf("\n\n\n\n");
	printf("Checking Output image ...\n");

	int correct_values = 0;
	int total_values = 0;
    double eps = 0.001;

	for(int i = 0; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS; i++){ 
        // need to modify to PSNR
		total_values++;
        if(i < 20 || i > IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS-20) cout << img_out[i] << "    " << img_out_gt[i] << endl;
		if((double)img_out[i] - img_out_gt[i] > eps || img_out_gt[i] - (double)img_out[i] > eps)
		{
			if(correct_values + 1 == total_values)
				printf("Missmatch in FC3 check\n");
		}
        else
            correct_values++;
	}

	printf("DONE: %d out of %d are correct\n\n", correct_values, total_values);


	return 0;

}

template<int SIZE>
void load_img(hls::stream<data_type>& stream_in, string filename){
	FILE* img_in_head = fopen(filename.c_str(),"r");
	for(int i = 0; i < SIZE; i++){
        data_type temp;
		fscanf(img_in_head, "%f", &temp);
        stream_in << temp;
    }
    cout << "Done loading " << filename << endl;
}