

class Mem_Interface
{
public:
	Mem_Interface(const Config &config);
	virtual res_t sub_send_request(Request *request) = 0;
	virtual Response *sub_get_response(void) = 0;
	virtual res_t super_send_request(Request *request) = 0;
	virtual Response *super_get_response(void) = 0;
	void add_sub_interface(Mem_Interface *sub_interface);
	void add_super_interface(Mem_Interface *super_interface);
	virtual res_t step(void) = 0;
	virtual void print_results(void) = 0;
protected:
	const Config &config;
	Mem_Interface *sub_interface;
	Mem_Interface *super_interface;
	std::list<Request *>super_in_request_buffer;
	std::map<unsigned long, Response *>super_in_response_buffer;
	std::unordered_map<Request *, unsigned long>sub_out_request_buffer;
	std::list<Request *>sub_in_request_buffer;
	std::map<unsigned long, Response *>sub_in_response_buffer;
	std::unordered_map<Request *, unsigned long>super_out_request_buffer;
};