
class Cpu_Prefetch_InsOnly: public Driver
{
public:
	Cpu_Prefetch_InsOnly(const Config &config, int cpu_num);
	res_t step();
	res_t add_l1i(Cache_L1i_Standalone *l1i);
	
	void print_results(void);
private:
	
	unsigned long total_stalls;
	unsigned long ifetch_stalls;
	Trace *trace;
	std::list<std::vector<Ins *> >fe_pipe;
	int fe_pipe_depth;
	int fe_pipe_width;
	std::vector<Ins *> fetch_block;
	std::list<Ins *> ls_buffer;

	int ls_buffer_size;
	int ld_buffer_size;
	int st_buffer_size;

	Cache_L1i_Standalone *l1i;
	Br_Predictor *br_predictor;
	bool last_misp;
	int pf_pipe_width;
	std::vector<Ins *> prefetch_block;
	std::list<std::list<Ins *> > fetch_buffer;
	int f_buffer_size;
	uint64_t cl_mask;
	uint64_t prev_prefetch_cl;
	uint64_t prev_fetch_cb;
	int fetch_per_block;
};