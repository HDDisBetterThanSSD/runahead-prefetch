

class Cache_L1i_Standalone: public Cache
{
public:
	Cache_L1i_Standalone(const Config &config, struct cache_config cache_config, cache_type_t cache_type);
	res_t step(void);
	res_t sub_send_request(Request *request);
	Response *sub_get_response(std::string req_obj);
	void add_miss_latency(int miss_latency);
	void print_results(void);
	bool L2_accept(void);
private:
	int miss_latency;
	std::map<unsigned long, std::list<Request *> > super_in_miss_buffer;
	std::map<std::string, std::list<Request *> > super_in_request_buffer;
	std::unordered_map<uint64_t, unsigned long> super_in_request_completion;

	//std::unordered_map<std::string, std::list<Response *> > super_in_response_buffer;
	std::unordered_map<std::string, std::list<Response *> > super_in_response_buffer;

	std::map<std::string, unsigned long> total_misses;
	std::map<std::string, unsigned long> total_requests;

	unsigned long last_miss_cycle;
	unsigned int last_miss_bytes;
};