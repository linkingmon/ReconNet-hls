#include <hls_video.h>

#include "ap_fixed.h"
#include <iostream>

using namespace std;

// typedef ap_fixed<16, 4> data_type;
typedef float data_type;

#define IMAGE_SIZE 33
#define IMAGE_CHANNELS 1


void check_read(hls::stream<data_type> &in, data_type& out_val){
    while(true){
        if(in.empty() == 0){
            in >> out_val;
            break;
        }
    }
}

void show_stream(hls::stream<data_type> &in, int size){
    for(int x = 0 ; x < size ; x++){
        for(int y = 0 ; y < size ; y++){
            float temp;
            check_read(in, temp);
            cout << temp << " ";
        }
        cout << '\n';
    }
}

data_type relu(data_type a){
    #pragma HLS inline
	return a > (data_type)0 ? a : (data_type)0;
}

template<int INPUT_SIZE, int INPUT_CHANNELS, int KERNEL_SIZE, int CHANNELS, int FILTERS, int STRIDE>
void conv_layer(hls::stream<data_type> &out, hls::stream<data_type> &in,
		data_type weight[KERNEL_SIZE][KERNEL_SIZE][CHANNELS][FILTERS]/*,
		data_type bias[FILTERS]*/) {
	int i, j, k, filter;
	data_type sum, placeholder;
	int row_offset, col_offset, channel_offset;
	hls::LineBuffer<INPUT_SIZE * INPUT_CHANNELS * (KERNEL_SIZE -1) + KERNEL_SIZE * INPUT_CHANNELS, 1, data_type> conv_buff;

	for(i = 0 ; i < INPUT_SIZE * INPUT_CHANNELS * (KERNEL_SIZE -1) + KERNEL_SIZE * INPUT_CHANNELS; i++) {
        check_read(in, placeholder);
        conv_buff.shift_up(0);
        conv_buff.insert_top(placeholder, 0);
	}

	for (i = 0 ; i < (INPUT_SIZE - KERNEL_SIZE + 1); i += STRIDE)
		for (j = 0 ; j < (INPUT_SIZE - KERNEL_SIZE + 1); j += STRIDE){
			for (filter = 0 ; filter < FILTERS ; filter++) {
				sum = 0;
				for (row_offset = 0 ; row_offset < KERNEL_SIZE; row_offset++)
					for (col_offset = 0 ; col_offset < KERNEL_SIZE; col_offset++)
						for (channel_offset = 0 ; channel_offset < CHANNELS ; channel_offset++) {
							// #pragma HLS pipeline
							int t1, t2;
							static data_type val1, val2;
							t1 = row_offset * INPUT_SIZE * INPUT_CHANNELS;
							t2 = col_offset * INPUT_CHANNELS;
							val1 = conv_buff.getval(t1 + t2 + channel_offset,
									0);
							val2 = weight[row_offset][col_offset][channel_offset][filter];
							sum += val1 * val2;
                            // cout << "(" << val1 << "," << val2 << ")" << endl;
						}
                // cout << sum << endl;
				out << relu(sum/* + bias[filter]*/);
			}


			if ((j + STRIDE < (INPUT_SIZE - KERNEL_SIZE + 1))) {
				for (int p = 0 ; p < INPUT_CHANNELS ; p++)
					if (in.empty() == 0) {
						check_read(in, placeholder);
						conv_buff.shift_up(0);
						conv_buff.insert_top(placeholder, 0);
					}
			} else if ((i + STRIDE < (INPUT_SIZE - KERNEL_SIZE + 1))
					&& (j + STRIDE >= (INPUT_SIZE - KERNEL_SIZE + 1)))
				for (int p = 0 ; p < KERNEL_SIZE * INPUT_CHANNELS ; p++)
					if (in.empty() == 0) {
						check_read(in, placeholder);
						conv_buff.shift_up(0);
						conv_buff.insert_top(placeholder, 0);
					}
		}
}

