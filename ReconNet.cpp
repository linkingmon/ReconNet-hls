#include <hls_video.h>

#include "ap_fixed.h"
#include "weights.h"
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

void show_stream(hls::stream<data_type> &in, int size, int channel){
	for(int x = 0 ; x < size ; x++){
		for(int y = 0 ; y < size ; y++){
			for(int n_channel = 0 ; n_channel < channel ; n_channel++){
				float temp;
				check_read(in, temp);
				cout << temp << " ";
			}
			cout << '\n';
		}
	}
	assert(0);
}

data_type relu(data_type a){
    #pragma HLS inline
	return a > (data_type)0 ? a : (data_type)0;
}

template<int INPUT_SIZE, int INPUT_CHANNELS, int KERNEL_SIZE, int FILTERS, int STRIDE>
void conv_layer(hls::stream<data_type> &out, hls::stream<data_type> &in,
		data_type weight[KERNEL_SIZE][KERNEL_SIZE][INPUT_CHANNELS][FILTERS]/*,
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
						for (channel_offset = 0 ; channel_offset < INPUT_CHANNELS ; channel_offset++) {
							// #pragma HLS pipeline
							int t1, t2;
							static data_type val1, val2;
							t1 = row_offset * INPUT_SIZE * INPUT_CHANNELS;
							t2 = col_offset * INPUT_CHANNELS;
							val1 = conv_buff.getval(t1 + t2 + channel_offset,
									0);
							val2 = weight[row_offset][col_offset][channel_offset][filter];
							sum += val1 * val2;
                            // cout << "(" << row_offset << "," << col_offset << "," << channel_offset << "," << filter << ") >> (" << val1 << "," << val2 << ")" << endl;
						}
                // cout << sum << endl;
				out << relu(sum/* + bias[filter]*/);
                // assert(0);
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
	cout << "CONV DONE" << endl;
}

template<int PAD_SIZE, int CHANNELS>
void pad_layer(hls::stream<data_type> &out, hls::stream<data_type> &in){
	for(int x = -PAD_SIZE ; x < IMAGE_SIZE+PAD_SIZE ; x++){
		for(int y = -PAD_SIZE ; y < IMAGE_SIZE+PAD_SIZE ; y++){
			for(int n_channel = 0 ; n_channel < CHANNELS ; n_channel++){
				data_type temp;
				if(x < 0 || x >= IMAGE_SIZE || y < 0 || y >= IMAGE_SIZE)
					temp = (data_type)0;
				else
					check_read(in, temp);
				out << temp;
			}
		}
	}
	cout << "PAD DONE" << endl;
}

void ReconNet(hls::stream<data_type> &img_in_stream,
            hls::stream<data_type> &img_out_stream
         ){
	#pragma HLS INTERFACE axis port = img_out_stream
	#pragma HLS INTERFACE axis port = img_in_stream 
	// #pragma HLS INTERFACE m_axi depth=69120 port = weight
	#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL

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
	// template<int INPUT_SIZE, int INPUT_CHANNELS, int KERNEL_SIZE, int FILTERS, int STRIDE>
	pad_layer<5,1>(img_pad_out, img_in_stream);
	conv_layer<43, 1, 11, 64, 1>(conv1_out, img_pad_out, kernel1_weight/*, kernel1_bias*/);
	conv_layer<33, 64, 1, 32, 1>(conv2_out, conv1_out, kernel2_weight/*, kernel2_bias*/);
	pad_layer<3,32>(conv2_pad_out, conv2_out);
	conv_layer<39, 32, 7, 1, 1>(conv3_out, conv2_pad_out, kernel3_weight/*, kernel3_bias*/);
	pad_layer<5,1>(conv3_pad_out, conv3_out);
	conv_layer<43, 1, 11, 64, 1>(conv4_out, conv3_pad_out, kernel4_weight/*, kernel4_bias*/);
	conv_layer<33, 64, 1, 32, 1>(conv5_out, conv4_out, kernel5_weight/*, kernel5_bias*/);
	pad_layer<3,32>(conv5_pad_out, conv5_out);
	conv_layer<39, 32, 7, 1, 1>(img_out_stream, conv5_pad_out, kernel6_weight/*, kernel6_bias*/);
}