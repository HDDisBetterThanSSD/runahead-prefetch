
class Driver
{
public:
	Driver(const Config &config, int cpu_num): config(config), cpu_num(cpu_num) { }
	virtual res_t step() = 0;
	unsigned long get_retire_ins_count(void) {return retire_ins_count;}
	virtual void print_results(void) = 0;
protected:
	const Config &config;
	int cpu_num;
	unsigned long retire_ins_count;
};