
class Simulate
{
public:
	Simulate(const Config &config);
	void add_driver(Driver *driver);
	void set_mem_hierarchy(Mem_Hierarchy *mem_hierarchy);
	void run(void);
	void print_results(void);
private:
	const Config &config;
	std::vector<Driver *> drivers;
	Mem_Hierarchy *mem_hierarchy;
};