///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Cadence Design Systems, Inc. All rights reserved worldwide.
//
// The code contained herein is the proprietary and confidential information
// of Cadence or its licensors, and is supplied subject to a previously
// executed license and maintenance agreement between Cadence and customer.
// This code is intended for use with Cadence high-level synthesis tools and
// may not be used with other high-level synthesis tools. Permission is only
// granted to distribute the code as indicated. Cadence grants permission for
// customer to distribute a copy of this code to any partner to aid in designing
// or verifying the customer's intellectual property, as long as such
// distribution includes a restriction of no additional distributions from the
// partner, unless the partner receives permission directly from Cadence.
//
// ALL CODE FURNISHED BY CADENCE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, AND CADENCE SPECIFICALLY DISCLAIMS ANY WARRANTY OF NONINFRINGEMENT,
// FITNESS FOR A PARTICULAR PURPOSE OR MERCHANTABILITY. CADENCE SHALL NOT BE
// LIABLE FOR ANY COSTS OF PROCUREMENT OF SUBSTITUTES, LOSS OF PROFITS,
// INTERRUPTION OF BUSINESS, OR FOR ANY OTHER SPECIAL, CONSEQUENTIAL OR
// INCIDENTAL DAMAGES, HOWEVER CAUSED, WHETHER FOR BREACH OF WARRANTY,
// CONTRACT, TORT, NEGLIGENCE, STRICT LIABILITY OR OTHERWISE.
//
////////////////////////////////////////////////////////////////////////////////

#include "dut.h"
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

// The thread function for the design
void dut::thread1()
{
	// Reset the interfaces
	// Also initialize any variables that are part of the module class here
	{
		HLS_DEFINE_PROTOCOL( "reset" );
		din.reset();
		dout.reset();
		wait();
	}
while_1:
	while( true )
	{
	  HLS_CONSTRAIN_LATENCY( 0, HLS_ACHIEVABLE, "lat1" ); // reduce cycle counts
	  //HLS_PIPELINE_LOOP( HARD_STALL, 16, "main_loop_pipeline" );	// PIPELINE min 29
#if defined (II)
    // HLS_PIPELINE_LOOP(SOFT_STALL, 1, "Loop" );
#endif

		input_t rows[rows_size] = {0};  //整列為0
    HLS_FLATTEN_ARRAY(rows); // FLATTEN
		input_t beads[arr_size][rows_size] = {0};
    HLS_FLATTEN_ARRAY(beads); // FLATTEN
		//output_t arr[arr_size] = {0, 3, 1, 7, 4, 1, 1, 20, 30, 40}; //tb.cc裡有
		//input_t arr[arr_size] = {0, 3, 1, 7, 4, 1, 1, 20, 30, 40};
    input_t arr[arr_size] = {0};
    HLS_FLATTEN_ARRAY(arr); // FLATTEN
    for (int i = 0; i < arr_size; ++i) {
    HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        arr[i] = din.get();
    }  
   
		// Setting the beads
    	for (int i = 0; i < arr_size; ++i) {
      HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        	for (int j = 0; j < rows_size; ++j) {
          HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
            	beads[i][j] = din.get();
        	}
    	}

		printf( "Beads before gravity step:\n");
    	for (const auto& row : beads) {
        	for (int bead : row) {
            	printf(  "%d", bead ," ");
        	}
        	printf( " \n");
    	}
		
		{
#if defined(DPOPT_ALL)
       //HLS_DPOPT_REGION("entirecomputation");
#endif
#if defined(LAT)
         //HLS_CONSTRAIN_LATENCY( 0, LAT, "computation" );
#endif
beads_loop:  // Gravity step
    for (int j = 0; j < rows_size; ++j) {
    HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        int sum = 0;
        for (int i = 0; i < arr_size; ++i) {
        HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
            sum += beads[i][j];
            beads[i][j] = 0;
        }
        for (int i = arr_size - sum; i < arr_size; ++i) {
        HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
            beads[i][j] = 1;
        }
    }
    for (int j = 0; j < rows_size; ++j) {
    HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        int sum = 0;
        for (int i = 0; i < arr_size; ++i) {
        HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
            sum += beads[i][j];
        }
        for (int i = 0; i < arr_size; ++i) {
        HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
            beads[i][j] = (i < sum);
        }
    }

    /*printf( "Beads after gravity step:\n");
    for (const auto& row : beads) {
        for (int bead : row) {
            printf( "%d", bead ," ");
        }
        printf( " \n");
    }*/
	// Collect the beads back into the array
    for (int i = 0; i < arr_size; ++i) {
    HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        int j;
        for (j = 0; j < rows_size && beads[i][j]; ++j);
        HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        arr[i] = j;
    }
		}
		//output_t out_val = beads[grid_size-1];
		for (int i = 0; i < arr_size; ++i) {
    HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
        	dout.put( arr[i] );   
    	}
	printf( "Beads after gravity step:\n");
    	for (const auto& row : beads) {
        	for (int bead : row) {
            		printf( "%d", bead );
        	}
        	printf( " \n");
    	}
		//dout.put( out_val );            // output the result
	}
}
