#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/beast/_experimental/test/stream.hpp>
#include "test_schema_generated.h"
#include "clserver/connection.hpp"

// The flatbuffers namespaces
namespace fbs=flatbuffers;
namespace ct=CommsTest;
namespace bsys=boost::system;
namespace bbtest=boost::beast::test;
namespace asio=boost::asio;


using boost::asio::ip::tcp;
using namespace clserver;

//------------------------------------------------------------------------------
// Helper function to create a test message
//------------------------------------------------------------------------------

template<typename Stream>
struct TestSendReceive
{
    Connection<Stream>& conn_;
    asio::streambuf sbsend_;
    asio::streambuf sbreceive_;
    unsigned int i_;

    TestSendReceive(Connection<Stream>& conn) : conn_{conn}, i_{0}{ }

    void start()
    {
        std::ostream os(&sbsend_);
        if (i_ < 10) os << "test" << i_;
        else if (i_ == 10) os << "exit";
        else return;

        auto cbt = sbsend_.data();
        std::string msg{asio::buffers_begin(cbt), asio::buffers_end(cbt)};
        std::cerr << "Sending message: " <<  msg << std::endl;
        ++i_;
        conn_.async_send_message(sbsend_,
                                 std::bind(&TestSendReceive<Stream>::on_sent,
                                           this, sp::_1, sp::_2));
    }

    void on_sent(const bsys::error_code& ec, std::size_t s)
    {
        if (ec) { std::cerr << ec.message() << std::endl; return; }
        sbsend_.consume(s);
        if (i_ > 10) return;
        conn_.async_receive_message(sbreceive_,
                                    std::bind(&TestSendReceive<Stream>::on_received,
                                              this, sp::_1, sp::_2));
    }

    void on_received(const bsys::error_code& ec, std::size_t s)
    {
        if (ec) { std::cerr << ec.message() << std::endl; return; }
        sbreceive_.consume(sbreceive_.size());
        sbreceive_.commit(s);
        auto cbt = sbreceive_.data();
        std::string msg{asio::buffers_begin(cbt), asio::buffers_end(cbt)};
        std::cerr << "Received message: " << msg << std::endl;
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
        tcp::socket socket{ioc};

        tcp::resolver resolver(ioc);
        std::string service = boost::lexical_cast<std::string>(5666);
        tcp::resolver::query query("127.0.0.1", service);
        asio::connect(socket, resolver.resolve(query));

        std::cerr << "Accepted socket" << std::endl;

        Connection<tcp::socket> conn{std::move(socket), "clingoserver"};
        TestSendReceive<tcp::socket> tsr{conn};
        conn.validate(
            [&tsr](const bsys::error_code& ec)
            {
                if (ec)
                {
                    std::cerr << "Validation error: " << ec.message() << std::endl;
                    return;
                }
                tsr.start();
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

