
#pragma once

#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "read_request.hpp"
#include "write_reponse.hpp"
#include <iostream>
#include <exception>

namespace pacata {
	
class session_dummy{
public:
    session_dummy(boost::asio::io_service & io_service, boost::shared_ptr<boost::asio::ip::tcp::socket> clientsocket)
        : io_service(io_service), context(boost::make_shared<session_context>(session_context(io_service, clientsocket)))
        , coro(0)
    {
        async_read_request_context(*(this->context), *this);
    }

    session_dummy(boost::asio::io_service & io_service, boost::shared_ptr<session_context> context)
        : io_service(io_service), context(context)
        , coro(0)
    {
		io_service.dispatch(*this);
    }
	
    void operator()(boost::system::error_code ec = boost::system::error_code(), std::size_t bytes_transfered = 0)
    {
        if (coro++ == 0)
        {
            pacata::response_opts opts(*(this->context->m_request_opts));
            async_write_response_context(*(this->context), 200, opts, *this);
        }
    }
private:
    boost::shared_ptr<session_context> context;
    int coro;
	boost::asio::io_service &io_service;
};

class norm_session_error{
public:
	norm_session_error(boost::asio::io_service & io_service, boost::shared_ptr<session_context> context, std::exception& e)
        : io_service(io_service), context(context)
        , coro(0), e(e)
    {
		io_service.dispatch(*this);
    }
	
    void operator()(boost::system::error_code ec = boost::system::error_code(), std::size_t bytes_transfered = 0)
    {
        if (coro++ == 0)
        {
            pacata::response_opts opts(*(this->context->m_request_opts));
			this->s = boost::make_shared<std::string>(std::string("Internal Server Error : ") + this->e.what());
			std::cerr << this->s << std::endl;
            async_write_response_context(*(this->context), 500, opts, boost::asio::buffer(this->s->data(), this->s->length()), *this);
        }
    }
private:
    boost::shared_ptr<session_context> context;
    int coro;
	boost::asio::io_service &io_service;
    boost::shared_ptr<std::string> s;
	std::exception e;
};

// OnceOp must meet:
// std::string OnceOp(boost::shared_ptr<pacata::session_context> context, pacata::response_opts& opts)
template <class OnceOp, class ErrorSession=norm_session_error>
class session_once{
public:
    session_once(boost::asio::io_service & io_service, boost::shared_ptr<boost::asio::ip::tcp::socket> clientsocket)
        : io_service(io_service), context(boost::make_shared<session_context>(session_context(io_service, clientsocket)))
        , op(), coro(0)
    {
        async_read_request_context(*(this->context), *this);
    }
    session_once(boost::asio::io_service & io_service, boost::shared_ptr<session_context> context)
        : io_service(io_service), context(context)
        , op(), coro(0)
    {
		io_service.dispatch(*this);
    }

    void operator()(boost::system::error_code ec = boost::system::error_code(), std::size_t bytes_transfered = 0)
    {
        if (coro++ == 0)
        {
            pacata::response_opts opts(*(this->context->m_request_opts));
			try
			{
				this->s = boost::make_shared<std::string>(this->op(this->context, opts));
				async_write_response_context(*(this->context), 200, opts, boost::asio::buffer(this->s->data(), this->s->length()), *this);
			} catch (std::exception& e)
			{
				ErrorSession(this->io_service, this->context, e);
			}
        }
    }
private:
    boost::shared_ptr<session_context> context;
    OnceOp op;
    int coro;
	boost::asio::io_service &io_service;
    boost::shared_ptr<std::string> s;
};


template <class RouteMap, class ErrorSession=norm_session_error>
class session_route{
public:
    session_route(boost::asio::io_service & io_service, boost::shared_ptr<boost::asio::ip::tcp::socket> clientsocket)
        : io_service(io_service), context(boost::make_shared<session_context>(session_context(io_service, clientsocket)))
        , map()
    {
        async_read_request_context(*(this->context), *this);
    }

    session_route(boost::asio::io_service & io_service, boost::shared_ptr<session_context> context)
        : io_service(io_service), context(context)
        , map()
    {
		io_service.dispatch(*this);
    }
	
    void operator()(boost::system::error_code ec = boost::system::error_code(), std::size_t bytes_transfered = 0)
    {
		try
		{
			this->map(this->io_service, this->context);
		} catch (std::exception& e)
		{
			ErrorSession(this->io_service, this->context, e);
		}
    }
private:
    boost::shared_ptr<session_context> context;
    RouteMap map;
	boost::asio::io_service &io_service;
};

}
