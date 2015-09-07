/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier A. Varela (varela@eit.uni-kl.de)
//
// Last review: 30/06/2015
//
// Zynq ZC702
//
///////////////////////////////////////////////////  

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "mt19937ar.h"    
#include "ls_golden.h"   

#define ADDRESS_MT0a        0X43C00000
#define ADDRESS_MT0b        0X43C01000
#define ADDRESS_MT1a        0X43C10000
#define ADDRESS_MT1b        0X43C11000
#define ADDRESS_BM0    		0X43C20000
#define ADDRESS_BM1    		0X43C30000
#define ADDRESS_CORR    	0X43C40000
#define ADDRESS_BS0 		0X43C50000
#define ADDRESS_BS1   		0X43CD0000
#define ADDRESS_MAX   		0X43C60000
#define ADDRESS_LSDATAGEN	0x43C70000
#define ADDRESS_LSDATAACC	0x43C80000
#define ADDRESS_CHOL 		0x43C90000
#define ADDRESS_CHOLB0		0x43C91000
#define ADDRESS_CHOLB1 		0x43C92000
#define ADDRESS_CHOLB2 		0x43C93000
#define ADDRESS_CHOLPEEK	0x43C94000
#define ADDRESS_LSUPDATE1	0x43CA0000
#define ADDRESS_LSUPDATE2	0x43CB0000
#define ADDRESS_CFLOWACC	0x43CC0000

/*
// *******************************************************
// Option's Parameters:
const float T 			= 1.0f;		// maturity (1year = 1.0)
const float S0_1 		= 100.00f;	// spot price (current price of the stock)
const float S0_2 		= 90.00f;	// spot price (current price of the stock)
const float K 			= 105.00f;	// option's strike price 
const float sigma1 		= 0.20f;	// volatility of stock price 
const float sigma2 		= 0.35f;	// volatility of stock price 
const float r   		= 0.07f;	// risk-free interest rate (1-year interest rate)
const float q 			= 0.10f;		// dividend yield
const float rho 		= 0.50f;	// correlation between boths stocks in the stock market
const uint32_t callPut 	= 1; 		// Option type: 0=call (buy), any other value=put (sell)
// *******************************************************
// Simulation's Parameters:
const uint32_t STEPSMC 	= 365;
const uint32_t PATHSMC 	= 10000;

const uint32_t seedValue1 = 0;		// Mersenne-Twister seed for stock 1
const uint32_t seedValue2 = 12345;	// Mersenne-Twister seed for stock 2

const float freqFPGA 	= 100.0f;	// FPGA clock frequency in MHz
// *******************************************************
*/

// *******************************************************
// Option's Parameters:
const float T 			= 1.0f;		// maturity (1year = 1.0)
const float S0_1 		= 100.00f;	// spot price (current price of the stock)
const float S0_2 		= 100.00f;	// spot price (current price of the stock)
const float K 			= 100.00f;	// option's strike price 
const float sigma1 		= 0.30f;	// volatility of stock price 
const float sigma2 		= 0.30f;	// volatility of stock price 
const float r   		= 0.06f;	// risk-free interest rate (1-year interest rate)
const float q 			= 0.0f;		// dividend yield
const float rho 		= 0.10f;	// correlation between boths stocks in the stock market
const uint32_t callPut 	= 0; 		// Option type: 0=call (buy), any other value=put (sell)
// *******************************************************
// Simulation's Parameters:
const uint32_t STEPSMC 	= 365;
const uint32_t PATHSMC 	= 10000;

const uint32_t seedValue1 = 0;		// Mersenne-Twister seed for stock 1
const uint32_t seedValue2 = 12345;	// Mersenne-Twister seed for stock 2

const float freqFPGA 	= 100.0f;	// FPGA clock frequency in MHz
// *******************************************************


/*
// *******************************************************
// Option's Parameters:
const float T 			= 1.0f;		// maturity (1year = 1.0)
const float S0_1 		= 100.00f;	// spot price (current price of the stock)
const float S0_2 		= 100.00f;	// spot price (current price of the stock)
const float K 			= 100.00f;	// option's strike price 
const float sigma1 		= 0.30f;	// volatility of stock price 
const float sigma2 		= 0.30f;	// volatility of stock price 
const float r   		= 0.06f;	// risk-free interest rate (1-year interest rate)
const float q 			= 0.0f;		// dividend yield
const float rho 		= 0.10f;	// correlation between boths stocks in the stock market
const uint32_t callPut 	= 5; 		// Option type: 0=call (buy), any other value=put (sell)
// *******************************************************
// Simulation's Parameters:
const uint32_t STEPSMC 	= 50000;
const uint32_t PATHSMC 	= 32000; //10000;

const uint32_t seedValue1 = 0;		// Mersenne-Twister seed for stock 1
const uint32_t seedValue2 = 12345;	// Mersenne-Twister seed for stock 2

const float freqFPGA 	= 100.0f;	// FPGA clock frequency in MHz
// *******************************************************
*/

