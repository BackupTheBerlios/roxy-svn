//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_PROXY_REQUEST_HANDLER_HPP
#define HTTP_PROXY_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>

namespace http {
namespace proxy {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
public:
  /// Construct with a directory containing files to be served.
  explicit request_handler();

  /// Handle a request and produce a reply.
  void handle_header(const std::string local_ip,
      const std::string remote_ip, const request& req_in, request& req_out, reply& rep);

  void handle_body();

private:

  static bool request_handler::parse_uri(const std::string& uri_in, std::string& uri_out,
          std::string& host_out, std::string& port_out);

  static bool parse_host(const std::string& host_in, std::string& host_out, std::string& port_out);

};

} // namespace proxy
} // namespace http

#endif // HTTP_PROXY_REQUEST_HANDLER_HPP
