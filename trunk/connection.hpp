//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_PROXY_CONNECTION_HPP
#define HTTP_PROXY_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http {
namespace proxy {

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  boost::mutex io_mutex;

  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
      request_handler& handler);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  /// Get the server socket associated with the connection.
  boost::asio::ip::tcp::socket& server_socket(); 

  /// Start the first asynchronous operation for the connection.
  void start();

private:
  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e);

  ///
  inline
  int do_connect_server(const std::string& host, const std::string& port);
  
  /// Handle header of the client request
  void handle_client_read_header(const boost::system::error_code& e, 
      std::size_t bytes_transferred);
  
  /// Handle body of the client request
  void handle_client_read_body(const boost::system::error_code& e, 
      std::size_t bytes_transferred);
  
  /// Handle header of the server reply
  void handle_server_write(const boost::system::error_code& e);

  /// Handle header of the server reply
  void handle_server_write_body(const boost::system::error_code& e, 
      std::size_t bytes_transferred);  
  
  ///
  void handle_server_read(const boost::system::error_code& e, 
      std::size_t bytes_transferred);  
  
  /// Handle body of the server reply
//  void handle_write_body(const boost::system::error_code& e); 
  
  /// Strand to ensure the connection's handlers are not called concurrently.
  boost::asio::io_service::strand strand_; 
  
  /// Socket for the connection.
  boost::asio::ip::tcp::socket socket_;
  
  /// Server socket for the connection.
  boost::asio::ip::tcp::socket server_socket_;

  /// The handler used to process the incoming request.
  request_handler& request_handler_;

  ///
  boost::asio::ip::tcp::resolver resolver_;
  
  /// Buffer for incoming data.
  boost::array<char, 8192> buffer_;

  /// Buffer for incoming data.
  boost::array<char, 8192> server_buffer_;
  
  boost::array<char, 8192> body_buffer_;  
  
  /// The incoming request.
  request request_;
  
  request request_server;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  reply reply_;
  
  int state;
  
  int id;
  
  std::string buff;
  
  int request_bytes_transferred;
  
  std::size_t header_bytes_parsed;
  
  boost::asio::const_buffer buff2;
  std::vector<boost::asio::const_buffer> testv;
  
  boost::asio::deadline_timer timer_;  
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace proxy
} // namespace http

#endif // HTTP_PROXY_CONNECTION_HPP
