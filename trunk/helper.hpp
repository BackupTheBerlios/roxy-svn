// Copyright (c) 2008 Andrew Selivanov (andrew.selivanov at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <sstream>
#include <iomanip>
#include <fstream>

inline std::string time_stamp()
{
    const char* formatter = "%d-%m-%y %H:%M:%S";          
    std::time_t cur_time_t = std::time(NULL);
    std::tm cur_time_tm = *std::localtime(&cur_time_t);
    char buf[0x7f];
    size_t buf_length = std::strftime(buf, 0x7f, formatter, &cur_time_tm);
    return std::string(buf,buf_length);;
}

template <typename T>
void DBG(const T& msg)
{
    std::cout << time_stamp() << " " << GetCurrentThreadId() << " " << msg << std::endl;        
}

template <typename T1, typename T2>
void DBG(const T1& msg1, const T2& msg2)
{
    std::cout << time_stamp() << " " << GetCurrentThreadId() << " " << msg1 << " " << msg2 << std::endl;        
}

template <typename T1, typename T2, typename T3>
void DBG(const T1& msg1, const T2& msg2, const T3& msg3)
{
    std::cout << time_stamp() << " " << GetCurrentThreadId() << " " << msg1 << " " << msg2 << " " << msg3<< std::endl;        
}

template <typename T>
void DUMP(const T& data)
{
  const char* formatter = "%d%m%y%H%M%S";          
  std::time_t cur_time_t = std::time(NULL);
  std::tm cur_time_tm = *std::localtime(&cur_time_t);
  char buf[0x7f];
  size_t buf_length = std::strftime(buf, 0x7f, formatter, &cur_time_tm);

  static unsigned int dump_no = 0;
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(5) << dump_no;
  std::string file_name = "d" + ss.str() + "_" + std::string(buf,buf_length) + ".bin";
  dump_no++;
  
  std::fstream debug_file;      
  debug_file.open(file_name.c_str(), std::ios::binary | std::ios::out);
  debug_file << data;
  debug_file.close();  
}

#endif /*HELPER_HPP_*/
