
typedef enum response_type {
	DO_NOTHING,
	FILL,
	WB
} response_type_t;

class Response
{
public:
	Response(const Config &config, Request *request);
	~Response(void);

	static Response *NewResponse(const Config &config, Request *request);
	static void FreeResponse(Response *response);

	Request *request;
	res_t response_res;
	response_type_t response_type;
	mesi_t mesi_bits;
	uint64_t response_time;
private:
	static std::vector<Response *>free_response_list;

	const Config &config;
};