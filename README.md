## Beads sorting Implementation Using Status HLS


This is the project that implements ee6470 final project q1


## Usage
1. Clone the file to ./ee6470 folder (Git Bash)
```properties
git clone https://github.com/cmosinverter/ee6470-final-q1.git path_count
```
2. Source the setup file
```properties
cd Final_HLS
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
## As for the other two folders of RISCV-VP, please place 'basic-acc' in \ee6470\riscv-vp\vp\src\platform\ for execution; and place 'basic-sobel' in \ee6470\riscv-vp\sw\ for execution.

Replace the files with those ending in '_mc' to run multi-core.

```properties
cd $EE6470 &&\
cd riscv-vp/vp/build  &&\
cmake .. &&\
make install &&\
cd $EE6470 &&\
cd riscv-vp/sw &&\
cd basic-sobel &&\
make sim 
```

