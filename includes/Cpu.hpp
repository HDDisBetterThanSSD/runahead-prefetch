
class Cpu: public Driver
{
public:
	Cpu(const Config &config, int cpu_num);
	res_t step();
	res_t add_l1i_mmu(Mmu *l1i_mmu);
	res_t add_l1d_mmu(Mmu *l1d_mmu);
	void print_results(void) { }
protected:
	unsigned long retire_ins_count;
	Trace *trace;
	Mmu *l1i_mmu;
	Mmu *l1d_mmu;
	std::list<std::vector<Ins *> >fe_pipe;
	int fe_pipe_depth;
	int fe_pipe_width;
	std::vector<Ins *> fetch_block;
	std::list<Ins *> ls_buffer;

	int ls_buffer_size;
	int ld_buffer_size;
	int st_buffer_size;
};