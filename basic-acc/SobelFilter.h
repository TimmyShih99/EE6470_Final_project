#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;
using namespace std;
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <algorithm>
#include "filter_def.h"

struct SobelFilter : public sc_module {
  tlm_utils::simple_target_socket<SobelFilter> tsock;

  /*sc_fifo<unsigned char> i_r;
  sc_fifo<unsigned char> i_g;
  sc_fifo<unsigned char> i_b;
  sc_fifo<int> o_result; // + unsigned*/

  sc_fifo<unsigned char> i_0;
  sc_fifo<unsigned int> o_result;
  sc_fifo<unsigned int> o_compute_cycle;  // FIFO channel for compute cycle

  SC_HAS_PROCESS(SobelFilter);

  SobelFilter(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0) 
  {
    tsock.register_b_transport(this, &SobelFilter::blocking_transport);
    SC_THREAD(do_filter);
  }

  ~SobelFilter() {
	}
  
  unsigned int base_offset;
  sc_time start_compute, end_compute;  // SystemC events

  void do_filter(){   //改這
    cout << "Start Filter::do_filter" << endl;
    { wait(CLOCK_PERIOD, SC_NS); }

    while (true) {
      //cout << "Start do" << endl;
      int rows[rows_size] = {0};             // rows_size = 64;
      int beads[arr_size][rows_size] = {0};  // arr_size = 10;
      int arr[arr_size] = {0};
      
      // Setting the beads
    	for (unsigned int i = 0; i < arr_size; ++i) {
        	for (unsigned int j = 0; j < rows_size; ++j) {
            	beads[i][j] = i_0.read();
              //cout << beads[i][j] ;
        	}
          //cout << "i = " << i <<  endl;
    	}

      cout << "Beads before gravity step: " << endl;
      for (const auto& row : beads) {
          for (int bead : row) {
              cout << bead << " ";
          }
          cout << endl;
      }

      // Gravity step
      for (unsigned int j = 0; j < rows_size; ++j) {
          int sum = 0;
          for (unsigned int i = 0; i < arr_size; ++i) {
              sum += beads[i][j];
              beads[i][j] = 0;
          }
          for (unsigned int i = arr_size - sum; i < arr_size; ++i) {
              beads[i][j] = 1;
          }
      }

      
      for (unsigned int j = 0; j < rows_size; ++j) {
          int sum = 0;
          for (unsigned int i = 0; i < arr_size; ++i) {
              sum += beads[i][j];
          }
          for (unsigned int i = 0; i < arr_size; ++i) {
              beads[i][j] = (i < sum);
          }
      }

      cout << "Beads after gravity step:\n";
      for (const auto& row : beads) {
          for (int bead : row) {
              cout << bead << " ";
          }
          cout << endl;
      }

      // Collect the beads back into the array
      for (int i = 0; i < arr_size; ++i) {
          int j;
          for (j = 0; j < rows_size && beads[i][j]; ++j);
          arr[i] = j;
      }
      // result
      for (int i = 0; i < arr_size; ++i) {
        	o_result.write( arr[i] );   
    	}

      //wait(CLOCK_PERIOD*1, SC_NS);
    }
  }          //到這

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    delay += sc_time(CLOCK_PERIOD*1, SC_NS);
    //cout << "blocking_transport called" <<endl; 
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    wait(delay);
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    addr -= base_offset;


    // cout << (int)data_ptr[0] << endl;
    // cout << (int)data_ptr[1] << endl;
    // cout << (int)data_ptr[2] << endl;
    word buffer;
    //cout << cmd << endl;
    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        // cout << "READ" << endl;
        switch (addr) {
          case SOBEL_FILTER_RESULT_ADDR:
            buffer.uint = o_result.read();
            break;
          default:
            std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        data_ptr[0] = 1;
        data_ptr[1] = buffer.uint;
        data_ptr[2] = 1;
        data_ptr[3] = 1;
        //cout << int(buffer.uc[0]) <<", "<< int(buffer.uc[3]) <<" ; ";
        break;

      case tlm::TLM_WRITE_COMMAND:
        // cout << "WRITE" << endl;
        switch (addr) {
          case SOBEL_FILTER_R_ADDR:
            i_0.write(data_ptr[1]); // i_0.write
            //cout << "data_ptr[0]: " << data_ptr[1] << endl;
            //std::cerr << data_ptr[0] << ;
            break;
          default:
            std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
        }
        break;
      case tlm::TLM_IGNORE_COMMAND:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      default:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
};
#endif
