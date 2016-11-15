typedef enum request_type {
	FETCH,
	RFO,
	DEMAND_DATA,
	WRITE,
	INVALIDATE
} request_type_t;

class Request
{
public:
	Request(const Config &config, std::string req_obj, uint64_t da);

	static Request *NewRequest(const Config& config, std::string req_obj, uint64_t da);
	static void FreeRequest(Request *request);

	uint64_t da;
	std::string req_obj;
	request_type_t request_type;
	uint64_t request_size;
private:
	static std::vector<Request *> free_request_list;

	const Config &config;
};