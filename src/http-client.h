#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include "openssl/ssl3.h"

using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

enum { max_length = 1024 };

class HttpClient
{
public:
  HttpClient(asio::io_context& io_context,
      asio::ssl::context& context,
      const tcp::resolver::results_type& endpoints)
    : socket_(io_context, context)
  {
    socket_.set_verify_mode(asio::ssl::verify_none);
//    socket_.set_verify_callback(
//        std::bind(&HttpClient::verify_certificate, this, _1, _2));

    connect(endpoints);
  }

private:
  bool verify_certificate(bool preverified,
      asio::ssl::verify_context& ctx)
  {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n" << std::endl;

    return preverified;
  }

  void connect(const tcp::resolver::results_type& endpoints)
  {
    asio::async_connect(socket_.lowest_layer(), endpoints,
        [this](const std::error_code& error,
          const tcp::endpoint& /*endpoint*/)
        {
          if (!error)
          {
            handshake();
          }
          else
          {
            std::cout << "Connect failed: " << error.message() << "\n" << std::endl;
          }
        });
  }

  void handshake()
  {
    socket_.async_handshake(asio::ssl::stream_base::client,
        [this](const std::error_code& error)
        {
          if (!error)
          {
            send_request();
          }
          else
          {
            std::cout << "Handshake failed: " << error.message() << "\n" << std::endl;
          }
        });
  }

  void send_request()
  {
    std::cout << "Send request" << std::endl;
    std::string request = "COUCOU";

    asio::async_write(socket_,
        asio::buffer(request, request.size()),
        [this](const std::error_code& error, std::size_t length)
        {
          if (!error)
          {
            receive_response(length);
          }
          else
          {
            std::cout << "Write failed: " << error.message() << "\n" << std::endl;
          }
        });
  }

  void receive_response(std::size_t length)
  {
    asio::async_read(socket_,
        asio::buffer(reply_, length),
        [this](const std::error_code& error, std::size_t length)
        {
          if (!error)
          {
            std::cout << "Reply: ";
            std::cout.write(reply_, length);
            std::cout << "\n" << std::endl;
          }
          else
          {
            std::cout << "Read failed: " << error.message() << "\n" << std::endl;
          }
        });
  }

  asio::ssl::stream<tcp::socket> socket_;
  char reply_[max_length];
};

#endif // HTTP_CLIENT_H
