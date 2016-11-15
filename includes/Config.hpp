//#include <ifstream>

class Config
{
public:
	Config();
	void parse_config(std::ifstream &config_file);
	void parse_args(int argc, char *argv[]);
	void print_config(void);

	// configurations
	bool l1i_standalone;
	std::vector<std::string> trace_files;
	std::string outfile;
	std::string logfile;
	std::vector<struct cpu_config> cpu_configs;
	struct mem_hier_config mem_hier_config;
	std::vector<struct cache_config> l1i_configs;
	std::vector<struct cache_config> l1d_configs;
	std::vector<struct cache_config> l2_configs;
	std::vector<struct cache_config> l3_configs;
	struct mem_config mem_config;
	struct tlb_config tlb_config;
	struct page_table_config page_table_config;
	struct br_predictor_config br_predictor_config;
	struct prefetch_config prefetch_config;
	int debug_level;

private:
	res_t update_config(const std::string &config, const std::string &value);
};