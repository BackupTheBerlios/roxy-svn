//
// win_main.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include "server.hpp"

#if defined(_WIN32)

#include <windows.h>

boost::function0<void> console_ctrl_function;

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    console_ctrl_function();
    return TRUE;
  default:
    return FALSE;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 4)
    {
      std::cerr << "Usage: http_proxy <address> <port> <threads>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    http_proxy 0.0.0.0 8080 1\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    http_proxy 0::0 8080 1\n";
      return 1;
    }

    // Initialise server.
    std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
    http::proxy::server s(argv[1], argv[2], num_threads);

    // Set console control handler to allow server to be stopped.
    console_ctrl_function = boost::bind(&http::proxy::server::stop, &s);
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    // Run the server until stopped.
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  //wait console_ctrl_function to complete
  Sleep(2000);

  return 0;
}

#endif // defined(_WIN32)
