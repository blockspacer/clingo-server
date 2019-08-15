//--------------------------------------------------------------------------------
// Provide async message passing.
// -------------------------------------------------------------------------------

#ifndef CLSERVER_CONNECTION_HH
#define CLSERVER_CONNECTION_HH

#include <boost/asio.hpp>
#include <deque>

namespace clserver
{

namespace asio=boost::asio;
namespace bsys=boost::system;
namespace sp=std::placeholders;

//-------------------------------------------------------------------------------
// Connection class provides a general way of sending and receiving asynchronous
// messages on a stream/socket. Messages are framed by first sending a 32-bit message
// size block followed by the message of exactly that size.
//
// At the start of a connection both sides of the connection must both send and
// receive a specific string to establishes a legitimate connection.
// -------------------------------------------------------------------------------

template<typename Stream>
class Connection
{

public:
    Connection(Stream stream, const std::string& validate_id);

    Connection(Connection&&) = delete;               // non movable
    Connection(const Connection&) = delete;          // non copy-constructible
    ~Connection();

    Connection& operator=(const Connection&) = delete; // non copyable

    // Start a connect send/receive sequence to validate the connection
    template<typename Handler> void validate(Handler h);

    template<typename Handler>
    void async_receive_message(asio::streambuf& sb, Handler h);

    template<typename Handler>
    void async_send_message(const asio::streambuf& sb, Handler h);

//    Stream &stream();
private:
    //---------------------------------------------------------------------------
    // Definitions
    //---------------------------------------------------------------------------

    using rw_handler_t = std::function<void(const bsys::error_code&, std::size_t)>;
    using validate_handler_t = std::function<void(const bsys::error_code&)>;

    //---------------------------------------------------------------------------
    // Internal member functions
    //---------------------------------------------------------------------------

    // Check if there is a queued read/write request and if so handle the queue front.
    void _check_rqueue();
    void _check_wqueue();

    // Callbacks for when validate messages are sent and received
    void _on_validate_sent(const bsys::error_code& ec, std::size_t s);
    void _on_validate_received(const bsys::error_code& ec, std::size_t s);

    // When there is an error we need to clear the queues and propagate the error
    void _receive_error(const bsys::error_code& ec, std::size_t s);
    void _send_error(const bsys::error_code& ec, std::size_t s);

    // Internal read/write handlers
    void _on_receive_message_size(const bsys::error_code& ec, std::size_t s);
    void _on_receive_message_body(const bsys::error_code& ec, std::size_t s);
    void _on_send_message_size(const bsys::error_code& ec, std::size_t s);
    void _on_send_message_body(const bsys::error_code& ec, std::size_t s);


    //---------------------------------------------------------------------------
    // Inner classes
    //---------------------------------------------------------------------------

    struct _ReadReq
    {
        asio::streambuf& streambuf_;
        rw_handler_t handler_;

        template<typename Handler>
        _ReadReq(asio::streambuf& sb, Handler h) :
            streambuf_{sb}, handler_{h} {}
    };

    struct _WriteReq
    {
        const asio::streambuf& streambuf_;
        rw_handler_t handler_;

        template<typename Handler>
        _WriteReq(const asio::streambuf& sb, Handler h) :
            streambuf_{sb}, handler_{h} {}
    };

    //-------------------------------------------------------------------------------
    // Note: wrapping the stream allows us to explicitly destroy the stream
    // before the Connection object itself. This in turn causes any current
    // read/write operations to fail which will then call the error handler and
    // cause the queue to be cleaned up properly.
    // -------------------------------------------------------------------------------
    struct _StreamWrapper
    {
        Stream stream_;
        _StreamWrapper(Stream stream) : stream_{std::move(stream)} {}
    };

    //---------------------------------------------------------------------------
    // Internal member variable
    //---------------------------------------------------------------------------
    std::unique_ptr<_StreamWrapper> sw_;

    // The connection validation identifier that establishes a valid connection
    std::string validate_id_;
    asio::streambuf validate_sb_;
    validate_handler_t validate_handler_;
    bool validated_;

    // Size of the current read an write messages
    uint32_t rsize_;
    uint32_t wsize_;

    // Are the read and write queues currently active
    bool ractive_;
    bool wactive_;

