#include <stdio.h>
#include <stdlib.h>
#include <hls_video.h>
#include <iostream>
#include "hls_opencv.h"
#include <math.h>
#include "ap_fixed.h"
#include "hls_half.h"
typedef half data_type;
using namespace std;

// typedef ap_fixed<16, 4> data_type;
// typedef float data_type;


#define IMAGE_SIZE 33
#define IMAGE_CHANNELS 1
#define PATCH_ROWS 8
#define PATCH_COLS 8
#define INPUT_SIZE 256

template<int SIZE>
void load_img(hls::stream<data_type>& stream_in, string filename);

void ReconNet(hls::stream<data_type> &img_in_stream,
            hls::stream<data_type> &img_out_stream
         );
		 
// void ReconNet(hls::stream<ap_axiu<32,1,1,1> >& AXI_video_stream_in,
//             hls::stream<ap_axiu<32,1,1,1> >& AXI_video_stream_out
//          );

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

	data_type img_out[INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS]; // 256*256*1
	float img_out_gt[INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS]; // 256*256*1

    // char INPUT_IMAGE[30] = "barbara.tif";
    // char OUTPUT_IMAGE[30] = "barbara_recon.png";
    // char OUTPUT_IMAGE_GOLDEN[30] = "barbara_recon.png";
	cout << "Loading Input stream " << endl;
    load_img<INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS>(img_in_stream, "barbara_in.txt");

	// hls::stream<ap_axiu<32,1,1,1> > src_axi, dst_axi;
	// IplImage* src = cvLoadImage(INPUT_IMAGE);
    // cout << "Load image success " << endl;
    // IplImage* dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
    // IplImage2AXIvideo(src, src_axi);

    // pass through kernel
    cout << "Start Kernel " << endl;
    // ReconNet(img_in_stream, dst_axi);
    ReconNet(img_in_stream, img_out_stream);
    cout << "Done Kernel " << endl;
    // stream out the output image
	// data_type trash;
	// for(int i = 0 ; i < IMAGE_SIZE*PATCH_ROWS*IMAGE_SIZE*PATCH_COLS*IMAGE_CHANNELS ; i++)
	for(int i = 0 ; i < INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS ; i++)
		// if ( i%(IMAGE_SIZE*PATCH_ROWS) < INPUT_SIZE) img_out_stream >> img_out[i];
		// else img_out_stream >> trash;
		img_out_stream >> img_out[i];
    cout << "Loading Output stream " << endl;
	// AXIvideo2IplImage(dst_axi, dst);
	// cvSaveImage(OUTPUT_IMAGE, dst);
	// cvReleaseImage(&src);
	// cvReleaseImage(&dst);
	FILE* img_out_write = fopen("barbara_out_csim.txt","w+");
	for(int i = 0; i < INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS; i++){
		// printf("%f", img_out[i]);
		cout << img_out[i] <<endl;
		fprintf(img_out_write, "%f\n", float(img_out[i]));
	}

    cout << "Write Output csim result" << endl;
    // char tempbuf[2000];
    // sprintf(tempbuf, "diff --brief -w %s %s", OUTPUT_IMAGE, OUTPUT_IMAGE_GOLDEN);

    // int ret = system(tempbuf);
    // if (ret != 0) {
    //     printf("????????????? Test Failed!\n");
    //     ret = 1;
    // } else {
    //     printf("------------- Test Passed!\n");
    // }

	// return 0;

    // // stream out the output image
	// for(int i = 0 ; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS ; i++)
	// 	img_out_stream >> img_out[i];
    // cout << "Loading Output stream " << endl;

    // // read GT of teh output image
	FILE* img_out_head = fopen("barbara_out_0_1.txt","r");
	if(img_out_head == NULL) return -1;
	for(int i = 0; i < INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS; i++)
		fscanf(img_out_head, "%f", &img_out_gt[i]);
    cout << "Read Output GT " << endl;

    // compare the output results
	printf("\n\n\n\n");
	printf("Checking Output image ...\n");

	// int correct_values = 0;
	// int total_values = 0;
    // double eps = 0.001;
	int threshold = 24;
	float diff = 0;
	for(int i = 0; i < INPUT_SIZE*INPUT_SIZE*IMAGE_CHANNELS; i++){ 
        // need to modify to PSNR
		diff += pow(((float)(img_out[i]*255) - (img_out_gt[i]*255)), 2);
		// total_values++;
        // if(i < 20 || i > IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS-20) cout << img_out[i] << "    " << img_out_gt[i] << endl;
		// if((double)img_out[i] - img_out_gt[i] > eps || img_out_gt[i] - (double)img_out[i] > eps)
		// {
		// 	if(correct_values + 1 == total_values)
		// 		printf("Missmatch in FC3 check\n");
		// }
        // else
        //     correct_values++;
	}
	float mse = diff/IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS;
	float psnr = 10 * log10f(255.0*255.0/mse);
	// printf("DONE: %d out of %d are correct\n\n", correct_values, total_values);
	if (psnr > 24) printf("Success!!! PSNR is %f", psnr);
	else printf("Failed QQ PSNR is %f", psnr);

	return 0;

}

template<int SIZE>
void load_img(hls::stream<data_type>& stream_in, string filename){
	FILE* img_in_head = fopen(filename.c_str(),"r");
	for(int i = 0; i < SIZE; i++){
        data_type temp;
		float test;
		fscanf(img_in_head, "%f", &test);
		// cout << test << endl;
		temp = test;
		// cout << temp << endl;
        stream_in << temp;
    }
    cout << "Done loading " << filename << endl;
}