#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/_experimental/test/stream.hpp>
#include "test_schema_generated.h"
#include "clserver/connection.hpp"

// The flatbuffers namespaces
namespace fbs=flatbuffers;
namespace ct=CommsTest;
namespace bsys=boost::system;
namespace bbtest=boost::beast::test;
namespace asio=boost::asio;
namespace sp=std::placeholders;

using boost::asio::ip::tcp;
using namespace clserver;

//------------------------------------------------------------------------------
// Function to setup a ping (read/write) loop
//------------------------------------------------------------------------------

template<typename Stream>
struct PingLoop
{
    Connection<Stream>& conn_;
    asio::streambuf sb_;
    std::string msg_;

    PingLoop(Connection<Stream>& conn) : conn_{conn}{ }

    void start()
    {
        conn_.async_receive_message(sb_,std::bind(&PingLoop<Stream>::on_received,
                                                  this, sp::_1, sp::_2));
    }

    void on_received(const bsys::error_code& ec, std::size_t s)
    {
        if (ec) { std::cerr << ec.message() << std::endl; return; }
        sb_.commit(s);
        auto cbt = sb_.data();
        std::string msg{asio::buffers_begin(cbt), asio::buffers_end(cbt)};
        std::cerr << "Received message: " << msg << std::endl;
        if (msg == "exit"){ std::cerr << "Exiting" << std::endl; return; }
        conn_.async_send_message(sb_,
                                 std::bind(&PingLoop<Stream>::on_sent,
                                           this, sp::_1, sp::_2));
    }

    void on_sent(const bsys::error_code& ec, std::size_t s)
    {
        if (ec) { std::cerr << ec.message() << std::endl; return; }
        sb_.consume(s);
        start();
    }
};


//------------------------------------------------------------------------------
// Test cases
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	try
	{
        asio::io_context ioc;
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), 5666)};
        tcp::socket socket{ioc};
        acceptor.accept(socket);

        std::cerr << "Accepted socket" << std::endl;

        Connection<tcp::socket> conn{std::move(socket), "clingoserver"};
        PingLoop<tcp::socket> pl{conn};
        conn.validate(
            [&pl](const bsys::error_code& ec)
            {
                if (ec)
                {
                    std::cerr << "Validation error: " << ec.message() << std::endl;
                    return;
                }
                pl.start();
            });

        ioc.run();
    }
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}
	return 0;
}

