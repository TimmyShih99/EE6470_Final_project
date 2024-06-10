## Path Counter Implementation Using Status HLS


This is the project that implements ee6470 final project q1


## Usage
1. Clone the file to ./ee6470 folder (Git Bash)
```properties
git clone https://github.com/cmosinverter/ee6470-final-q1.git path_count
```
2. Source the setup file
```properties
cd path_count
```
```properties
source staratus_env.sh
```
3. Go to stratus directory
```properties
cd stratus
```
4. Run SystemC-based behavioural simulation
```properties
make sim_B
```
5. Run HLS synthesis and Verilog simulation (BASIC)
```properties
make sim_BASIC_V
```
6. Run HLS synthesis and Verilog simulation (Loop Unrolling)
```properties
make sim_FLAT_UNROLL_ALL_V
```
7. Run HLS synthesis and Verilog simulation (Loop Unrolling)
```properties
make sim_FLAT_UNROLL_ALL_FAST_V
```
8. Run HLS synthesis and Verilog simulation (DPA)
```properties
make sim_DPA_V
```

