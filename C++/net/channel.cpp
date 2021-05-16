/**
 * Creating a channel with both server/client operations.
 * Server and client code use Asio library and are based on the official
 * examples from Christopher M. Kohlhoff.
 * 
 * Copyright (c) 2021 Roberto Metere <roberto@metere.it>
 * Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "channel.h"
#include <iostream>
#include <utility>
#include <asio.hpp>
#include <thread>
// -----------------------------------------------------------------------------

namespace net {
std::ostream& operator<<(std::ostream& stream, const Bytes& data)
{
  for (size_t i = 0; i < data.size(); i++) {
    stream << data[i];
  }
    
  return stream;
};
}; // Namespace - why is this required? (otherwise link errors...)
// -----------------------------------------------------------------------------

using namespace net;
// -----------------------------------------------------------------------------

Channel::Channel(const int port)
{
  if (port < 1 || port > 65535) {
    std::cerr << "Channel(). Port must be in the range 1-65535. Using default " << CHANNEL_H_DEFAULT_PORT << "." << std::endl;
    this->port = CHANNEL_H_DEFAULT_PORT;
  } else {
    this->port = port;
  }
};
// -----------------------------------------------------------------------------

Channel::~Channel() { };
// -----------------------------------------------------------------------------

void Channel::send(
    const std::string &message
  , const int port
  , const std::string &host
  , const unsigned int attempts
  , const std::chrono::duration<int,std::milli> &delay_ms
)
{
  Bytes data;
  for (size_t i = 0; i < message.size(); i++) {
    data.push_back(message[i]);
  }
  this->send(data, port, host, attempts, delay_ms);
};
// -----------------------------------------------------------------------------

void Channel::send(
    const Bytes &message
  , const int port
  , const std::string &host
  , const unsigned int attempts
  , const std::chrono::duration<int,std::milli> &delay_ms
)
{
  unsigned int i;
  
  if (message.size() == 0) {
    return; // Nothing to send
  }
  
  for (i = 0U; i < attempts; i++) {
    try {
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::connect(socket, resolver.resolve(host, std::to_string(port)));
      asio::write(socket, asio::const_buffer(&message[0], message.size()));
      break; // Attempt successful!
    } catch (std::exception& e) {
      std::cerr << "Channel::send(). Attempt " << i << " of " << attempts << " failed. Error: " << e.what() << "." << std::endl;
      
      if (i == attempts - 1) {
        std::cerr << "Channel::send(). Maximum attempts reached, exiting..." << std::endl;
        std::exit(1);
      }
      
      std::this_thread::sleep_for(delay_ms);
    }
  }
};
// -----------------------------------------------------------------------------

std::string Channel::receiveText(const int port)
{
  Bytes data;
  
  data = this->receive(port);
  
  return std::string(data.begin(), data.end());
};
// -----------------------------------------------------------------------------

Bytes Channel::receive(const int port)
{
  byte buffer[CHANNEL_H_MAX_BUFFER];
  Bytes data;
  size_t i, length;
  
  try {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), (port >= 1 && port <= 65535) ? port : this->port));
    
    asio::ip::tcp::socket socket = acceptor.accept();
    asio::error_code error;
    length = socket.read_some(asio::buffer(buffer), error);
    buffer[CHANNEL_H_MAX_BUFFER - 1] = '\0'; // Reduce buffer overflows chances?
    if (length < CHANNEL_H_MAX_BUFFER) {
      buffer[length] = '\0'; // Manually required?
    }
    
    if (error == asio::error::eof) {
      return data; // Connection closed cleanly by peer.
    } else if (error) {
      throw asio::system_error(error); // Some other error.
    }
    
    // Copy buffer to data
    for (i = 0; i <= length; i++) {
      data.push_back(buffer[i]);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  
  return data;
};
// -----------------------------------------------------------------------------
