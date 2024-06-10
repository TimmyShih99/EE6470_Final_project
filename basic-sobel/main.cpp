#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

#define PROCESSORS 2
#define STIM_LEN 10
#define GRID_SIZE 8
#define DMA_BANDWIDTH 4
#define INITIAL_CYCLES 0

/*unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};*/

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

/*unsigned int input_rgb_raw_data_offset;
const unsigned int output_rgb_raw_data_offset=54;
int width;
int height;
unsigned int width_bytes;
unsigned char bits_per_pixel;
unsigned short bytes_per_pixel;
unsigned char *source_bitmap;
unsigned char *target_bitmap;
const int WHITE = 255;
const int BLACK = 0;
const int THRESHOLD = 90;*/

// Sobel Filter ACC
static char* const SOBELFILTER_START_ADDR = reinterpret_cast<char* >(0x73000000);
static char* const SOBELFILTER_READ_ADDR  = reinterpret_cast<char* >(0x73000004);

// DMA addresses
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * )0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * )0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * )0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * )0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * )0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

// DMA parameters
bool _is_using_dma = true;
int write_cycles = 0;
int read_cycles = 0;

// global memory for the response
unsigned int response[STIM_LEN] = {0};


void write_data_to_ACC(char* ADDR, volatile unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    //std::cout << "buffer: " << buffer << std::endl;
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
    write_cycles += INITIAL_CYCLES + len / DMA_BANDWIDTH;
  }else{
    // Directly Send
    memcpy(ADDR, (void*)buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, volatile unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
    read_cycles += INITIAL_CYCLES + len / DMA_BANDWIDTH;
  }else{
    // Directly Read
    memcpy((void*)buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void store_response(const unsigned int response[], const std::string& filePath) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        // Write the array data to the file
        for(int i = 0; i < STIM_LEN; i++) {
            outputFile << response[i] << "\n";
        }
        outputFile.close();
        std::cout << "Response stored successfully in the file." << std::endl;
    } else {
        std::cout << "Failed to open the file for writing." << std::endl;
    }
}

int main(int argc, char *argv[]) {

  FILE *infp = fopen("stimulus.dat", "r");
  
  volatile unsigned char  buffer[4] = {0};
  word data;
  
  for (int s = 0; s < STIM_LEN; s ++){  // 10列
    for (int i = 0; i < 5; i ++){  // 8*8=64列
      for (int j = 0; j < 4 ; j ++){
        int m;
        fscanf(infp, "%d ", &m);
			  bool b = m != 0;  // Convert integer to boolean
        buffer[0] = 1;
        buffer[1] = b;
        //printf( "%d", buffer[1]);
        buffer[2] = 1;
        buffer[3] = 0;
        write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
        //printf( "%d", buffer[0]);
      }
    fscanf(infp, "\n"); // To move to the next line
    }
  }
  for (int j = 0; j < 10 ; j ++){
    read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer, 4);
    //printf( "%d", buffer[0]);
    auto sample0 = buffer[0];
    auto sample1 = buffer[1];
    auto sample2 = buffer[2];
    for (;;) {
      sample0 = buffer[0];
      sample1 = buffer[1];
      sample2 = buffer[2];
      if (sample0 == 1 && sample2 == 1) {
        //std::cout << int(buffer[1]) << ", "  ;
        break;
      }
    }
    //for (int j = 0; j < 10 ; j ++){
      //std::cout << int(buffer[1]) << ", "  ;
    //}
    std::cout << int(sample1) << ", "  ;
    memcpy(data.uc, (void*)buffer, 4);
    response[j] = data.uint;
  }
  store_response(response, "response.dat");
  printf("Total Write Cycles: %d\n", write_cycles);
  printf("Total Read Cycles: %d\n", read_cycles);
  printf("Total DMA Cycles: %d\n", write_cycles + read_cycles);
  return 0;
}
