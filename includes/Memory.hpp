
class Memory: public Mem_Interface
{
public:
	Memory(const Config &config);
	res_t sub_send_request(Request *request);
	Response *sub_get_response(void);
	res_t super_send_request(Request *request);
	Response *super_get_response(void);
	res_t step(void);
	void print_results(void);
private:
	unsigned long bus_last_use_step;
};