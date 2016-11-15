
class Mem_Hierarchy
{
public:
	Mem_Hierarchy(const Config &config);
	Mmu *get_cpu_mmu(mmu_type_t mmu_type, int cpu_num);
	Cache_L1i_Standalone *get_cpu_l1i();
	res_t step(void);
	void print_results(void);
private:
	const Config &config;
	std::map<mmu_type_t, std::vector<Mmu *> > mmus;
	std::map<std::string, std::vector<Mem_Interface *> > caches;
	Cache_L1i_Standalone *cache_l1i_standalone;
	Memory *memory;
};