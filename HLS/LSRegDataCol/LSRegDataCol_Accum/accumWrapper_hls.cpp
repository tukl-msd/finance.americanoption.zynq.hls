/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier A. Varela (varela@eit.uni-kl.de)
//
// Last review: 29/06/2015
//
// Using: Xilinx Vivado HLS 2014.3
//
///////////////////////////////////////////////////  

#include "accumWrapper_hls.hpp"
#include "accumulator_hls.hpp"

void lsrdcaccumregio (	const uint32_t stepsMC,
						const uint32_t pathsMC,
						volatile uint32_t *peekStep_x0,
						volatile uint32_t *peekStep_x1,
						volatile uint32_t *peekStep_x2,
						volatile uint32_t *peekStep_x3,
						volatile uint32_t *peekStep_x4,
						volatile uint32_t *peekStep_y,
						volatile uint32_t *peekStep_yx,
						volatile uint32_t *peekStep_yx2,
						volatile uint32_t *peekPath_x0,
						volatile uint32_t *peekPath_x1,
						volatile uint32_t *peekPath_x2,
						volatile uint32_t *peekPath_x3,
						volatile uint32_t *peekPath_x4,
						volatile uint32_t *peekPath_y,
						volatile uint32_t *peekPath_yx,
						volatile uint32_t *peekPath_yx2,
						hls::stream<float> &stream_x0,
						hls::stream<float> &stream_x1,
						hls::stream<float> &stream_x2,
						hls::stream<float> &stream_x3,
						hls::stream<float> &stream_x4,
						hls::stream<float> &stream_y,
						hls::stream<float> &stream_yx,
						hls::stream<float> &stream_yx2,
						hls::stream<float> &accum_x0,
						hls::stream<float> &accum_x1,
						hls::stream<float> &accum_x2,
						hls::stream<float> &accum_x3,
						hls::stream<float> &accum_x4,
						hls::stream<float> &accum_y,
						hls::stream<float> &accum_yx,
						hls::stream<float> &accum_yx2 )
{
#pragma HLS UNROLL

#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL

#pragma HLS INTERFACE s_axilite port=peekStep_x0  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_x1  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_x2  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_x3  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_x4  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_y   bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_yx  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep_yx2 bundle=CONTROL

#pragma HLS INTERFACE s_axilite port=peekPath_x0  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_x1  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_x2  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_x3  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_x4  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_y   bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_yx  bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath_yx2 bundle=CONTROL

#pragma HLS INTERFACE axis register port=stream_x0
#pragma HLS INTERFACE axis register port=stream_x1
#pragma HLS INTERFACE axis register port=stream_x2
#pragma HLS INTERFACE axis register port=stream_x3
#pragma HLS INTERFACE axis register port=stream_x4
#pragma HLS INTERFACE axis register port=stream_y
#pragma HLS INTERFACE axis register port=stream_yx
#pragma HLS INTERFACE axis register port=stream_yx2

#pragma HLS INTERFACE axis register port=accum_x0
#pragma HLS INTERFACE axis register port=accum_x1
#pragma HLS INTERFACE axis register port=accum_x2
#pragma HLS INTERFACE axis register port=accum_x3
#pragma HLS INTERFACE axis register port=accum_x4
#pragma HLS INTERFACE axis register port=accum_y
#pragma HLS INTERFACE axis register port=accum_yx
#pragma HLS INTERFACE axis register port=accum_yx2


	accumregio(	stepsMC, pathsMC, peekStep_x0,  peekPath_x0, stream_x0,  accum_x0  );
	accumregio(	stepsMC, pathsMC, peekStep_x1,  peekPath_x1, stream_x1,  accum_x1  );
	accumregio(	stepsMC, pathsMC, peekStep_x2,  peekPath_x2, stream_x2,  accum_x2  );
	accumregio(	stepsMC, pathsMC, peekStep_x3,  peekPath_x3, stream_x3,  accum_x3  );
	accumregio(	stepsMC, pathsMC, peekStep_x4,  peekPath_x4, stream_x4,  accum_x4  );
	accumregio(	stepsMC, pathsMC, peekStep_y,   peekPath_y,  stream_y,   accum_y   );
	accumregio(	stepsMC, pathsMC, peekStep_yx,  peekPath_yx, stream_yx,  accum_yx  );
	accumregio(	stepsMC, pathsMC, peekStep_yx2, peekPath_yx2,stream_yx2, accum_yx2 );


	return;
}