// ---------------------------------------------------------------
typedef struct
{
	int fd;
	unsigned page_offset;
	void *ptr;
} IO_DEV;
// ---------------------------------------------------------------
IO_DEV open_dev(int base_addr) {
	int fd;
	unsigned page_addr, page_offset;
	void *ptr;
	unsigned page_size = sysconf(_SC_PAGESIZE);

	//printf("page_size = %d\n",page_size);
	
	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		perror("Could not open /dev/mem");
		exit(-1);
	}

	/* mmap the device into memory */
	page_addr = (base_addr & (~(page_size-1)));
	page_offset = base_addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);

	IO_DEV dev = {fd, page_offset, ptr};
	return dev;
}
// ---------------------------------------------------------------
void *get_dev_ptr(IO_DEV dev, int addr_offset) {
	return (void*)((char*)dev.ptr + dev.page_offset + addr_offset);
}
// ---------------------------------------------------------------
void close_dev(IO_DEV dev) {
	unsigned page_size = sysconf(_SC_PAGESIZE);
	munmap(dev.ptr, page_size);
	close(dev.fd);
}
// ---------------------------------------------------------------

void CheckOperationFPGA (void)
{	
	printf("\n\n");
	printf("------------------------------------------------\n");
	printf("     CHECKING FPGA OPERATION  (Debug mode)      \n");
	printf("------------------------------------------------\n");
	IO_DEV dev_mt0a			= open_dev(ADDRESS_MT0a);
	IO_DEV dev_mt0b			= open_dev(ADDRESS_MT0b);
	IO_DEV dev_mt1a			= open_dev(ADDRESS_MT1a);
	IO_DEV dev_mt1b			= open_dev(ADDRESS_MT1b);
	IO_DEV dev_bm0			= open_dev(ADDRESS_BM0);
	IO_DEV dev_bm1			= open_dev(ADDRESS_BM1);
	IO_DEV dev_corr		 	= open_dev(ADDRESS_CORR);
	IO_DEV dev_bs0		 	= open_dev(ADDRESS_BS0);
	IO_DEV dev_bs1			= open_dev(ADDRESS_BS1);
	IO_DEV dev_max			= open_dev(ADDRESS_MAX);
	IO_DEV dev_lsdatagen	= open_dev(ADDRESS_LSDATAGEN);
	IO_DEV dev_lsdataacc 	= open_dev(ADDRESS_LSDATAACC);
	IO_DEV dev_chol			= open_dev(ADDRESS_CHOL);
	IO_DEV dev_cholb0		= open_dev(ADDRESS_CHOLB0);
	IO_DEV dev_cholb1		= open_dev(ADDRESS_CHOLB1);
	IO_DEV dev_cholb2		= open_dev(ADDRESS_CHOLB2);
	IO_DEV dev_cholpeek		= open_dev(ADDRESS_CHOLPEEK);
	IO_DEV dev_lsupdate1	= open_dev(ADDRESS_LSUPDATE1);
	IO_DEV dev_lsupdate2	= open_dev(ADDRESS_LSUPDATE2);
	IO_DEV dev_cflowacc		= open_dev(ADDRESS_CFLOWACC);
	

	volatile unsigned &status_mt0 = *((unsigned*)get_dev_ptr(dev_mt0a, 0));
	printf("status_mt0: %08x\n", status_mt0);
	
	volatile unsigned &status_mt1 = *((unsigned*)get_dev_ptr(dev_mt1a, 0));
	printf("status_mt1: %08x\n", status_mt1);
	
	volatile unsigned &status_bm0 = *((unsigned*)get_dev_ptr(dev_bm0, 0));
	printf("status_bm0: %08x\n", status_bm0);
	
	volatile unsigned &status_bm1 = *((unsigned*)get_dev_ptr(dev_bm1, 0));
	printf("status_bm1: %08x\n", status_bm1);
	
	volatile unsigned &status_corr = *((unsigned*)get_dev_ptr(dev_corr, 0));
	printf("status_corr: %08x\n", status_corr);
	
	volatile unsigned &status_bs0 = *((unsigned*)get_dev_ptr(dev_bs0, 0));
	printf("status_bs0: %08x\n", status_bs0);
	
	volatile unsigned &status_bs1 = *((unsigned*)get_dev_ptr(dev_bs1, 0));
	printf("status_bs1: %08x\n", status_bs1);
		
	volatile unsigned &status_max = *((unsigned*)get_dev_ptr(dev_max, 0));
	printf("status_max: %08x\n", status_max);
	
	volatile unsigned &status_lsdatagen = *((unsigned*)get_dev_ptr(dev_lsdatagen, 0));
	printf("status_lsdatagen: %08x\n", status_lsdatagen);
	
	volatile unsigned &status_lsdataacc = *((unsigned*)get_dev_ptr(dev_lsdataacc, 0));
	printf("status_lsdataacc: %08x\n", status_lsdataacc);
	
	volatile unsigned &status_chol = *((unsigned*)get_dev_ptr(dev_chol, 0));
	printf("status_chol: %08x\n", status_chol);
	
	volatile unsigned &status_lsupdate1 = *((unsigned*)get_dev_ptr(dev_lsupdate1, 0));
	printf("status_lsupdate1: %08x\n", status_lsupdate1);
	
	volatile unsigned &status_lsupdate2 = *((unsigned*)get_dev_ptr(dev_lsupdate2, 0));
	printf("status_lsupdate2: %08x\n", status_lsupdate2);
	
	volatile unsigned &status_cflowacc = *((unsigned*)get_dev_ptr(dev_cflowacc, 0));
	printf("status_cflowacc: %08x\n", status_cflowacc);
	
	// -----------------------------------------------------------------
	// MT
	printf("\n");
	printf("setting MT ...\n");
	uint32_t seeds0[624];
	uint32_t seeds1[624];
	
	// generate seeds 0
	init_genrand(seedValue1);
	generate_numbers();
	get_mt_state(seeds0);
	
	// generate seeds 1
	init_genrand(seedValue2);
	generate_numbers();
	get_mt_state(seeds1);
		
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_mt0, 0x2000)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_mt0a, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_mt1, 0x2000)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_mt1a, 0x0010)));

	*((uint32_t*)get_dev_ptr(dev_mt0a, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); // quantity fwdMT  	
	*((uint32_t*)get_dev_ptr(dev_mt1a, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); // quantity fwdMT  
		
	for(int i=0; i<624; i++)
		*((uint32_t*)get_dev_ptr(dev_mt0b, i*0x04)) = seeds0[i];  // seeds MT
	
	for(int i=0; i<624; i++)
		*((uint32_t*)get_dev_ptr(dev_mt1b, i*0x04)) = seeds1[i];  // seeds MT

	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_mt0, 0x2000)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_mt0a, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_mt1, 0x2000)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_mt1a, 0x0010)));
	
	// -----------------------------------------------------------------
	// BM  
	printf("\n");
	printf("setting BM ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)));
		
	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 
	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 		
	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0018)) = (uint32_t) (PATHSMC * STEPSMC /2); 
	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0018)) = (uint32_t) (PATHSMC * STEPSMC /2); 
	
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)));
	
	// -----------------------------------------------------------------
	// CORR
	printf("\n");
	printf("setting CORR ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)));
		
	float sqrt_1_minus_rho2 = (float) sqrtf(1-(rho*rho));
	
	*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC *2); 	
	*((float*)   get_dev_ptr(dev_corr, 0x0018)) = rho; 								
	*((float*)   get_dev_ptr(dev_corr, 0x0020)) = sqrt_1_minus_rho2; 				
			
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)));
	
	// -----------------------------------------------------------------
	// BS
	printf("\n");
	printf("setting BS ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0020)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0020)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0020)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0020)));
	
	float dt		= T /((float) STEPSMC);
	float m11 		= ((r-q)-0.5*sigma1*sigma1)*dt;
	float m12 		= sigma1 * sqrtf(dt);
	float m21 		= ((r-q)-0.5*sigma2*sigma2)*dt;
	float m22 		= sigma2 * sqrtf(dt);
	
	float discount  = expf(-r*dt);
		
	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)) = (uint32_t) STEPSMC; 	
	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)) = (uint32_t) PATHSMC; 	
	*((float*)   get_dev_ptr(dev_bs0, 0x0020)) = S0_1; 
	*((float*)   get_dev_ptr(dev_bs0, 0x0028)) = m11; 
	*((float*)   get_dev_ptr(dev_bs0, 0x0030)) = m12; 
		
	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)) = (uint32_t) STEPSMC; 	
	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)) = (uint32_t) PATHSMC; 	
	*((float*)   get_dev_ptr(dev_bs1, 0x0020)) = S0_2; 
	*((float*)   get_dev_ptr(dev_bs1, 0x0028)) = m21; 
	*((float*)   get_dev_ptr(dev_bs1, 0x0030)) = m22;  
	
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs0, 0x0020)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0020)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)));
	printf("----*((uint32_t*)get_dev_ptr(dev_bs1, 0x0020)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0020)));
		
	// -----------------------------------------------------------------
	// MAX
	printf("\n");
	printf("setting MAX ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_max, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_max, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_max, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_max, 0x0018)));
		
	*((uint32_t*)get_dev_ptr(dev_max, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_max, 0x0018)) = (uint32_t) PATHSMC; 		
			
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_max, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_max, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_max, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_max, 0x0018)));
		
	// -----------------------------------------------------------------
	// LSDATAGEN
	printf("\n");
	printf("setting LSDATAGEN ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_pathstester, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_pathstester, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0018)));
	
	
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsdatagen, 0x0020)) = K;		
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0028)) = callPut; 		
		
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_pathstester, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_pathstester, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0018)));
		
	// -----------------------------------------------------------------
	// LSDATAACC
	printf("\n");
	printf("setting LSDATAACC ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)));
		
	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)) = (uint32_t) PATHSMC; 		
			
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)));
		
	// -----------------------------------------------------------------
	// CHOL
	printf("\n");
	printf("setting CHOL ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)));
		
	*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)) = (uint32_t) STEPSMC;		
	
	for(int i=0; i<1024; i++)
	{
		*((float*)get_dev_ptr(dev_cholb0, i*0x04)) = (float)(0x0000FF00 + i); 
		*((float*)get_dev_ptr(dev_cholb1, i*0x04)) = (float)(0x0000FF00 + i); 
		*((float*)get_dev_ptr(dev_cholb2, i*0x04)) = (float)(0x0000FF00 + i); 
	}	
	
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)));
		
	// -----------------------------------------------------------------
	// LSUPDATE1
	printf("\n");
	printf("setting LSUPDATE1 ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)));
		
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsupdate1, 0x0020)) = K;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0028)) = callPut; 		
		
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)));
		
	// -----------------------------------------------------------------
	// LSUPDATE2
	printf("\n");
	printf("setting LSUPDATE2 ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)));	
	
	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsupdate2, 0x0020)) = discount;		
			
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)));
	printf("----*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)));
		
	// -----------------------------------------------------------------
	// CFLOWACC
	printf("\n");
	printf("setting CFLOWACC ...\n");
	
	printf("--before\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)));
	
	*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)) = (uint32_t) PATHSMC; 		
				
	printf("--after\n");	
	printf("----*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)) = %d\n",	*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)));
	
	// -----------------------------------------------------------------
	printf("RUNNING-ONLY TEST\n");
	printf("\n");
	printf("Triggering all modules except MT...\n");	
	
	status_bm0 			|= 0x01;
	status_bm1 			|= 0x01;
	status_corr			|= 0x01;
	status_bs0 			|= 0x01;
	status_bs1 			|= 0x01;
	status_max 			|= 0x01;
	status_lsdatagen 	|= 0x01;
	status_lsdataacc 	|= 0x01;
	status_chol 		|= 0x01;
	status_lsupdate1 	|= 0x01;
	status_lsupdate2 	|= 0x01;
	status_cflowacc 	|= 0x01;
	
	printf("status_mt0:        %08x\n", status_mt0);
	printf("status_mt1:        %08x\n", status_mt1);
	printf("status_bm0:        %08x\n", status_bm0);
	printf("status_bm1:        %08x\n", status_bm1);
	printf("status_corr:       %08x\n", status_corr);
	printf("status_bs0:        %08x\n", status_bs0);
	printf("status_bs1:        %08x\n", status_bs1);
	printf("status_max:        %08x\n", status_max);
	printf("status_lsdatagen:  %08x\n", status_lsdatagen);
	printf("status_lsdataacc:  %08x\n", status_lsdataacc);
	printf("status_chol:       %08x\n", status_chol);
	printf("status_lsupdate1:  %08x\n", status_lsupdate1);
	printf("status_lsupdate2:  %08x\n", status_lsupdate2);
	printf("status_cflowacc:   %08x\n", status_cflowacc);

	printf("Triggering MT...\n");	
	status_mt0 			|= 0x01;
	status_mt1 			|= 0x01;
	
	printf("Execution has started ...\n");
	printf("status_mt0:        %08x\n", status_mt0);
	printf("status_mt1:        %08x\n", status_mt1);
	printf("\n");
	printf("\n");
	// --------------------------------------------------------------
	// DEBUG CHECK
	while ( (status_mt0&0x0000000E)== 0)  		{ printf("status_mt0:       %08x\n", status_mt0); }
	printf("mt0 has finished (%08x)\n",status_mt0);
	while ( (status_mt1&0x0000000E)== 0) 		{ printf("status_mt1:       %08x\n", status_mt1); }
	printf("mt1 has finished (%08x)\n",status_mt1);
	while ( (status_bm0&0x0000000E)== 0) 		{ printf("status_bm0:       %08x\n", status_bm0); }
	printf("bm0 has finished (%08x)\n",status_bm0);
	while ( (status_bm1&0x0000000E)== 0) 		{ printf("status_bm1:       %08x\n", status_bm1); }
	printf("bm1 has finished (%08x)\n",status_bm1);
	while ( (status_corr&0x0000000E)== 0)		{ printf("status_corr:      %08x\n", status_corr); }
	printf("corr has finished (%08x)\n",status_corr);
	while ( (status_bs0&0x0000000E)== 0) 		{ printf("status_bs0:       %08x\n", status_bs0); }
	printf("bs0 has finished (%08x)\n",status_bs0);
	while ( (status_bs1&0x0000000E)== 0) 		{ printf("status_bs1:       %08x\n", status_bs1); }
	printf("bs1 has finished (%08x)\n",status_bs1);
	while ( (status_max&0x0000000E)== 0) 		{ printf("status_max:       %08x\n", status_max); }
	printf("max has finished (%08x)\n",status_max);
	while ( (status_lsdatagen&0x0000000E)== 0) 	{ printf("status_lsdatagen: %08x\n", status_lsdatagen); }
	printf("lsdatagen has finished (%08x)\n",status_lsdatagen);
	while ( (status_lsdataacc&0x0000000E)== 0) 	{ printf("status_lsdataacc: %08x\n", status_lsdataacc); }
	printf("lsdataacc has finished (%08x)\n",status_lsdataacc);
	while ( (status_chol&0x0000000E)== 0) 		{ printf("status_chol:      %08x\n", status_chol); }
	printf("chol has finished (%08x)\n",status_chol);
	while ( (status_lsupdate1&0x0000000E)== 0) 	{ printf("status_lsupdate1: %08x\n", status_lsupdate1); }
	printf("lsupdate1 has finished (%08x)\n",status_lsupdate1);
	while ( (status_lsupdate2&0x0000000E)== 0) 	{ printf("status_lsupdate2: %08x\n", status_lsupdate2); }
	printf("lsupdate2 has finished (%08x)\n",status_lsupdate2);
	while ( (status_cflowacc&0x0000000E)== 0) 	{ printf("status_cflowacc:  %08x\n", status_cflowacc); }
	printf("cflowacc has finished (%08x)\n",status_cflowacc);
	// --------------------------------------------------------------
	
	float totalSum = *((float*)get_dev_ptr(dev_cflowacc, 0x0018));
	float optionValue = totalSum / (float) PATHSMC;
	printf("LS FPGA ready!\n");
	
	printf("\n");
	printf("=================================================\n");
	printf("--Option's Parameters\n");
	printf("--T        = %.2f\n",T);
	printf("--S0_1     = %.2f\n",S0_1);
	printf("--S0_2     = %.2f\n",S0_2);
	printf("--K        = %.2f\n",K);
	printf("--callPut  = %s\n",(callPut == 0)?"call":"put");
	printf("--sigma1   = %.2f\n",sigma1);
	printf("--sigma2   = %.2f\n",sigma2);
	printf("--r        = %.2f\n",r);
	printf("--q        = %.2f\n",q);
	printf("--rho      = %.2f\n",rho);
	printf("--m11      = %.6f\n",m11);
	printf("--m12      = %.6f\n",m12);
	printf("--m21      = %.6f\n",m21);
	printf("--m22      = %.6f\n",m22);
	printf("--dt       = %.6f\n",dt);	
	printf("--discount = %.6f\n",discount);	
	printf("=================================================\n");
	printf("FPGA totalSum     = %.4f\n",totalSum);
	printf("=================================================\n");
	printf("FPGA optionValue                 = %.4f\n",optionValue);
	printf("=================================================\n");
	
	close_dev(dev_mt0a);
	close_dev(dev_mt0b);
	close_dev(dev_mt1a);
	close_dev(dev_mt1b);
	close_dev(dev_bm0);
	close_dev(dev_bm1);
	close_dev(dev_corr);
	close_dev(dev_bs0);
	close_dev(dev_bs1);
	close_dev(dev_max);
	close_dev(dev_lsdatagen);
	close_dev(dev_lsdataacc);
	close_dev(dev_chol);
	close_dev(dev_cholb0);
	close_dev(dev_cholb1);
	close_dev(dev_cholb2);
	close_dev(dev_cholpeek);
	close_dev(dev_lsupdate1);
	close_dev(dev_lsupdate2);
	close_dev(dev_cflowacc);
	
	return;
}
// ---------------------------------------------------------------

