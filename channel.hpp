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

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
//-----------------------------------------------------------------------------

#include <string>
#include <vector>
#include <chrono>

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 10000
#define MAX_BUFFER 65536

namespace net {
/**
 * A Channel offers send and receive methods to communicate through the network.
 */
template <class T> class Channel
{
public:
  /**
   * Constructor of a Channel.
   * 
   * @param port the listening port to open when receiving
   */
  Channel(const int port = DEFAULT_PORT);
  
  /**
   * Destructor
   */
  virtual ~Channel();
  
  /**
   * Send a message to another host:port
   * 
   * @param message the message to send
   * @param host the remote host to send the message to
   * @param port the port where the remote host is listening to
   * @param attempts number of attempts before giving up
   * @param delay_ms wait some milliseconds between two attempts
   */
  void send(
      const T &message
    , const int port
    , const std::string &host = DEFAULT_HOST
    , const unsigned int attempts = 10
    , const std::chrono::duration<int,std::milli> &delay_ms = std::chrono::milliseconds(1000)
  );
  
  /**
   * Receive a message.
   * 
   * @return the message received
   */
  T receive();
  
protected:
  std::vector<T> view; ///< Record what is sent or received
  
private:
  int port;  ///< Listening port for receiving
};
// -----------------------------------------------------------------------------

} // namespace
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#include <iostream>
#include <utility>
#include <asio.hpp>
#include <thread>
// -----------------------------------------------------------------------------

using namespace net;
// -----------------------------------------------------------------------------

template <class T> Channel<T>::Channel(const int port)
{
  if (port < 1 || port > 65535) {
    std::cerr << "Channel(). Port must be in the range 1-65535. Using default " << DEFAULT_PORT << "." << std::endl;
    this->port = DEFAULT_PORT;
  } else {
    this->port = port;
  }
}
// -----------------------------------------------------------------------------

template <class T> Channel<T>::~Channel() { }
// -----------------------------------------------------------------------------

template <class T> void Channel<T>::send(
      const T &message
    , const int port
    , const std::string &host
    , const unsigned int attempts
    , const std::chrono::duration<int,std::milli> &delay_ms
)
{
  if (message == "") {
    return; // Nothing to send
  }
  unsigned int i;
  
  for (i = 0U; i < attempts; i++) {
    try {
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::connect(socket, resolver.resolve(host, std::to_string(port)));
      asio::write(socket, asio::const_buffer(message.c_str(), message.length()));
      this->view.push_back(message);
      break; // Attempt successful!
    } catch (std::exception& e) {
      std::cerr << "Channel::send(). Exception: " << e.what() << std::endl;
      std::cerr << "Channel::send(). Attempt " << i << " of " << attempts << " failed." << std::endl;
      
      std::this_thread::sleep_for(delay_ms);
    }
  }
  
  if (i == attempts) {
    std::cerr << "Channel::send(). Maximum attempts reached: exiting." << std::endl;
    std::exit(1);
  }
}
// -----------------------------------------------------------------------------

template <class T> T Channel<T>::receive()
{
  char data[MAX_BUFFER];
  std::string message = "";
  size_t length;
  
  try {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), this->port));
    
    asio::ip::tcp::socket socket = acceptor.accept();
    asio::error_code error;
    length = socket.read_some(asio::buffer(data), error);
    data[MAX_BUFFER - 1] = '\0'; // Reduce buffer overflows chances?
    if (length < MAX_BUFFER) {
      data[length] = '\0'; // Manually required
    }
    message = data;
    
    if (error == asio::error::eof) {
      return 0; // Connection closed cleanly by peer.
    } else if (error) {
      throw asio::system_error(error); // Some other error.
    }
    
    this->view.push_back(message);
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  
  return message;
}
// -----------------------------------------------------------------------------

#endif // CHANNEL_HPP
