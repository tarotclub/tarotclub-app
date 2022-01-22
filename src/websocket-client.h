#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

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

class WebSocketClient
{
public:
    std::string Run(const std::string &host, const std::string &port)
    {
        std::string response;
        try
        {
            // The io_context is required for all I/O
            boost::asio::io_context ioc;

            // The SSL context is required, and holds certificates
            boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

            // Verify the remote server's certificate
            ctx.set_verify_mode(boost::asio::ssl::verify_none);

            // Launch the asynchronous operation
            // The session is constructed with a strand to
            // ensure that handlers do not execute concurrently.
            mSession = std::make_shared<session>(ioc, ctx);

            mSession->run(host, port);

            // Run the I/O service. The call will return when
            // the get operation is complete.
            ioc.run();

        }
        catch(std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return response;
    }

    void Send(const std::string &message)
    {
        if (mSession)
        {
            mSession->Send(message);
        }
    }

    void Close()
    {
        if (mSession)
        {
            mSession->Close();
        }
    }

private:

    // Sends a WebSocket message and prints the response
    class session : public std::enable_shared_from_this<session>
    {
        boost::asio::ip::tcp::resolver resolver_;
        boost::beast::websocket::stream<
            boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
        boost::beast::flat_buffer buffer_;
        std::string host_;

    public:
        // Resolver and socket require an io_context
        explicit
        session(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx);

        // Start the asynchronous operation
        void
        run(const std::string &host, const std::string &port);

        void Close()
        {
            // Close the WebSocket connection
            ws_.async_close(boost::beast::websocket::close_code::normal,
                boost::beast::bind_front_handler(
                    &session::on_close,
                    shared_from_this()));
        }

        void Send(const std::string &message);


        void
        on_resolve(
            boost::beast::error_code ec,
            boost::asio::ip::tcp::resolver::results_type results);

        void
        on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);

        void
        on_ssl_handshake(boost::beast::error_code ec);

        void
        on_handshake(boost::beast::error_code ec);

        void
        on_write(
            boost::beast::error_code ec,
            std::size_t bytes_transferred);

        void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);


        void
        on_close(boost::beast::error_code ec);
    };

    std::shared_ptr<session> mSession;
};

#endif // WEBSOCKET_CLIENT_H
