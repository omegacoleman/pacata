Pacata
=======

Pacata is an header only library that helps you develop http-based server programs.

## Pacata 是什么
Pacata 是avhttpd项目的一个fork，是一个 header only 的 C++ 库，基于 Boost ，帮助大家轻易编写高效的基于 HTTP 协议的服务器程序。

## Pacata 不是什么
Pacata 并不是一个完整的 HTTP 协议库，只是帮助大家编写简单的 HTTP 协议程序，以配合 nginx 这样的前端工作。
利用 nginx 的 proxy_pass 功能，相对 fastcgi 模式要更加灵活。必要的时候还可以让 Pacata 编写的程序直接面对用户处理访问。

Pacata 并不打算支持 gzip/chunked ， SSL 之类的高级功能， 因为这些都应该是由 nginx 这样的反向代理服务器用 proxy-pass 完成的工作。

## Pacata 设计

Pacata 底层以 stack-less 协程技术编写而成。

Pacata 鼓励使用模版技术，直接将网站逻辑嵌入会话模版类之中。借助 C++ 编译器的强大优化功能，带来难以想像的性能提升。

使用 Pacata 并不会绑架你的库。因为这些功能并不需要你继承任何对象，他只是一组 asio 复合回调函数。

## Example

下面就是一个例子：

```c++

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
```

可见，Pacata不但提供了广博的自由度，而且十分便利，易于使用。