    // Read and write queues - items pushed onto the back and popped from the front
    std::deque<_ReadReq> rqueue_;
    std::deque<_WriteReq> wqueue_;
};

//-------------------------------------------------------------------------------
// Connection public member functions
//-------------------------------------------------------------------------------

template<typename Stream>
Connection<Stream>::Connection(Stream stream,
                               const std::string& validate_id) :
    sw_{std::make_unique<_StreamWrapper>(std::move(stream))},
    validate_id_{validate_id}, validated_{false},
    rsize_{0}, wsize_{0}, ractive_{false}, wactive_{false}
{ }

template<typename Stream>
Connection<Stream>::~Connection()
{
    sw_.reset(nullptr);
}

//---------------------------------------------------------------------------
// Start the validation process by first writing the validate_id_ then trying to
// read it.
// ---------------------------------------------------------------------------

template<typename Stream>
template<typename Handler>
void Connection<Stream>::validate(Handler h)
{
    if (validated_)
    {
        h(bsys::errc::make_error_code(bsys::errc::already_connected));
        return;
    }

    // Perform async write of validate_id
    validate_handler_ = h;
    asio::async_write(sw_->stream_, asio::buffer(validate_id_, validate_id_.length()),
                      std::bind(&Connection<Stream>::_on_validate_sent,
                                this, sp::_1, sp::_2));
}

//---------------------------------------------------------------------------
// Async read/write functions.
// ---------------------------------------------------------------------------

template<typename Stream>
template<typename Handler>
void Connection<Stream>::async_receive_message(asio::streambuf& sb, Handler h)
{
    rqueue_.emplace_back(sb,h);
    _check_rqueue();
}

template<typename Stream>
template<typename Handler>
void Connection<Stream>::async_send_message(const asio::streambuf& sb, Handler h)
{
    wqueue_.emplace_back(sb,h);
    _check_wqueue();
}

/*
template<typename Stream>
Stream &Connection<Stream>::stream()
{
    return sw_->stream_;
}
*/

//------------------------------------------------------------------------------
// Connection internal member functions
// ------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// If the queue is not empty and is not currently active then pop from the front
// and set up the send/receive.
// -----------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_check_rqueue()
{
    if (!validated_ || ractive_ || rqueue_.empty()) return;

    // Perform async read for a message size frame
    ractive_ = true;
    asio::async_read(sw_->stream_, asio::buffer(&rsize_, 4),
                     std::bind(&Connection<Stream>::_on_receive_message_size,
                               this, sp::_1, sp::_2));
}

template<typename Stream>
void Connection<Stream>::_check_wqueue()
{
    if (!validated_ || wactive_ || wqueue_.empty()) return;

    wactive_ = true;
    wsize_ = htonl(wqueue_.front().streambuf_.size());

    // Perform async write for a message size frame
    asio::async_write(sw_->stream_, asio::buffer(&wsize_, 4),
                     std::bind(&Connection<Stream>::_on_send_message_size,
                               this, sp::_1, sp::_2));
}


//---------------------------------------------------------------------------
// When the validate message has been sent successfully start the read.
//---------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_on_validate_sent(const bsys::error_code& ec, std::size_t s)
{
    if (ec) { validate_handler_(ec); return; }

    // Set up to read exactly the connection string
    auto len = validate_id_.length();
    auto mbt = validate_sb_.prepare(len);
    asio::async_read(sw_->stream_, mbt, asio::transfer_exactly(len),
                     std::bind(&Connection<Stream>::_on_validate_received,
                               this, sp::_1, sp::_2));
}

//---------------------------------------------------------------------------
// When the validate message has been received
//---------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_on_validate_received(const bsys::error_code& ec, std::size_t s)
{
    if (ec) { validate_handler_(ec); return; }

    validate_sb_.commit(s);
    auto cbt = validate_sb_.data();
    auto size = validate_sb_.size();
    auto buf_start = asio::buffers_begin(cbt);
    auto buf_end = asio::buffers_begin(cbt) + size;

    std::string received_str(buf_start, buf_end);

    if (received_str != validate_id_)
    {
        validate_handler_(bsys::errc::make_error_code(bsys::errc::bad_message));
        return;
    }

//    std::cerr << "=========== CONNECTION IS VALID ===========" <<std::endl;

    // Validated and check the read/write queues
    validate_handler_(ec);
    validated_ = true;
    _check_rqueue();
    _check_wqueue();
}


//---------------------------------------------------------------------------
// On a message read error
//---------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_receive_error(const bsys::error_code& ec, std::size_t s)
{
//    std::cerr << "---- Stream read error: " << ec.value() << std::endl;
    while (!rqueue_.empty())
    {
        rqueue_.front().handler_(ec,s);
        rqueue_.pop_front();
    }
}

//---------------------------------------------------------------------------
// On a message write error
//---------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_send_error(const bsys::error_code& ec, std::size_t s)
{
//    std::cerr << "---- Stream write error: " << ec.value() << std::endl;
    while (!wqueue_.empty())
    {
        wqueue_.front().handler_(ec,s);
        wqueue_.pop_front();
    }
}

//---------------------------------------------------------------------------
// Internal read/write handlers
//---------------------------------------------------------------------------

template<typename Stream>
void Connection<Stream>::_on_receive_message_size(const bsys::error_code& ec,
                                               std::size_t s)
{
    // On error clear the queue and call all the handler queued handlers.
    if (ec) { _receive_error(ec,s); return; }

    // Convert size data into host-endian format and read exactly that number
    rsize_ = ntohl(rsize_);
    auto& sb = rqueue_.front().streambuf_;
    auto mbt = sb.prepare(rsize_);
    asio::async_read(sw_->stream_, mbt,  asio::transfer_exactly(rsize_),
                     std::bind(&Connection<Stream>::_on_receive_message_body,
                               this, sp::_1, sp::_2));
}


template<typename Stream>
void Connection<Stream>::_on_receive_message_body(const bsys::error_code& ec,
                                               std::size_t s)
{
    // On error clear the queue and call all the handler queued handlers.
    if (ec) { _receive_error(ec,s); return; }

    // Clean up and start the next async read if necessary
    rqueue_.front().handler_(ec,s);
    rqueue_.pop_front();
    ractive_ = false;
    _check_rqueue();            // check if we have more reads
}

template<typename Stream>
void Connection<Stream>::_on_send_message_size(const bsys::error_code& ec,
                                                std::size_t s)
{
    // On error clear the queue and call all the handler queued handlers.
    if (ec) { _send_error(ec,s); return; }

    // Write the message body
    auto& sb = wqueue_.front().streambuf_;
    asio::async_write(sw_->stream_, sb.data(),
                      std::bind(&Connection<Stream>::_on_send_message_body,
                                this, sp::_1, sp::_2));
}

template<typename Stream>
void Connection<Stream>::_on_send_message_body(const bsys::error_code& ec,
                                                std::size_t s)
{
    // On error clear the queue and call all the handler queued handlers.
    if (ec) { _send_error(ec,s); return; }

    // Clean up and start the next async write if necessary
    wqueue_.front().handler_(ec,s);
    wqueue_.pop_front();
    wactive_ = false;
    _check_wqueue();          // check if we have more writes
}



}

#endif // CLSERVER_CONNECTION_HH
