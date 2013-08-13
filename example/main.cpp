
#include <boost/asio.hpp>
#include <pacata.hpp>

class Example
{
public:
	Example(){};
	std::string operator()(boost::shared_ptr<pacata::session_context> context, pacata::response_opts& opts)
	{
		if (context->m_request_opts->find(pacata::http_options::request_uri) == "/hello")
		{
			return "Hello!";
		}
		return "Wow!";
	}
};

class ExampleRouteMap
{
public:
	ExampleRouteMap(){};
	void operator()(boost::asio::io_service & io_service, boost::shared_ptr<pacata::session_context> context)
	{
		if (context->m_request_opts->find(pacata::http_options::request_uri) == "/dummy")
		{
			pacata::session_dummy s(io_service, context);
		} else if (context->m_request_opts->find(pacata::http_options::request_uri) == "/wrong")
		{
			// Cause an error.
            char* myarray= new char[0x7fffffff];
		} else {
			pacata::session_once<Example> s(io_service, context);
		}
	}
};

int main(int argc, char **argv)
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::acceptor acceptor(io_service,
		boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 4000)
	);
	pacata::startup_httpd<pacata::session_route<ExampleRouteMap> >(io_service, acceptor);
	try
	{
		io_service.run();
	} catch (std::exception e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
    return 0;
}
