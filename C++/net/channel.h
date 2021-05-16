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

#ifndef CHANNEL_H
#define CHANNEL_H
//-----------------------------------------------------------------------------

#include <string>
#include <vector>
#include <chrono>

#define CHANNEL_H_DEFAULT_HOST "localhost"
#define CHANNEL_H_DEFAULT_PORT 10000
#define CHANNEL_H_MAX_BUFFER 65536

namespace net {

using byte = unsigned char;
class Bytes : public std::vector<byte> {
public:
  friend std::ostream& operator<<(std::ostream& stream, const Bytes& data);
};

/**
 * A Channel offers send and receive methods to communicate through the network.
 */
class Channel
{
public:
  /**
   * Constructor of a Channel.
   * 
   * @param port the listening port to open when receiving
   */
  Channel(const int port = CHANNEL_H_DEFAULT_PORT);
  
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
      const Bytes &message
    , const int port
    , const std::string &host = CHANNEL_H_DEFAULT_HOST
    , const unsigned int attempts = 10
    , const std::chrono::duration<int,std::milli> &delay_ms = std::chrono::milliseconds(1000)
  );
  
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
      const std::string &message
    , const int port
    , const std::string &host = CHANNEL_H_DEFAULT_HOST
    , const unsigned int attempts = 10
    , const std::chrono::duration<int,std::milli> &delay_ms = std::chrono::milliseconds(1000)
  );
  
  /**
   * Receive a message.
   * 
   * @param port the listening port to open when receiving
   * @return the message received
   */
  Bytes receive(const int port = 0);
  
  /**
   * Receive a text message.
   * 
   * @param port the listening port to open when receiving
   * @return the message received
   */
  std::string receiveText(const int port = 0);
  
private:
  int port; ///< Listening port for receiving
};
// -----------------------------------------------------------------------------

} // namespace
// -----------------------------------------------------------------------------

#endif // CHANNEL_H
