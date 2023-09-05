#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <cstdlib>
#include <cstring>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "openssl/ssl3.h"

// Report a failure
static void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class HttpClient
{
public:
    enum Action { HTTP_GET, HTTP_POST };
    struct Request {
        std::string host;
        std::string port;
        std::string target;
        std::string body;
        Action action;

        bool quit;

        Request (bool q = false)
            : quit(q)
            , action(HTTP_POST)
        {
        }
    };

    std::string ExecuteAsync(const Request &request)
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
            auto sess = std::make_shared<session>(boost::asio::make_strand(ioc), ctx);

            sess->run(request);

            // Run the I/O service. The call will return when
            // the get operation is complete.
            ioc.run();

            response = sess->getResponseBody();
        }
        catch(std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return response;
    }

private:

    // Performs an HTTP GET and prints the response
    class session : public std::enable_shared_from_this<session>
    {
        boost::asio::ip::tcp::resolver resolver_;
        boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
        boost::beast::flat_buffer buffer_; // (Must persist between reads)
        boost::beast::http::request<boost::beast::http::string_body> req_;
        boost::beast::http::response<boost::beast::http::string_body> res_;

    public:

        std::string getResponseBody() {
            return res_.body();
        }

        explicit
        session(
            boost::asio::any_io_executor ex,
            boost::asio::ssl::context& ctx)
        : resolver_(ex)
        , stream_(ex, ctx)
        {
        }

        // Start the asynchronous operation
        void run(const Request &request)
        {
            // Set SNI Hostname (many hosts need this to handshake successfully)
            if(! SSL_set_tlsext_host_name(stream_.native_handle(), request.host.c_str()))
            {
                boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                std::cerr << ec.message() << "\n";
                return;
            }

            boost::beast::http::verb verb = request.action == HTTP_POST ? boost::beast::http::verb::post : boost::beast::http::verb::get;

            // Set up an HTTP GET request message
            req_.version(11);
            req_.method(verb);
            req_.target(request.target);
            req_.set(boost::beast::http::field::host, request.host);
            req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req_.set(boost::beast::http::field::content_type, "application/json");

            if (request.action == HTTP_POST)
            {
                req_.body() = request.body;
                req_.prepare_payload();
            }

            // Look up the domain name
            resolver_.async_resolve(
                request.host,
                request.port,
                boost::beast::bind_front_handler(
                    &session::on_resolve,
                    shared_from_this()));
        }

        void
        on_resolve(
            boost::beast::error_code ec,
            boost::asio::ip::tcp::resolver::results_type results)
        {
            if(ec)
                return fail(ec, "resolve");

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

            // Make the connection on the IP address we get from a lookup
            boost::beast::get_lowest_layer(stream_).async_connect(
                results,
                boost::beast::bind_front_handler(
                    &session::on_connect,
                    shared_from_this()));
        }

        void
        on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type)
        {
            if(ec)
                return fail(ec, "connect");

            // Perform the SSL handshake
            stream_.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(
                    &session::on_handshake,
                    shared_from_this()));
        }

        void
        on_handshake(boost::beast::error_code ec)
        {
            if(ec)
                return fail(ec, "handshake");

            // Set a timeout on the operation
           boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

            // Send the HTTP request to the remote host
            boost::beast::http::async_write(stream_, req_,
                boost::beast::bind_front_handler(
                    &session::on_write,
                    shared_from_this()));
        }

        void
        on_write(
            boost::beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail(ec, "write");

            // Receive the HTTP response
            boost::beast::http::async_read(stream_, buffer_, res_,
                boost::beast::bind_front_handler(
                    &session::on_read,
                    shared_from_this()));
        }

        void
        on_read(
            boost::beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail(ec, "read");

            // Write the message to standard out
            std::cout << res_ << std::endl;

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

            // Gracefully close the stream
            stream_.async_shutdown(
                boost::beast::bind_front_handler(
                    &session::on_shutdown,
                    shared_from_this()));
        }

        void
        on_shutdown(boost::beast::error_code ec)
        {
            if(ec == boost::asio::error::eof)
            {
                // Rationale:
                // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                ec = {};
            }
            if(ec)
                return fail(ec, "shutdown");

            // If we get here then the connection is closed gracefully
        }
    };

};

#endif // HTTP_CLIENT_H
