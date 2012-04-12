//
// resolve.hpp
// ~~~~~~~~
//
// Copyright (c) 1998-2006 Andreas Haberstroh (andreas at ibusy dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_NET_DNS_RESOLVE_HPP
#define BOOST_NET_DNS_RESOLVE_HPP

#include <boost/asio/detail/push_options.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/net/dns.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace boost {
  namespace net {
    namespace dns {

/// Provides DNS resolution using dns::message packets
/**
The resolve class makes a DNS resolution query synchronously.
Internally, the class does the work asynchronously when multiple servers are listed.
*/
class resolve
{
private:

  /*
  need an atomic reference counter
  */
  class reference_counter
  {
  public:
      reference_counter() : mutex(), count(0) { }
      int inc()
      {
          boost::mutex::scoped_lock scoped_lock(mutex);
          return ++count;
      }
      int dec()
      {
          boost::mutex::scoped_lock scoped_lock(mutex);
          return --count;
      }

  private:
      boost::mutex mutex;
      mutable int count;
  };

private:
  /// resolve has it's own io_service handler
  io_service          ioservice;
  ip::udp::socket     socket;

  /// Timer used to retry sending a packet. Busy servers don't always anwers
  deadline_timer      retry_timer;

  /// Timer used to kill the async_dns_request if there isn't a response
  deadline_timer      death_timer;

  /// The maximum length async_dns_request should try to get a response.
  uint32_t            death_timeout;

  /// List of servers to request resolution from
  vector<ip::udp::endpoint> endpointList;

  /// Response message to pass back when a request is resolved
  dns::message        responseMessage;

  /// Outbound DNS request buffer
  dns_buffer_t  reqBuffer;

  /// Number of servers left before
  reference_counter   requestCount;

  /// General mutex for locking buffers
  boost::mutex        bufferMutex;

public:
  /// Constructor for the resolve class
  resolve()
  : ioservice(),
    socket(ioservice, ip::udp::endpoint(ip::udp::v4(), 0)),
    retry_timer(ioservice),
    death_timer(ioservice),
    death_timeout(30),
    endpointList(),
    responseMessage(),
    reqBuffer(),
    requestCount(),
    bufferMutex()
  {
  }

  /// Adds a name server to the list of servers that we will query
  /**
  @param addr IP address for the name server
  */
  void addServer( ip::address & addr )
  {
    boost::mutex::scoped_lock lock(bufferMutex);
    ip::udp::endpoint endpoint(addr, 53);
    endpointList.push_back( endpoint );
  }

  /// Executes the DNS query
  /**
  @param qmessage Query message to send to the server list
  @return Query result. If the query failed or timed out, dns::message.result() will return no_result.
  */
  dns::message & query(dns::message & qmessage)
  {
    // set a few defaults for a message
    qmessage.recursive(true);
    qmessage.action(dns::message::query);
    qmessage.opcode(dns::message::squery);

    // make our message id unique
    qmessage.id( 0xaffe );

    qmessage.encode( reqBuffer );

    // in the event nothing get's resolved, answer with the original question
    responseMessage = qmessage;
    responseMessage.result(dns::message::no_result);

    // die if we don't get a response within death_timeout
    death_timer.expires_from_now(boost::posix_time::seconds(death_timeout));
    death_timer.async_wait(boost::bind(&resolve::request_timeout, this));

    for( vector<ip::udp::endpoint>::iterator iter = endpointList.begin(); iter != endpointList.end(); ++iter )
    {
      // we're waiting for N of requests
      requestCount.inc();

      // setup the receive buffer
      shared_dns_buffer_t recvBuffer( new dns_buffer_t );
      socket.async_receive_from(
          boost::asio::buffer( *recvBuffer.get() ), *iter,
          boost::bind(&resolve::handle_recv, this,
            recvBuffer,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    // kick off the request
    send_packet();

    // run a blocking service
    ioservice.run();

    return responseMessage;
  }

private:
  void request_timeout()
  {
    retry_timer.cancel();
    death_timer.cancel();

    // just tear everything down at this point.
    if( socket.is_open() )
      socket.close();

    if( requestCount.dec() == 0 )
      ioservice.stop();
  }

  void resend_timeout()
  {
    retry_timer.cancel();

    // No response, so resend the message again
    if( socket.is_open() )
      send_packet();
    else
      // if the socket gets closed, then, we've probably received an answer!
      request_timeout();
  }

  void send_packet()
  {
    // 2 seconds should be a good time to wait for a response.
    retry_timer.expires_from_now(boost::posix_time::seconds(2));
    retry_timer.async_wait(boost::bind(&resolve::resend_timeout, this));

    // send out the packets for request
    for( vector<ip::udp::endpoint>::iterator iter = endpointList.begin(); iter != endpointList.end(); ++iter )
      socket.send_to(boost::asio::buffer(reqBuffer), *iter);
  }

  void handle_recv(shared_dns_buffer_t inBuffer, const boost::system::error_code& ec, std::size_t bytes_transferred)
  {
    if (!ec || ec == boost::asio::error::message_size)
    {
      // only decode if we haven't decoded already!
      if( responseMessage.result() != dns::message::no_result )
      {
        boost::mutex::scoped_lock lock(bufferMutex);

        // clamp the recvBuffer with the number of bytes transferred
        inBuffer.get()->length(bytes_transferred);

        // decode the buffer and say we've succeeded
        responseMessage.decode( *inBuffer.get() );
      }

      // consider the job done.
      request_timeout();
    }
  }
};

      } // namespace dns
  } // namespace net
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_NET_DNS_RESOLVE_HPP