template<int KERNEL_SIZE, int CHANNELS, int FILTERS>
void load_weight(data_type ary_in[KERNEL_SIZE*KERNEL_SIZE*CHANNELS*FILTERS], data_type weight[KERNEL_SIZE][KERNEL_SIZE][CHANNELS][FILTERS]){
    for(int i = 0 ; i < FILTERS ; i++){
        for(int j = 0 ; j < KERNEL_SIZE ; j++){
            for(int k = 0 ; k < KERNEL_SIZE ; k++){
                for(int l = 0 ; l < CHANNELS ; l++){
                    weight[j][k][l][i] = ary_in[i*KERNEL_SIZE*KERNEL_SIZE*CHANNELS+j*KERNEL_SIZE*CHANNELS+k*CHANNELS+l];
                }
            }
        }
    }
}

template<int PAD_SIZE>
void pad_layer(hls::stream<data_type> &out, hls::stream<data_type> &in){
    for(int x = -PAD_SIZE ; x < IMAGE_SIZE+PAD_SIZE ; x++){
        for(int y = -PAD_SIZE ; y < IMAGE_SIZE+PAD_SIZE ; y++){
            data_type temp;
            if(x < 0 || x >= IMAGE_SIZE || y < 0 || y >= IMAGE_SIZE)
                temp = (data_type)0;
            else
                check_read(in, temp);
            out << temp;
        }
    }
}

void ReconNet(hls::stream<data_type> &img_in_stream,
            data_type kernel1_ary[11*11],
            data_type kernel2_ary[1*1] ,
            data_type kernel3_ary[7*7] ,
            data_type kernel4_ary[11*11],
            data_type kernel5_ary[1*1] ,
            data_type kernel6_ary[7*7] ,
            hls::stream<data_type> &img_out_stream
         ){
	#pragma HLS INTERFACE axis port = img_out_stream
	#pragma HLS INTERFACE axis port = img_in_stream 
	// #pragma HLS INTERFACE m_axi depth=69120 port = weight
	#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL

    data_type kernel1_weight[11][11][1][1];
    data_type kernel2_weight[1][1][1][1];
    data_type kernel3_weight[7][7][1][1];
    data_type kernel4_weight[11][11][1][1];
    data_type kernel5_weight[1][1][1][1];
    data_type kernel6_weight[7][7][1][1];

    // load data to BRAM
    load_weight<11, 1, 1>(kernel1_ary, kernel1_weight);
    load_weight<1, 1, 1>(kernel2_ary, kernel2_weight);
    load_weight<7, 1, 1>(kernel3_ary, kernel3_weight);
    load_weight<11, 1, 1>(kernel4_ary, kernel4_weight);
    load_weight<1, 1, 1>(kernel5_ary, kernel5_weight);
    load_weight<7, 1, 1>(kernel6_ary, kernel6_weight);

    #pragma dataflow

	hls::stream<data_type> conv1_out;
	hls::stream<data_type> conv2_out;
	hls::stream<data_type> conv3_out;
	hls::stream<data_type> conv4_out;
	hls::stream<data_type> conv5_out;
    hls::stream<data_type> img_pad_out;
	hls::stream<data_type> conv2_pad_out;
	hls::stream<data_type> conv3_pad_out;
	hls::stream<data_type> conv5_pad_out;
    // Start image streaming
	pad_layer<5>(img_pad_out, img_in_stream);
	conv_layer<43, 1, 11, 1, 1, 1>(conv1_out, img_pad_out, kernel1_weight/*, kernel1_bias*/);
	conv_layer<33, 1, 1, 1, 1, 1>(conv2_out, conv1_out, kernel2_weight/*, kernel2_bias*/);
	pad_layer<3>(conv2_pad_out, conv2_out);
	conv_layer<39, 1, 7, 1, 1, 1>(conv3_out, conv2_pad_out, kernel3_weight/*, kernel3_bias*/);
	pad_layer<5>(conv3_pad_out, conv3_out);
	conv_layer<43, 1, 11, 1, 1, 1>(conv4_out, conv3_pad_out, kernel4_weight/*, kernel4_bias*/);
	conv_layer<33, 1, 1, 1, 1, 1>(conv5_out, conv4_out, kernel5_weight/*, kernel5_bias*/);
	pad_layer<3>(conv5_pad_out, conv5_out);
	conv_layer<39, 1, 7, 1, 1, 1>(img_out_stream, conv5_pad_out, kernel6_weight/*, kernel6_bias*/);
}