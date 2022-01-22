#include "websocket-client.h"

#include <cstdlib>
#include <cstring>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "openssl/ssl3.h"

static void fail(boost::beast::error_code ec, const char *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

WebSocketClient::session::session(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx)
    : resolver_(boost::asio::make_strand(ioc))
    , ws_(boost::asio::make_strand(ioc), ctx)
{
}

void WebSocketClient::session::run(const std::string &host, const std::string &port)
{
    // Save these for later
    host_ = host;

    // Look up the domain name
    resolver_.async_resolve(
                host,
                port,
                boost::beast::bind_front_handler(
                    &session::on_resolve,
                    shared_from_this()));
}

void WebSocketClient::session::Send(const std::string &message)
{
    // Send the message
    ws_.async_write(
                boost::asio::buffer(message),
                boost::beast::bind_front_handler(
                    &session::on_write,
                    shared_from_this()));
}

void WebSocketClient::session::on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
{
    if(ec)
        return fail(ec, "resolve");

    // Set a timeout on the operation
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    boost::beast::get_lowest_layer(ws_).async_connect(
                results,
                boost::beast::bind_front_handler(
                    &session::on_connect,
                    shared_from_this()));
}

void WebSocketClient::session::on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail(ec, "connect");

    // Set a timeout on the operation
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(
                ws_.next_layer().native_handle(),
                host_.c_str()))
    {
        ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()),
                                      boost::asio::error::get_ssl_category());
        return fail(ec, "connect");
    }

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(
                    &session::on_ssl_handshake,
                    shared_from_this()));
}

void WebSocketClient::session::on_ssl_handshake(boost::beast::error_code ec)
{
    if(ec)
        return fail(ec, "ssl_handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    boost::beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    boost::beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(boost::beast::websocket::stream_base::decorator(
                       [](boost::beast::websocket::request_type& req)
                   {
                       req.set(boost::beast::http::field::user_agent,
                       std::string(BOOST_BEAST_VERSION_STRING) +
                       " websocket-client-async-ssl");
                   }));

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/",
                        boost::beast::bind_front_handler(
                            &session::on_handshake,
                            shared_from_this()));
}

void WebSocketClient::session::on_handshake(boost::beast::error_code ec)
{
    if(ec)
        return fail(ec, "handshake");

    // Read a message into our buffer
    ws_.async_read(
                buffer_,
                boost::beast::bind_front_handler(
                    &session::on_read,
                    shared_from_this()));
}

void WebSocketClient::session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");
}

void WebSocketClient::session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // The make_printable() function helps print a ConstBufferSequence
    std::cout << boost::beast::make_printable(buffer_.data()) << std::endl;
    buffer_.clear();

    ws_.async_read(
                buffer_,
                boost::beast::bind_front_handler(
                    &session::on_read,
                    shared_from_this()));

    if(ec)
        return fail(ec, "read");
}

void WebSocketClient::session::on_close(boost::beast::error_code ec)
{
    if(ec)
        return fail(ec, "close");

    // If we get here then the connection is closed gracefully


}
