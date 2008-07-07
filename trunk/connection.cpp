//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>

#include "request_handler.hpp"
#include "helper.hpp"

namespace http {
namespace proxy {

connection::connection(boost::asio::io_service& io_service,
    request_handler& handler)
  : strand_(io_service),
    socket_(io_service),
    server_socket_(io_service),
    request_handler_(handler),
    resolver_(io_service),
    request_bytes_transferred(0),
    timer_(io_service)
{
  static int id_ = 0;
  id = id_++;
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

boost::asio::ip::tcp::socket& connection::server_socket()
{
  return server_socket_;
}

void connection::start()
{  
  socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(
        boost::bind(&connection::handle_client_read_header, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));
}

int connection::do_connect_server(const std::string& host, const std::string& port)
{
  // Get a list of endpoints corresponding to the server name. 
  boost::system::error_code error;
  boost::asio::ip::tcp::resolver::query query(host, port);
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = 
    resolver_.resolve(query, error);
  
  if(error)
  {
    DBG("Host resolve error"); 
    return -1;
  }
  
  boost::asio::ip::tcp::resolver::iterator end;

  error = boost::asio::error::host_not_found;
  while (error && endpoint_iterator != end)
  {        
    server_socket_.close();                 
    server_socket_.connect(*endpoint_iterator++, error);       
  }   
  if (error)
  {
      DBG("Connection error"); 
      server_socket_.close();
      return -1;     
  } 
//  DBG("connected:", host, port);  
  return 0;
}

void connection::handle_client_read_header(const boost::system::error_code& e, 
    std::size_t bytes_transferred)
{
  if (!e)
  {
      //DBG(">>> PARSE <<<");
    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred, header_bytes_parsed);

    if (result)
    {
      std::string remote_address = socket_.remote_endpoint().address().to_string();
      std::string local_address = socket_.local_endpoint().address().to_string();
        
      request_handler_.handle_header(local_address, remote_address, request_, 
          request_server, reply_);
      
      int res = do_connect_server(request_server.host, request_server.port);
      if(res < 0)
      {
        reply_ = reply::stock_reply(reply::bad_gateway);
        boost::asio::async_write(socket_, reply_.to_buffers(),
            strand_.wrap(
              boost::bind(&connection::handle_write, shared_from_this(),
                boost::asio::placeholders::error)));        
      }

      DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");      
      DBG(id, request_server.host, request_server.uri);
      DBG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");      
      
      timer_.expires_from_now(boost::posix_time::seconds(5));
      timer_.async_wait(boost::bind(&connection::handle_write, shared_from_this(), 
          boost::asio::placeholders::error));      
      
      buff = request_server.to_buffers();      
      if(request_.has_body())
      { 
        DBG("HAS_BODY()");
        DBG(bytes_transferred, header_bytes_parsed);

        if(bytes_transferred != header_bytes_parsed)
        {

          buff += std::string(buffer_.data() + header_bytes_parsed, 
              bytes_transferred - header_bytes_parsed);
          
          request_bytes_transferred = bytes_transferred - header_bytes_parsed;
          
          //DBG(buff);          
          if(request_bytes_transferred == request_.body_length())
          {
            boost::asio::async_write(server_socket_, boost::asio::buffer(buff),
                strand_.wrap(
                  boost::bind(&connection::handle_server_write, shared_from_this(),
                    boost::asio::placeholders::error)));                      
          }
          else
          {
            boost::asio::async_write(server_socket_, boost::asio::buffer(buff),
                strand_.wrap(
                  boost::bind(&connection::handle_server_write_body, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));                      
          }
        }
        else
        {
        	request_bytes_transferred = 0;
            //DBG("[[[",buff,"]]]");
          buff2 = boost::asio::buffer(buff);
          testv.push_back(buff2);
          boost::asio::async_write(server_socket_, testv,
              strand_.wrap(
                boost::bind(&connection::handle_server_write_body, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred)));
        }
      }
      else
      {
    	  //DBG(buff);
        boost::asio::async_write(server_socket_, boost::asio::buffer(buff),
            strand_.wrap(
              boost::bind(&connection::handle_server_write, shared_from_this(),
                boost::asio::placeholders::error)));         
      }     
    }
    else if (!result)
    {
        //DBG(">>> BAD REQUEST <<<");
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
    }
    else
    {
      socket_.async_read_some(boost::asio::buffer(buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_client_read_header, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket. 
}

void connection::handle_client_read_body(const boost::system::error_code& e, 
    std::size_t bytes_transferred)
{
  //DBG("... handle_client_read_body ...");  
  if (!e)
  {
	    //DBG(request_bytes_transferred, bytes_transferred);
	    //DBG(std::string(body_buffer_.data(),bytes_transferred));
    request_bytes_transferred += bytes_transferred;          
    
    if(request_bytes_transferred < request_.body_length())
    {    
      boost::asio::async_write(server_socket_, boost::asio::buffer(body_buffer_, 
          bytes_transferred),
          strand_.wrap(
            boost::bind(&connection::handle_server_write_body, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));            
    }
    else if(request_bytes_transferred == request_.body_length())
    {
      boost::asio::async_write(server_socket_, boost::asio::buffer(body_buffer_, 
          bytes_transferred),
          strand_.wrap(
            boost::bind(&connection::handle_server_write, shared_from_this(),
              boost::asio::placeholders::error)));      
    }
    else
    {
        boost::asio::async_write(server_socket_, boost::asio::buffer(body_buffer_, 
            bytes_transferred),
            strand_.wrap(
              boost::bind(&connection::handle_server_write, shared_from_this(),
                boost::asio::placeholders::error)));      
    	
//    	  DBG("... BAD REQ ...");     	
//      reply_ = reply::stock_reply(reply::bad_request);
//      boost::asio::async_write(socket_, reply_.to_buffers(),
//          strand_.wrap(
//            boost::bind(&connection::handle_write, shared_from_this(),
//              boost::asio::placeholders::error)));      
    }    
  }
}

void connection::handle_server_write_body(const boost::system::error_code& e, 
    std::size_t bytes_transferred)
{
  if (!e)
  { 
    socket_.async_read_some(boost::asio::buffer(body_buffer_),
        strand_.wrap(
          boost::bind(&connection::handle_client_read_body, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)));                        
  }
  
}

void connection::handle_server_write(const boost::system::error_code& e)
{
  if (!e)
  {   
      server_socket_.async_read_some(boost::asio::buffer(server_buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_server_read, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));          
  }
  
}

inline void replace_all(std::string& str, const std::string to_replace, 
		const std::string replacement)
{
	size_t k = to_replace.length();
    size_t i = 0;
            
    while (std::string::npos != (i = str.find(to_replace, i)))
    {
        str.replace(i, k, replacement);
        i += k;
    }        
}


void connection::handle_server_read(const boost::system::error_code& e, 
    std::size_t bytes_transferred)
{  
  if (!e) 
  {
	  std::string mybuff = std::string(server_buffer_.data(),bytes_transferred);
	  
	  //DBG(mybuff);
	  
    boost::asio::async_write(socket_, boost::asio::buffer(boost::asio::buffer(mybuff), 
        bytes_transferred),
        strand_.wrap(
          boost::bind(&connection::handle_server_write, shared_from_this(),
            boost::asio::placeholders::error)));   
  }
  else
  {
    if(request_.headers2["Proxy-Connection"] == "Keep-Alive")
    {
      DBG("keep-alive!");            
      socket_.async_read_some(boost::asio::buffer(buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_client_read_header, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));      
    }
    else
    {
      DBG("shutdown!", request_.headers2["Proxy-Connection"]);      
      boost::system::error_code ignored_ec;
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);    
      server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);      
    }
    
  }
}


void connection::handle_write(const boost::system::error_code& e)
{
  if (!e)
  {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);    
    server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);    
  }       
  
  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

} // namespace proxy
} // namespace http