void BitstreamRepetition (const int amount)
{
	struct timespec startTimer, finishTimer;
	double elapsedTimeFPGAbitstream;
	
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);		
	
	printf("\n");
	printf("Programming FPGA (bitstream) multiple times for power measurement ... \n");
	
	for(int i=0; i< amount; ++i)
	{
		system("cat ./design_1_wrapper.bit.bin > /dev/xdevcfg");	
	}
		
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	elapsedTimeFPGAbitstream = (finishTimer.tv_sec - startTimer.tv_sec) * 1000.0;
	elapsedTimeFPGAbitstream += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000.0;
	printf("Programming FPGA (bitstream) multiple times: ready !! (%.3f ms)\n",elapsedTimeFPGAbitstream);
}
// ---------------------------------------------------------------






// ====================================================================
// MAIN
// ====================================================================
int main(int argc, char *argv[])
{
	// -----------------------------------------------------------------	
	// PREPARE TIMERS	
	// -----------------------------------------------------------------	
	struct timespec startTimer, finishTimer;
	double elapsedTimeFPGAtotal, elapsedTimeFPGAbitstream, elapsedTimeFPGAsetup, elapsedTimeFPGArun, elapsedTimeCPUGolden, elapsedTimeCPUGoldenAccum;
		
	
	// -----------------------------------------------------------------	
	// PROGRAM FPGA
	// -----------------------------------------------------------------	
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);		
	
	printf("\n");
	printf("Programming FPGA (bitstream) ... \n");
	
	// NOTE: this line is taken (and commented) from ./run.sh
	system("cat ./design_1_wrapper.bit.bin > /dev/xdevcfg");	
		
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	elapsedTimeFPGAbitstream = (finishTimer.tv_sec - startTimer.tv_sec) * 1000.0;
	elapsedTimeFPGAbitstream += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000.0;
	printf("Programming FPGA (bitstream): ready !!\n");
	
	// ---------------------------------
	// CHECK FPGA EXECUTION (DEBUG MODE) (OPTIONAL)
	CheckOperationFPGA ();
	// ---------------------------------
	
	printf("\n\n\n");
	printf("------------------------------------------------\n");
	printf("       Option Pricing: American 2D max          \n");
	printf("------------------------------------------------\n");
	IO_DEV dev_mt0a			= open_dev(ADDRESS_MT0a);
	IO_DEV dev_mt0b			= open_dev(ADDRESS_MT0b);
	IO_DEV dev_mt1a			= open_dev(ADDRESS_MT1a);
	IO_DEV dev_mt1b			= open_dev(ADDRESS_MT1b);
	IO_DEV dev_bm0			= open_dev(ADDRESS_BM0);
	IO_DEV dev_bm1			= open_dev(ADDRESS_BM1);
	IO_DEV dev_corr		 	= open_dev(ADDRESS_CORR);
	IO_DEV dev_bs0		 	= open_dev(ADDRESS_BS0);
	IO_DEV dev_bs1			= open_dev(ADDRESS_BS1);
	IO_DEV dev_max			= open_dev(ADDRESS_MAX);
	IO_DEV dev_lsdatagen	= open_dev(ADDRESS_LSDATAGEN);
	IO_DEV dev_lsdataacc 	= open_dev(ADDRESS_LSDATAACC);
	IO_DEV dev_chol			= open_dev(ADDRESS_CHOL);
	IO_DEV dev_cholb0		= open_dev(ADDRESS_CHOLB0);
	IO_DEV dev_cholb1		= open_dev(ADDRESS_CHOLB1);
	IO_DEV dev_cholb2		= open_dev(ADDRESS_CHOLB2);
	IO_DEV dev_cholpeek		= open_dev(ADDRESS_CHOLPEEK);
	IO_DEV dev_lsupdate1	= open_dev(ADDRESS_LSUPDATE1);
	IO_DEV dev_lsupdate2	= open_dev(ADDRESS_LSUPDATE2);
	IO_DEV dev_cflowacc		= open_dev(ADDRESS_CFLOWACC);
	
	// -----------------------------------------------------------------
	// Common Operations
	// -----------------------------------------------------------------	
	// Paths Generation additional variables
	float sqrt_1_minus_rho2 = (float) sqrtf(1-(rho*rho));
	float dt		= T /((float) STEPSMC);
	float m11 		= ((r-q)-0.5*sigma1*sigma1)*dt;
	float m12 		= sigma1 * sqrtf(dt);
	float m21 		= ((r-q)-0.5*sigma2*sigma2)*dt;
	float m22 		= sigma2 * sqrtf(dt);
	
	float discount  = expf(-r*dt);
				
	// -----------------------------------------------------------------
	// FPGA CONTROL AND SETUP
	// -----------------------------------------------------------------
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);	
	
	volatile unsigned &status_mt0 = *((unsigned*)get_dev_ptr(dev_mt0a, 0));
	volatile unsigned &status_mt1 = *((unsigned*)get_dev_ptr(dev_mt1a, 0));
	volatile unsigned &status_bm0 = *((unsigned*)get_dev_ptr(dev_bm0, 0));
	volatile unsigned &status_bm1 = *((unsigned*)get_dev_ptr(dev_bm1, 0));
	volatile unsigned &status_corr = *((unsigned*)get_dev_ptr(dev_corr, 0));
	volatile unsigned &status_bs0 = *((unsigned*)get_dev_ptr(dev_bs0, 0));
	volatile unsigned &status_bs1 = *((unsigned*)get_dev_ptr(dev_bs1, 0));
	volatile unsigned &status_max = *((unsigned*)get_dev_ptr(dev_max, 0));
	volatile unsigned &status_lsdatagen = *((unsigned*)get_dev_ptr(dev_lsdatagen, 0));
	volatile unsigned &status_lsdataacc = *((unsigned*)get_dev_ptr(dev_lsdataacc, 0));
	volatile unsigned &status_chol = *((unsigned*)get_dev_ptr(dev_chol, 0));
	volatile unsigned &status_lsupdate1 = *((unsigned*)get_dev_ptr(dev_lsupdate1, 0));
	volatile unsigned &status_lsupdate2 = *((unsigned*)get_dev_ptr(dev_lsupdate2, 0));
	volatile unsigned &status_cflowacc = *((unsigned*)get_dev_ptr(dev_cflowacc, 0));
		
	// -----------------------------------------------------------------
	// MT
	uint32_t seeds0[624];
	uint32_t seeds1[624];
	
	// generate seeds 0
	init_genrand(seedValue1);
	generate_numbers();
	get_mt_state(seeds0);
	
	// generate seeds 1
	init_genrand(seedValue2);
	generate_numbers();
	get_mt_state(seeds1);
	
	*((uint32_t*)get_dev_ptr(dev_mt0a, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 
	*((uint32_t*)get_dev_ptr(dev_mt1a, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 
		
	for(int i=0; i<624; i++)
		*((uint32_t*)get_dev_ptr(dev_mt0b, i*0x04)) = seeds0[i]; 
	
	for(int i=0; i<624; i++)
		*((uint32_t*)get_dev_ptr(dev_mt1b, i*0x04)) = seeds1[i];  
	
	// -----------------------------------------------------------------
	// BM  
	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 
	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC); 		
	*((uint32_t*)get_dev_ptr(dev_bm0, 0x0018)) = (uint32_t) (PATHSMC * STEPSMC /2); 
	*((uint32_t*)get_dev_ptr(dev_bm1, 0x0018)) = (uint32_t) (PATHSMC * STEPSMC /2); 
	
	// -----------------------------------------------------------------
	// CORR
	*((uint32_t*)get_dev_ptr(dev_corr, 0x0010)) = (uint32_t) (PATHSMC * STEPSMC *2); 	
	*((float*)   get_dev_ptr(dev_corr, 0x0018)) = rho; 								
	*((float*)   get_dev_ptr(dev_corr, 0x0020)) = sqrt_1_minus_rho2; 				
	
	// -----------------------------------------------------------------
	// BS
	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0010)) = (uint32_t) STEPSMC; 	
	*((uint32_t*)get_dev_ptr(dev_bs0, 0x0018)) = (uint32_t) PATHSMC; 	
	*((float*)   get_dev_ptr(dev_bs0, 0x0020)) = S0_1; 
	*((float*)   get_dev_ptr(dev_bs0, 0x0028)) = m11; 
	*((float*)   get_dev_ptr(dev_bs0, 0x0030)) = m12; 
		
	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0010)) = (uint32_t) STEPSMC; 	
	*((uint32_t*)get_dev_ptr(dev_bs1, 0x0018)) = (uint32_t) PATHSMC; 	
	*((float*)   get_dev_ptr(dev_bs1, 0x0020)) = S0_2; 
	*((float*)   get_dev_ptr(dev_bs1, 0x0028)) = m21; 
	*((float*)   get_dev_ptr(dev_bs1, 0x0030)) = m22;  
	
	// -----------------------------------------------------------------
	// MAX
	*((uint32_t*)get_dev_ptr(dev_max, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_max, 0x0018)) = (uint32_t) PATHSMC; 		
	
	// -----------------------------------------------------------------
	// LSDATAGEN
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsdatagen, 0x0020)) = K;		
	*((uint32_t*)get_dev_ptr(dev_lsdatagen, 0x0028)) = callPut; 		
		
	// -----------------------------------------------------------------
	// LSDATAACC
	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsdataacc, 0x0018)) = (uint32_t) PATHSMC; 		
		
	// -----------------------------------------------------------------
	// CHOL
	*((uint32_t*)get_dev_ptr(dev_chol, 0x0010)) = (uint32_t) STEPSMC;		
		
	// -----------------------------------------------------------------
	// LSUPDATE1
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsupdate1, 0x0020)) = K;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate1, 0x0028)) = callPut; 		
	
	// -----------------------------------------------------------------
	// LSUPDATE2
	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0010)) = (uint32_t) STEPSMC;		
	*((uint32_t*)get_dev_ptr(dev_lsupdate2, 0x0018)) = (uint32_t) PATHSMC; 		
	*((float*)   get_dev_ptr(dev_lsupdate2, 0x0020)) = discount;		
		
	// -----------------------------------------------------------------
	// CFLOWACC
	*((uint32_t*)get_dev_ptr(dev_cflowacc, 0x0010)) = (uint32_t) PATHSMC; 		
	
	// -----------------------------------------------------------------
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	elapsedTimeFPGAsetup = (finishTimer.tv_sec - startTimer.tv_sec);
	elapsedTimeFPGAsetup += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000000.0;
	
	// -----------------------------------------------------------------
	// FPGA RUN
	// -----------------------------------------------------------------
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);	

	// NOTE: since Mersenne-Twister (MT) needs to generate all paths in forward mode
	//       before delivering the paths in reverse mode, the most efficient triggering
	//       sequence for this architecture is to start with MTs:
	status_mt0 			|= 0x01;
	status_mt1 			|= 0x01;
	status_bm0 			|= 0x01;
	status_bm1 			|= 0x01;
	status_corr			|= 0x01;
	status_bs0 			|= 0x01;
	status_bs1 			|= 0x01;
	status_max 			|= 0x01;
	status_lsdatagen 	|= 0x01;
	status_lsdataacc 	|= 0x01;
	status_chol 		|= 0x01;
	status_lsupdate1 	|= 0x01;
	status_lsupdate2 	|= 0x01;
	status_cflowacc 	|= 0x01;
	
		
	// NOTE: only need to check when CashFlowAccumulator (status_cflowacc) is ready	
	while ( (status_cflowacc&0x0000000E)== 0) 	{} 
	
	float totalSum = *((float*)get_dev_ptr(dev_cflowacc, 0x0018));
	float optionValue = totalSum / (float) PATHSMC;
		
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	elapsedTimeFPGArun = (finishTimer.tv_sec - startTimer.tv_sec) * 1000.0;
	elapsedTimeFPGArun += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000.0;
	
	elapsedTimeFPGAtotal = elapsedTimeFPGAbitstream + elapsedTimeFPGAsetup + elapsedTimeFPGArun;
	
	printf("LS FPGA ready!\n");
	// --------------------------------------------------------------
	
	// --------------------------------------------------------------
	// CPU GOLDEN VALUE 
	// --------------------------------------------------------------
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);	
	
	float optionValueGolden = ls_golden ( 	STEPSMC, PATHSMC,
											S0_1, S0_2,
											(callPut==0)?1:-1,  // callPut = 1 (call); callPut = -1 (put)
											rho, sqrt_1_minus_rho2,
											K, discount,
											m11, m12,
											m21, m22,
											seedValue1,
											seedValue2 );
	
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	printf("LS CPU Golden ready!\n");											
	elapsedTimeCPUGolden = (finishTimer.tv_sec - startTimer.tv_sec) * 1000.0;
	elapsedTimeCPUGolden += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000.0;
	
	// --------------------------------------------------------------
	// CPU GOLDEN VALUE (ACCUM VERSION)
	// --------------------------------------------------------------
	//clock_gettime(CLOCK_MONOTONIC, &startTimer);
	clock_gettime(CLOCK_REALTIME, &startTimer);	
	
	float optionValueGoldenAccum = ls_goldenAccum ( 	STEPSMC, PATHSMC,
											S0_1, S0_2,
											(callPut==0)?1:-1,  // callPut = 1 (call); callPut = -1 (put)
											rho, sqrt_1_minus_rho2,
											K, discount,
											m11, m12,
											m21, m22,
											seedValue1,
											seedValue2 );
	//clock_gettime(CLOCK_MONOTONIC, &finishTimer);
	clock_gettime(CLOCK_REALTIME, &finishTimer);
	printf("LS CPU Golden (Accum version) ready!\n");																			
	elapsedTimeCPUGoldenAccum = (finishTimer.tv_sec - startTimer.tv_sec) * 1000.0;
	elapsedTimeCPUGoldenAccum += (finishTimer.tv_nsec - startTimer.tv_nsec) / 1000000.0;
	
	// --------------------------------------------------------------
	// RESULTS
	// --------------------------------------------------------------
	const double choleskyClockCycles    = 197 * STEPSMC;
	const double cflowAccumClockCycles  = PATHSMC;
	const double estimatedClockCycles   = (PATHSMC * STEPSMC * 2 ) + choleskyClockCycles + cflowAccumClockCycles;
	const double estimatedRuntime       = (estimatedClockCycles / freqFPGA) / 1000.0;
	printf("\n");
	printf("=================================================\n");
	printf("--Option's Parameters\n");
	printf("--T        = %.2f\n",T);
	printf("--S0_1     = %.2f\n",S0_1);
	printf("--S0_2     = %.2f\n",S0_2);
	printf("--K        = %.2f\n",K);
	printf("--callPut  = %s\n",(callPut == 0)?"call":"put");
	printf("--sigma1   = %.2f\n",sigma1);
	printf("--sigma2   = %.2f\n",sigma2);
	printf("--r        = %.2f\n",r);
	printf("--q        = %.2f\n",q);
	printf("--rho      = %.2f\n",rho);
	printf("--m11      = %.6f\n",m11);
	printf("--m12      = %.6f\n",m12);
	printf("--m21      = %.6f\n",m21);
	printf("--m22      = %.6f\n",m22);
	printf("--dt       = %.6f\n",dt);	
	printf("--discount = %.6f\n",discount);	
	printf("=================================================\n");
	printf("FPGA totalSum     = %.4f\n",totalSum);
	printf("=================================================\n");
	printf("FPGA optionValue                 = %.4f\n",optionValue);
	printf("CPU  optionValue                 = %.4f\n",optionValueGolden);
	printf("CPU  optionValue (Accum version) = %.4f\n",optionValueGoldenAccum);
	printf("=================================================\n");
	printf("FPGA ElapsedTime (total)   = %8.3f ms\n",elapsedTimeFPGAtotal);	
	printf("CPU Golden                 = %8.3f ms\n",elapsedTimeCPUGolden);	
	printf("CPU Golden (Accum version) = %8.3f ms\n",elapsedTimeCPUGoldenAccum);	
	printf("=================================================\n");	
	printf("FPGA ElapsedTime (total)   = %8.3f ms  @ %.2fMHz (clock frequency)\n",elapsedTimeFPGAtotal, freqFPGA);	
	printf("------------ (bitstream)   = %8.3f ms  (%3.3f%% of total)\n",elapsedTimeFPGAbitstream, elapsedTimeFPGAbitstream/elapsedTimeFPGAtotal*100);	
	printf("---------------- (setup)   = %8.3f ms  (%3.3f%% of total)\n",elapsedTimeFPGAsetup, elapsedTimeFPGAsetup/elapsedTimeFPGAtotal*100);	
	printf("------------------ (run)   = %8.3f ms  (%3.3f%% of total)\n",elapsedTimeFPGArun, elapsedTimeFPGArun/elapsedTimeFPGAtotal*100);	
	printf("FPGA Theoretical runtime   = %8.3f ms  @ %.2fMHz (clock frequency)\n",estimatedRuntime, freqFPGA);
	printf("=================================================\n");	
	//printf("FPGA bitstream config.     = %8.3f ms\n",elapsedTimeFPGAbitstream);
	//printf("=================================================\n");
	
	// -----------------------------------------------------------------
	// -----------------------------------------------------------------
	
	// ---------------------------------
	// BITSTREAM REPETITION FOR POWER CONSUMPTION (OPTIONAL)
	BitstreamRepetition (10);
	// -----------------------------------------------------------------
	// -----------------------------------------------------------------
	
	close_dev(dev_mt0a);
	close_dev(dev_mt0b);
	close_dev(dev_mt1a);
	close_dev(dev_mt1b);
	close_dev(dev_bm0);
	close_dev(dev_bm1);
	close_dev(dev_corr);
	close_dev(dev_bs0);
	close_dev(dev_bs1);
	close_dev(dev_max);
	close_dev(dev_lsdatagen);
	close_dev(dev_lsdataacc);
	close_dev(dev_chol);
	close_dev(dev_cholb0);
	close_dev(dev_cholb1);
	close_dev(dev_cholb2);
	close_dev(dev_cholpeek);
	close_dev(dev_lsupdate1);
	close_dev(dev_lsupdate2);
	close_dev(dev_cflowacc);
	
	return 0;
}


