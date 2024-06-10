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

#include "tb.h"
#include <esc.h>		// for the latency logging functions
#include <string>
#include <iostream>
#include <vector>
// The source thread reads data from a file and sends it to the DUT
void tb::source()
{
	dout.reset();		// reset the outputs and cycle the design's reset
	rst.write( 0 );		// assert reset (active low)
	wait( 2 );		// wait 2 cycles
	rst.write( 1 );		// deassert reset
	wait();

	// Open the stimulus file
	// char stim_file[256] = "stimulus.dat";
	// infp = fopen( stim_file, "r" );
	// if( infp == NULL )
	// {
	//	 cout << "Couldn't open stimulus.dat for reading." << endl;
	//   exit(0);
	// }
	std::vector<int> numOnes = {0, 3, 1, 7, 4, 1, 1, 20, 17, 13}; // 定義每一列要放入的 1 的數目
  for(int i = 0; i < 10; i++) {
  	input_t a = numOnes[i];
  	dout.put(a);
  }
 
	std::vector<std::vector<int>> myArray(10, std::vector<int>(20, 0)); // 初始化二維向量
	// 將每一列的指定數目的元素設置為 1
    for (int i = 0; i < 10; ++i) {
        int ones = numOnes[i];
        for (int j = 0; j < ones; ++j) {
            myArray[i][j] = 1;
        }
    }
	// 印出二維向量
	std::cout << "Input Array: " << std::endl;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            std::cout << myArray[i][j] << " ";
        }
        std::cout << std::endl;
    }
	// Read stimulus values from file and send to DUT
	for(int i = 0; i < 10; i++) {  // Assuming there are 10 rows (stimuli)
		for(int j = 0; j < 20; j++) {  // For each value in the row
			//int m;
			//fscanf(infp, "%d ", &m);
			input_t b = myArray[i][j];  // 資料給到 input_t b
			dout.put(b);  // Send the bit value                                         // 只送dout.put(b)就好  不用fscanf
		}
		start_time[i] = sc_time_stamp(); // Mark the time value was sent
		//fscanf(infp, "\n"); // To move to the next line
	}

	// Guard condition: after 100000 cycles the sink() thread should have ended the simulation.
	// If we're still here, timeout and print error message.
	wait( 100000 );
    	//fclose( infp );
	cout << "Error! TB source thread timed out!" << endl;
	esc_stop();
}

// The sink thread reads all the expected values from the design
void tb::sink()
{
	din.reset();
	wait();     // to synchronize with reset

	// Extract clock period from clk port
	sc_clock * clk_p = dynamic_cast < sc_clock * >( clk.get_interface() );
	sc_time clock_period = clk_p->period();

	// Open the simulation results file
	char output_file[256];
	sprintf( output_file, "%s/response.dat", getenv("BDW_SIM_CONFIG_DIR") );
	outfp = fopen(output_file, "wb");
	if (outfp == NULL)
	{
		cout << "Couldn't open output.dat for writing." << endl;
		exit(0);
	}

	// Read outputs from DUT
	unsigned long total_latency = 0;
	for( int i = 0; i < 10; i++ )
	{
		output_t inVal = din.get();
		fprintf( outfp, "%d\n", (int)inVal );	// write value to response file

		// Calculate latency for this particular output
		unsigned long latency = (sc_time_stamp() - start_time[i]) / clock_period;
		cout << "Latency for sample " << i << " is " <<  latency << endl;

		// Keep running total of all latency cycles for later
		total_latency += latency;
	}

	// Calculate, log and print average latency
	unsigned long average_latency = total_latency / 10;
	esc_log_latency( "dut", average_latency, "average_latency" );
	cout << "Average latency " << average_latency << "." << endl;
	std::cout<< "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
	// Close the results file and end the simulation
	fclose( outfp );
	esc_stop();
}
