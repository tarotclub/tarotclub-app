#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#define SEHE_USE_SSL
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include "openssl/ssl3.h"
#include "HttpProtocol.h"

//using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

enum { max_length = 8*1024 };

//class ExceptionTimeout : public std::exception
//{

//};

class HttpClient
{
public:
  HttpClient(asio::io_context& io_context,
      asio::ssl::context& context,
      asio::ip::tcp::resolver::iterator endpoint_iterator,
      const HttpRequest &req)
    : io_ctx_(io_context)
    , r(req)
    , socket_(io_context, context)
    , deadline_(io_context)
  {

    socket_.set_verify_mode(asio::ssl::verify_none);
    start_connect(endpoint_iterator);
    // Start the deadline actor. You will note that we're not setting any
    // particular deadline here. Instead, the connect and input actors will
    // update the deadline prior to each asynchronous operation.
    deadline_.async_wait(std::bind(&HttpClient::check_deadline, this));
  }

  // This function terminates all the actors to shut down the connection. It
  // may be called by the user of the client class, or by the class itself in
  // response to graceful termination or an unrecoverable error.
    void stop()
    {
        io_ctx_.stop();
    }

    HttpReply &get_reply() {
        return reply;
    }

    bool is_timeout() const {
        return timeout_;
    }

private:
  void check_deadline()
  {
    if (stopped_)
      return;

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expiry() <= asio::steady_timer::clock_type::now())
    {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled.
        timeout_ = true;
        stop();

        // There is no longer an active deadline. The expiry is set to the
        // maximum time point so that the actor takes no action until a new
        // deadline is set.
        deadline_.expires_at(asio::steady_timer::time_point::max());

         throw( std::runtime_error( "Oops!" ) );
    }

    // Put the actor back to sleep.
    deadline_.async_wait(std::bind(&HttpClient::check_deadline, this));
  }

  void start_connect(asio::ip::tcp::resolver::iterator endpoint_iter)
  {
      if (endpoint_iter != asio::ip::tcp::resolver::iterator())
      {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n" << std::endl;

        // Set a deadline for the connect operation.
        deadline_.expires_after(std::chrono::seconds(5));

        asio::async_connect(socket_.lowest_layer(), endpoint_iter, std::bind(&HttpClient::handle_connect, this, _1, _2));
      }
      else
      {
          stop();
      }
  }

  void handle_connect(const std::error_code& error,
      asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
  {
    if (stopped_)
      return;

    // The async_connect() function automatically opens the socket at the start
    // of the asynchronous operation. If the socket is closed at this time then
    // the timeout handler must have run first.
    if (!socket_.lowest_layer().is_open())
    {
      std::cout << "Connect timed out\n";

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
    }

    // Check if the connect operation failed before the deadline expired.
    else if (error)
    {
      std::cout << "Connect error: " << error.message() << "\n";

      // We need to close the socket used in the previous connection attempt
      // before starting a new one.
      socket_.shutdown();

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
    }

    // Otherwise we have successfully established a connection.
    else
    {
      std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

        handshake();
    }
  }

  void handshake()
  {
    socket_.async_handshake(asio::ssl::stream_base::client,
        [this](const std::error_code& error)
        {
          if (!error)
          {
              // Start the input actor.
                start_read();
                start_write();
          }
          else
          {
            std::cout << "Handshake failed: " << error.message() << "\n" << std::endl;
          }
        });
  }

    void start_read()
    {
        // Set a deadline for the read operation.
        deadline_.expires_after(std::chrono::seconds(5));

        // Start an asynchronous operation to read a newline-delimited message.
        asio::async_read_until(socket_, asio::dynamic_buffer(input_buffer_), '\n', std::bind(&HttpClient::handle_read_status_line, this, _1, _2));
    }

    void start_write()
    {
        if (stopped_)
          return;

        std::string request = HttpProtocol::GenerateRequest(r);

        // Start an asynchronous operation to send a heartbeat message.
        asio::async_write(socket_, asio::buffer(request), std::bind(&HttpClient::handle_write, this, _1));
    }

    void handle_write(const std::error_code& error)
    {
        if (stopped_)
          return;

        if (!error)
        {
          std::cout << "Request sent success\n" << std::endl;
        }
        else
        {
          std::cout << "Error on send request: " << error.message() << "\n" << std::endl;
          stop();
        }
    }

    void handle_read_status_line(const std::error_code& err, std::size_t n)
    {
        if (!err)
        {

            if (HttpProtocol::ParseReplyFirstLine(input_buffer_, reply))
            {
                // Remove first line
                input_buffer_.erase(0, n);
                // Read the response headers, which are terminated by a blank line.
                asio::async_read_until(socket_, asio::dynamic_buffer(input_buffer_), "\r\n\r\n",  std::bind(&HttpClient::handle_read_headers, this, _1, _2));
            }
            else
            {

                std::cout << "Parse reply first line failure" << std::endl;
                return;
            }
        }
        else
        {
            std::cout << "Error: " << err << "\n";
        }
    }

    void handle_read_headers(const std::error_code& err, std::size_t n)
    {
      if (!err)
      {
          if (HttpProtocol::ParseReplyHeaders(input_buffer_, reply))
          {
              input_buffer_.erase(0, n);
              handle_read_content(err, input_buffer_.size());
          }
          else
          {

              std::cout << "Parse headers failure" << std::endl;
              return;
          }
      }
      else
      {
        std::cout << "Error: " << err << "\n";
      }
    }

    void handle_read_content(const std::error_code& err, std::size_t n)
    {
        (void) n;
        if (!err)
        {
            reply.body.append(input_buffer_);
            input_buffer_.clear();
            if (!reply.chunked && (reply.contentLength == reply.body.size()))
            {
                stop();
            }
            else
            {
                // Read the response headers, which are terminated by a blank line.
                asio::async_read(socket_, asio::dynamic_buffer(input_buffer_), asio::transfer_at_least(1), std::bind(&HttpClient::handle_read_content, this, _1, _2));
            }
        }
        else if (err != asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
            stop();
        }
        else
        {
            stop();
        }
    }

    asio::io_context& io_ctx_;
    HttpRequest r;
    HttpReply   reply;
    bool stopped_ = false;
    asio::ip::tcp::resolver::results_type endpoints_;
    asio::ssl::stream<asio::ip::tcp::socket> socket_;
    std::string input_buffer_;
    asio::steady_timer deadline_;
    bool timeout_ = false;
};

#endif // HTTP_CLIENT_H
