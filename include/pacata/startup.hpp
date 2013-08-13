
#pragma once

#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace pacata {
template <class SessionOp> class async_accept_op{
public:
    async_accept_op(boost::asio::io_service & io_service, boost::asio::ip::tcp::acceptor & acceptor)
      : m_io_service(io_service)
      , m_acceptor(acceptor)
    {
        boost::shared_ptr<boost::asio::ip::tcp::socket> clientsocket
            = boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(m_io_service));
        m_acceptor.async_accept(*clientsocket,  boost::bind<void>(*this, _1, clientsocket));
    }

    void operator()(boost::system::error_code ec, boost::shared_ptr<boost::asio::ip::tcp::socket> clientsocket)
    {
        if (!ec)
            {SessionOp(m_io_service, clientsocket);}
        {async_accept_op<SessionOp>(m_io_service, m_acceptor);}
    }

private:
    boost::asio::io_service & m_io_service;
    boost::asio::ip::tcp::acceptor & m_acceptor;

};

template <class SessionOp> void startup_httpd(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor)
{
    {async_accept_op<SessionOp> op(io_service, acceptor);}
}

}
