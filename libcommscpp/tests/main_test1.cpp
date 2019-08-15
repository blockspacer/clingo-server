#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <boost/asio.hpp>
#include <boost/beast/_experimental/test/stream.hpp>
#include <iostream>
#include "test_schema_generated.h"
#include "clserver/connection.hpp"

// The flatbuffers namespaces
namespace fbs=flatbuffers;
namespace ct=CommsTest;
namespace bsys=boost::system;
namespace bbtest=boost::beast::test;

using namespace clserver;

//------------------------------------------------------------------------------
// Helper function to create a test message
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Test cases
//------------------------------------------------------------------------------


TEST_CASE("validate_connection")
{
    asio::io_context ioc;
    bbtest::fail_count fc{0};
//    bbtest::stream s1{ioc, fc, "clingoserver\n\n"};
//    bbtest::stream s1{ioc, "clingoserver\n\n"};
//    bbtest::stream s2{ioc, "clingoserver\n\n"};
    bbtest::stream s1{ioc};
    bbtest::stream s2{ioc};
    s1.connect(s2);

    // Create a flatbuffer message and test that it worked
    std::string msgstr1 = "test1";
    std::string msgstr2 = "A much longer text message that";

    // Create the mock connection
    bsys::error_code validated_ec1;
    bsys::error_code validated_ec2;

    Connection<bbtest::stream> conn1{std::move(s1), "clingoserver"};
    Connection<bbtest::stream> conn2{std::move(s2), "clingoserver"};
    conn1.validate(
        [&validated_ec1](const bsys::error_code& e){ validated_ec1 = e; });
    conn2.validate(
        [&validated_ec2](const bsys::error_code& e){ validated_ec2 = e; });

    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    ioc.poll_one();
    std::cerr << validated_ec1.category().name() << ":"
              << validated_ec1.message() << ":"
              << validated_ec1.value() << std::endl;
    REQUIRE(validated_ec1.value() == 0);
}
