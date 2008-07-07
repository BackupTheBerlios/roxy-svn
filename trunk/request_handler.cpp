//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>

//Boost
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "request_handler.hpp"
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

#include "helper.hpp"

namespace http {
namespace proxy {

inline bool caseInsCharCompareN(char a, char b)
{
  return(toupper(a) == toupper(b));
}

inline bool caseInsCompare(const std::string& s1, const std::string& s2)
{
  return((s1.size( ) == s2.size( )) &&
      equal(s1.begin( ), s1.end( ), s2.begin( ), caseInsCharCompareN));
}

request_handler::request_handler()
{

}

void request_handler::handle_header(const std::string local_ip, const std::string remote_ip,
    const request& req_in, request& req_out, reply& rep)
{
  bool is_anonymous = false;
  std::string req_host = "";
  std::string req_port = "80";
  std::string req_uri;

  parse_uri(req_in.uri, req_uri, req_host, req_port);

  req_out = req_in;
  req_out.uri = req_uri;
  req_out.host = req_host;
  req_out.port = req_port;

  if(!is_anonymous)
  {
    if(!req_out.headers2["X-Forwarded-For"].empty())
    {
        req_out.headers2["X-Forwarded-For"] += ", ";
    }

    req_out.headers2["X-Forwarded-For"] += remote_ip;

    if(!req_out.headers2["Via"].empty())
    {
        req_out.headers2["Via"] += ", ";
    }

    req_out.headers2["Via"] += boost::lexical_cast<std::string>(req_out.http_version_major)
    + "." + boost::lexical_cast<std::string>(req_out.http_version_minor) + " " + local_ip;
  }

  req_out.headers2["Connection"] = "close";
  req_out.headers2.erase("Proxy-Connection");
}

bool request_handler::parse_uri(const std::string& uri_in, std::string& uri_out,
        std::string& host_out, std::string& port_out)
{
    uri_out = uri_in;

    if(uri_in.length() > 7)
    {
        if( (uri_in.substr(0,7) == "http://"))
        {
            int pos = uri_in.find('/', 7);
            if(pos != std::string::npos)
            {
                std::string uri = uri_in.substr(pos, uri_in.length() - 7);
                std::string host = uri_in.substr(7, pos - 7);
                parse_host(host, host_out, port_out);
                uri_out = uri;
            }
        }
    }

    return true;
}

bool request_handler::parse_host(const std::string& host_in, std::string& host_out, std::string& port_out)
{
    if(!host_in.length())
    {
        return false;
    }

    int pos = host_in.find(':');
    if(pos != std::string::npos)
    {
        host_out = host_in.substr(0, pos);
        port_out = host_in.substr(pos + 1, host_in.length() - pos - 1);

    }
    else
    {
        host_out = host_in;
        port_out = "80";
    }

    return true;
}

} // namespace proxy
} // namespace http
