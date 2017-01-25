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
- cpu_configs[#]: provides options for configuring the CPU, "#" is the cpu number (here only 1 cpu is instantiated, but the number is still needed.)
 - cpu_configs[#].pf_pipe_width: Predict (Prefetch) Pipe Width (bytes) determines how many bytes are predicted by the branch predictor per cycle
 - cpu_configs[#].fe_pipe_width: Front-end Pipe Width (bytes) determines the number of bytes that are fetched per cycle
 - cpu_configs[#].pd_pipe_width: Pre-Decode Pipe Width (instructions) determines the number of instructions whose lengths can be determined in a cycle. In reference to stage present in Intel processors.
 - cpu_configs[#].d_pipe_width: Decode Pipe Width (instructions) determines the number of instructions that are decoded per cycle.
 - cpu_configs[#].f_buffer_size: depth of fetch buffer that lies between predict and fetch stages. size of each buffer entry is \<pf_pipe_width\> bytes.
 - cpu_configs[#].iq_buffer_size: depth of instruction queue buffer that lies between the predecode and decode stages. size of each buffer entry is variable but up to \<pd_pipe_width\> instructions or \<fe_pipe_width\> bytes.
- l1i_configs[#]: provides options for configuring the L1i cache, "#" is the cpu number (here only 1 cpu is instantiated, but the number is still needed)
 - l1i_configs[#].sets
 - l1i_configs[#].ways
 - l1i_configs[#].cl_size: cacheline size (bytes)
 - l1i_configs[#].replacement_policy: only least recently used (LRU) is implemented
 - l1i_configs[#].hit_latency: number of cycles for a request to return on a hit
 - l1i_configs[#].hit_request_bw: number of requests that can be serviced in a cycle. more important when predict and fetch are sending requests
 - l1i_configs[#].miss_latency: number of cycles for a request to return on a miss (assuming a uniform miss latency, i.e. L2 hit)
 - l1i_configs[#].miss_byte_bw: number of bytes that can be returned to the L1i cache from the L2 cache per cycle.
 - l1i_configs[#].addr_bits: size of addresses in bits. to ensure unique tags within the appropriate size constraints.
 - l1i_configs[#].in_request_buffer_size: number of pending requests that can be held by L1i cache before back-pressuring occurs.
 - l1i_configs[#].in_response_buffer_size: number of completed requests that can be held by the L1i cache awaiting the CPU to accepts before back-pressuring occurs.
- prefetch_config: provides options for configuring the prefetcher.
 - prefetch_config.prefetch: prefetching active (true/false)
 - prefetch_config.prefetch_type: type of prefetching
  - N1L: automatically prefetch the next cache line
  - N2L: automatically prefetch the next two cache lines
  - BR_PREDICTOR: prefetch the path determined by the branch predictor
 - br_predictor_config
