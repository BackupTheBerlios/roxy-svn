//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_PROXY_REQUEST_HPP
#define HTTP_PROXY_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "header.hpp"

namespace http {
namespace proxy {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  
  std::vector<header> headers;
  
  std::map< std::string, std::string > headers2;
  
  std::string host;
  std::string port;
  std::string content;
  
  bool has_body()
  {
    if(body_length() > 0)
      return true;
    else
      return false;
  }
  
  int body_length()
  {
    std::string content_length_str = headers2["Content-Length"];
    if(content_length_str.empty())
      return -1;
    else
      return boost::lexical_cast<int>(content_length_str);
  }
  
  std::string to_buffers()  
  {
    std::vector<boost::asio::const_buffer> buffers;
    std::string req = method + " " + uri + " HTTP/" 
    + boost::lexical_cast<std::string>(http_version_major) + "." 
    + boost::lexical_cast<std::string>(http_version_minor) + "\r\n";
    
    std::map<std::string, std::string>::iterator pos;
    for(pos = headers2.begin(); pos != headers2.end(); pos++)
    {
      req += (pos->first + ": " + pos->second + "\r\n");
    }    
    req += "\r\n";    
    return req;
  }
  
};

} // namespace proxy
} // namespace http

#endif // HTTP_PROXY_REQUEST_HPP
