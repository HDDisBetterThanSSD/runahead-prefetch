# runahead-prefetch
Runahead Prefetch code from Master's Thesis

This repo includes the beginnings of a multi-core multi-level cache simulator. Parts of this code was reused and modified to implement a simple L1i cache and runahead prefetcher.

# Dependencies
This simulator requires the zlib library and c++11.

# Install
cd into root directory. a Makefile should be present in that directory. execute:
```
$ make
```
or
```
$ make mp_tc_sim
```
You can also clean the build with:
```
$ make clean
```
The object files will be placed in the obj directory. the executable simulator will be named mp_tc_sim and will exist in the root directory.

# Usage
## Run
Execute the following for a basic simulation:
```
$ ./mp_tc_sim -t <trace_file>
```
## Configurations
Before every simulation the parameters used to configure the simulator are printed to the console. This implementation allows for a variety of configurations which can be configured in the Config.cpp file. Some of these configurations can also be changed by passing a command line option. Below includes a list of some of the configurations available:

- outfile: file that various high-level statistics from a simulation are outputed to, along with the terminal
- logfile: simulation details on a cycle-by-cycle granularity can be outputed here.
- debug_level: determines level of logfile detail
- l1i_standalone: This option configures the cache hierarchy and multi-core settings specifically for a runahead prefetch
- cpu_configs[#]: provides options for configuring the CPU, "#" is the cpu number (here only 1 cpu is instantiated)
 - cpu_configs[#].pf_pipe_width: Predict (Prefetch) Pipe Width (bytes) determines how many bytes are predicted by the branch predictor per cycle
 - cpu_configs[#].fe_pipe_width: Front-end Pipe Width (bytes) determines the number of bytes that are fetched per cycle
 - cpu_configs[#].pd_pipe_width: Pre-Decode Pipe Width (instructions) determines the number of instructions whose lengths can be determined in a cycle. In reference to stage present in Intel processors.
 - cpu_configs[#].d_pipe_width: Decode Pipe Width (instructions) determines the number of instructions that are decoded per cycle.
 - cpu_configs[#].f_buffer_size: depth of fetch buffer that lies between predict and fetch stages. size of each buffer entry is \<pf_pipe_width\> bytes.
 - cpu_configs[#].
