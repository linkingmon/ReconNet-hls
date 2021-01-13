#include <stdio.h>
#include <stdlib.h>
#include <hls_video.h>
#include <iostream>


#include "ap_fixed.h"
using namespace std;

// typedef ap_fixed<16, 4> data_type;
typedef float data_type;


#define IMAGE_SIZE 33
#define IMAGE_CHANNELS 1

template<int SIZE>
void load_weights(data_type ary_in[SIZE], string filename);
template<int SIZE>
void load_img(hls::stream<data_type>& stream_in, string filename);

void ReconNet(hls::stream<data_type> &img_in_stream,
            data_type kernel1_ary[11*11],
            data_type kernel2_ary[1*1] ,
            data_type kernel3_ary[7*7] ,
            data_type kernel4_ary[11*11],
            data_type kernel5_ary[1*1] ,
            data_type kernel6_ary[7*7] ,
            hls::stream<data_type> &img_out_stream
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

    // read input images and kernel weights
    load_img<33*33>(img_in_stream, "img_in.txt");
    load_weights<11*11>(kernel1_ary, "kernel_11.txt");
    load_weights<1*1>(kernel2_ary, "kernel_1.txt");
    load_weights<7*7>(kernel3_ary, "kernel_7.txt");
    load_weights<11*11>(kernel4_ary, "kernel_11.txt");
    load_weights<1*1>(kernel5_ary, "kernel_1.txt");
    load_weights<7*7>(kernel6_ary, "kernel_7.txt");

    // pass through kernel
    cout << "Start Kernel " << endl;
    ReconNet(img_in_stream,
             kernel1_ary,
             kernel2_ary,
             kernel3_ary,
             kernel4_ary,
             kernel5_ary,
             kernel6_ary,
             img_out_stream);
    cout << "Done Kernel " << endl;

    // stream out the output image
	for(int i = 0 ; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS ; i++)
		img_out_stream >> img_out[i];
    cout << "Loading Output stream " << endl;

    // read GT of teh output image
	FILE* img_out_head = fopen("img_out.txt","r");
	if(img_out_head == NULL) return -1;
	for(int i = 0; i < IMAGE_SIZE*IMAGE_SIZE*IMAGE_CHANNELS; i++)
		fscanf(img_out_head, "%f", &img_out_gt[i]);
    cout << "Read Output GT " << endl;

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
void load_weights(data_type ary_in[SIZE], string filename){
	FILE* img_in_head = fopen(filename.c_str(),"r");
	for(int i = 0; i < SIZE; i++){
        data_type temp;
		fscanf(img_in_head, "%f", &temp);
        ary_in[i] = temp;
    }
    cout << "Done loading " << filename << endl;
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