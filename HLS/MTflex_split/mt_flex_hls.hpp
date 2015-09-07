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

#ifndef __MT_FLEX_HLS_HPP__
#define __MT_FLEX_HLS_HPP__

#include <stdint.h>
#include <hls_stream.h>

#define MT_M 397
#define MT_N 624

void mtflexrego ( 	const uint32_t amount,
					volatile uint32_t *peekAmountFwd,
					volatile uint32_t *peekAmountRev,
					unsigned int seeds[MT_N],
					hls::stream<uint32_t> &uniform  );

#endif
