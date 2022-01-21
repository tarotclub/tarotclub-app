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
  HttpClient()
    : resolver(io_context)
    , ssl_ctx(asio::ssl::context::tls)
    , socket_(io_context, ssl_ctx)
    , deadline_(io_context)
    {

    }

    void StartHttp(const std::string &host, const std::string &port, std::function<void (bool)> connectCallback)
    {
        mConnectCallback = connectCallback;
        mIsWebSocket = false;
        mConnected = false;

        asio::ip::tcp::resolver::query query(host, port);
        asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        socket_.set_verify_mode(asio::ssl::verify_none);
        start_connect(endpoint_iterator);
        // Start the deadline actor. You will note that we're not setting any
        // particular deadline here. Instead, the connect and input actors will
        // update the deadline prior to each asynchronous operation.
        deadline_.async_wait(std::bind(&HttpClient::check_deadline, this));
    }

    void StartWebSocket(const std::string &host, const std::string &port, const std::string &protocol)
    {
        mIsWebSocket = true;
        mWsProtocol = protocol;

        asio::ip::tcp::resolver::query query(host, port);
        asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        socket_.set_verify_mode(asio::ssl::verify_none);
        start_connect(endpoint_iterator);
    }

    void Write(const HttpRequest &req, std::function<void (bool success, const HttpReply &reply)> writeCallback)
    {
        if (stopped_)
          return;

        mWriteCallback = writeCallback;

        std::string request_raw = HttpProtocol::GenerateRequest(req);

        // Start an asynchronous operation to send a heartbeat message.
        asio::async_write(socket_, asio::buffer(request_raw), std::bind(&HttpClient::handle_write, this, _1));
    }

    void Run()
    {
        io_context.restart();
        asio::executor_work_guard<decltype(io_context.get_executor())> work{io_context.get_executor()};
        io_context.run();
    }

    // This function terminates all the actors to shut down the connection. It
    // may be called by the user of the client class, or by the class itself in
    // response to graceful termination or an unrecoverable error.
    void stop()
    {
        io_context.stop();
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

            // There is no longer an active deadline. The expiry is set to the
            // maximum time point so that the actor takes no action until a new
            // deadline is set.
            deadline_.expires_at(asio::steady_timer::time_point::max());

            // FIXME send callback timeout
        //         throw( std::runtime_error( "Oops!" ) );
        }

        // Put the actor back to sleep.
        deadline_.async_wait(std::bind(&HttpClient::check_deadline, this));
    }

  void start_connect(asio::ip::tcp::resolver::iterator endpoint_iter)
  {
      if (endpoint_iter != asio::ip::tcp::resolver::iterator())
      {
        std::cout << "Trying " << endpoint_iter->endpoint() << "..." << std::endl;

        // Set a deadline for the connect operation.
        deadline_.expires_after(std::chrono::seconds(5));

        asio::async_connect(socket_.lowest_layer(), endpoint_iter, std::bind(&HttpClient::handle_connect, this, _1, _2));
      }
      else
      {
          if (mConnectCallback)
          {
              mConnectCallback(false);
          }
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
            std::cout << "Connect timed out" << std::endl;

            // Try the next available endpoint.
            start_connect(++endpoint_iter);
        }

        // Check if the connect operation failed before the deadline expired.
        else if (error)
        {
          std::cout << "Connect error: " << error.message() << std::endl;

          // We need to close the socket used in the previous connection attempt
          // before starting a new one.
          socket_.shutdown();

          // Try the next available endpoint.
          start_connect(++endpoint_iter);
        }

        // Otherwise we have successfully established a connection.
        else
        {
            std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;
            // Restart deadline for handshake
            deadline_.expires_after(std::chrono::seconds(5));
            sslHandshake();
        }
    }

    void sslHandshake()
    {
        socket_.async_handshake(asio::ssl::stream_base::client,
            [this](const std::error_code& error)
            {
                if (!error)
                {
                    mConnected = true;
                    std::cout << "SSL handshake success!" << std::endl;
                    // Put the actor back to sleep.
                    deadline_.async_wait(std::bind(&HttpClient::check_deadline, this));
                    // Start the input actor.
                    start_read();
                    if (mConnectCallback)
                    {
                        mConnectCallback(true);
                    }
                }
                else
                {
                    std::cout << "Handshake failed: " << error.message() << std::endl;
                    if (mConnectCallback)
                    {
                        mConnectCallback(false);
                    }
                }
            }
        );
    }

    void start_read()
    {
       // Start an asynchronous operation to read a newline-delimited message.
        asio::async_read_until(socket_, asio::dynamic_buffer(input_buffer_), '\n', std::bind(&HttpClient::handle_read_http_status_line, this, _1, _2));
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
            mWriteCallback(false, mReply);
        }
    }

    void handle_read_http_status_line(const std::error_code& err, std::size_t n)
    {
        if (!err)
        {

            if (HttpProtocol::ParseReplyFirstLine(input_buffer_, mReply))
            {
                // Remove first line
                input_buffer_.erase(0, n);
                // Read the response headers, which are terminated by a blank line.
                asio::async_read_until(socket_, asio::dynamic_buffer(input_buffer_), "\r\n\r\n",  std::bind(&HttpClient::handle_read_http_headers, this, _1, _2));
            }
            else
            {

                std::cout << "Parse reply first line failure" << std::endl;
                return;
            }
        }
        else
        {
            std::cout << "Error: " << err << std::endl;
        }
    }

    void handle_read_http_headers(const std::error_code& err, std::size_t n)
    {
      if (!err)
      {
          if (HttpProtocol::ParseReplyHeaders(input_buffer_, mReply))
          {
              input_buffer_.erase(0, n);
              handle_read_http_content(err, input_buffer_.size());
          }
          else
          {

              std::cout << "Parse headers failure" << std::endl;
              return;
          }
      }
      else
      {
        std::cout << "Error: " << err << std::endl;
      }
    }

    void handle_read_http_content(const std::error_code& err, std::size_t n)
    {
        (void) n;
        if (!err)
        {
            mReply.body.append(input_buffer_);
            input_buffer_.clear();
            if (!mReply.chunked && (mReply.contentLength == mReply.body.size()))
            {
                // If we have a valid Write callback, then it was a write request
                if (mWriteCallback)
                {
                    mWriteCallback(true, mReply);
                }
               // start_read();
            }
            else
            {
                // Read the response headers, which are terminated by a blank line.
                asio::async_read(socket_, asio::dynamic_buffer(input_buffer_), asio::transfer_at_least(1), std::bind(&HttpClient::handle_read_http_content, this, _1, _2));
            }
        }
        else if (err != asio::error::eof)
        {
            std::cout << "Error: " << err << std::endl;
            mWriteCallback(false, mReply);
            start_read();
        }
        else
        {
            mWriteCallback(false, mReply);
            start_read();
        }
    }

    // Websocket
    std::string BuildWebSocketHandshake(const std::string &path, const std::string &host)
    {
        WebSocketRequest ws;

        ws.request.method = "GET";
        ws.request.protocol = "HTTP/1.1";
        ws.request.query = path; // eg: "/api/v1/machines/upstream/data"
        ws.request.body = "";
        ws.protocol = mWsProtocol;
        ws.request.headers["Host"] = host;
    //    ws.request.headers["Content-length"] = std::to_string(0);

        return HttpProtocol::GenerateWebSocketRequest(ws);
    }

    // Websocket opcodes, from http://tools.ietf.org/html/rfc6455
    static const std::uint8_t WEBSOCKET_OPCODE_CONTINUATION     = 0x00U;
    static const std::uint8_t WEBSOCKET_OPCODE_TEXT             = 0x01U;
    static const std::uint8_t WEBSOCKET_OPCODE_BINARY           = 0x02U;
    static const std::uint8_t WEBSOCKET_OPCODE_CONNECTION_CLOSE = 0x08U;
    static const std::uint8_t WEBSOCKET_OPCODE_PING             = 0x09U;
    static const std::uint8_t WEBSOCKET_OPCODE_PONG             = 0x0AU;

    enum WS_RESULT { WS_SEND_PONG, WS_PARTIAL, WS_DATA, WS_CLOSE };

    /*****************************************************************************/
    WS_RESULT DecodeWsData(std::string &buf, std::string &payload)
    {
        // Return true if data reception is complete
        WS_RESULT res = WS_CLOSE;

        // Having buf unsigned char * is important, as it is used below in arithmetic
        uint64_t i, len, mask_len = 0, header_len = 0, data_len = 0;

        std::uint32_t buf_len = static_cast<std::uint32_t>(buf.size());

        /* Extracted from the RFC 6455 Chapter 5-2
         *
        The length of the "Payload data", in bytes: if 0-125, that is the
        payload length.  If 126, the following 2 bytes interpreted as a
        16-bit unsigned integer are the payload length.  If 127, the
        following 8 bytes interpreted as a 64-bit unsigned integer (the
        most significant bit MUST be 0) are the payload length.  Multibyte
        length quantities are expressed in network byte order.  Note that
        in all cases, the minimal number of bytes MUST be used to encode
        the length, for example, the length of a 124-byte-long string
        can't be encoded as the sequence 126, 0, 124.  The payload length
        is the length of the "Extension data" + the length of the
        "Application data".  The length of the "Extension data" may be
        zero, in which case the payload length is the length of the
        "Application data". */
        if (buf_len >= 2)
        {
            len = buf[1] & 127;
            mask_len = buf[1] & 128 ? 4 : 0;
            if (len < 126 && buf_len >= mask_len)
            {
                data_len = len;
                header_len = 2 + mask_len;
            }
            else if (len == 126 && buf_len >= 4 + mask_len)
            {
                header_len = 4 + mask_len;
                uint8_t byte1 = static_cast<uint8_t>(buf[2]);
                uint8_t byte2 = static_cast<uint8_t>(buf[3]);
                data_len = (static_cast<uint64_t>(byte1) << 8) + byte2;
            }
            else if (buf_len >= 10 + mask_len)
            {
                header_len = 10 + mask_len;
                data_len = (uint64_t) (((uint64_t) htonl(* (uint32_t *) &buf[2])) << 32) + htonl(* (uint32_t *) &buf[6]);
            }
        }

        // frame_len = header_len + data_len;
        // Apply mask if necessary
        char *mask_ptr = (&buf[0] + header_len) - mask_len; // pointer to the mask located in the header
        if (mask_len > 0)
        {
            for (i = 0; i < data_len; i++)
            {
                buf[i + header_len] ^= mask_ptr[i % 4];
            }
        }

        std::uint8_t opcode = static_cast<uint8_t>(buf[0]) & 0xFU;
        bool FIN = (static_cast<uint8_t>(buf[0]) & 0x80U) == 0x80U;
    //    TLogNetwork("received opcode: " + WsOpcodeToString(opcode));

        /*
        Manage fragmentation here: extract from the RFC:

        EXAMPLE: For a text message sent as three fragments, the first
          fragment would have an opcode of 0x1 and a FIN bit clear, the
          second fragment would have an opcode of 0x0 and a FIN bit clear,
          and the third fragment would have an opcode of 0x0 and a FIN bit
          that is set.

          */
        if(opcode == WEBSOCKET_OPCODE_PING)
        {
            res = WS_SEND_PONG;

        }
        else if (opcode == WEBSOCKET_OPCODE_CONNECTION_CLOSE)
        {
            res = WS_CLOSE;
        }
        else
        {
            if ((opcode == WEBSOCKET_OPCODE_TEXT) ||
                (opcode == WEBSOCKET_OPCODE_BINARY))
            {
                payload = buf.substr(header_len, data_len);
            }
            else if(opcode == WEBSOCKET_OPCODE_CONTINUATION)
            {
                payload += buf.substr(header_len, data_len);
            }

            if (FIN)
            {
                // We can deliver data to the consumer
                res = WS_DATA;
            }
            else
            {
                res = WS_PARTIAL;
            }
        }

        return res;
    }


    // ASIO STUFF
    asio::io_context io_context;
    asio::ip::tcp::resolver resolver;
    asio::ssl::context ssl_ctx;
    asio::ip::tcp::resolver::results_type endpoints_;
    asio::ssl::stream<asio::ip::tcp::socket> socket_;

    bool mIsWebSocket = false;
    std::string mWsProtocol;

    std::function<void (bool) > mConnectCallback;
    std::function<void (bool success, const HttpReply &reply)> mWriteCallback;

    HttpReply   mReply;
    bool stopped_ = false;
    std::string input_buffer_;
    asio::steady_timer deadline_;
    bool timeout_ = false;
    bool mConnected = false;
};

#endif // HTTP_CLIENT_H
