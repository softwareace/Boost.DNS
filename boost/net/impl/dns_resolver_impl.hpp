//
// dns_resolver_impl.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2008 Andreas Haberstroh (andreas at ibusy dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_NET_DNS_RESOLVER_IMPL_HPP
#define BOOST_NET_DNS_RESOLVER_IMPL_HPP

#include <vector>

#include <boost/net/dns_cache.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/random.hpp>
#include <boost/thread/detail/singleton.hpp>

using namespace boost::multi_index;

namespace boost {
  namespace net {
    namespace dns {

/// Single instance of the dns cache
typedef boost::detail::thread::singleton<net::dns::dns_cache_t> dns_cache_object;

class dns_resolver_impl 
{
private:
  /// Outbound DNS request buffer 
  typedef vector<ip::udp::endpoint> ep_vector_t;

  class dns_handler_base
  {
  public:
    dns_handler_base()
    {
    }
    
    virtual ~dns_handler_base()
    {
    }

    virtual void invoke(io_service& ios, const shared_resource_base_t& record, const boost::system::error_code& ec)
    {
    }
  };
  
  typedef shared_ptr<dns_handler_base>  dns_handler_base_t;

  /// Handler to wrap asynchronous callback function
  template <typename Handler>
  class dns_handler : public dns_handler_base
  {
  public:
    dns_handler(Handler h)
    : dns_handler_base(),
      handler_(h)
    {
    }

    virtual ~dns_handler()
    {
    }

    virtual void invoke(io_service& ios, const shared_resource_base_t& record, const boost::system::error_code& ec)
    {
      ios.post( 
        boost::asio::detail::bind_handler(handler_, record, ec)
      )
      ;
    }

  private:
    Handler handler_;
  };

  /*!
  DNS Query structure  
  */
  struct dns_query_t
  {
    dns_query_t(const net::dns::question& q)
    : _query_start(posix_time::second_clock::local_time(), posix_time::seconds(30)),
      _query_sent(posix_time::second_clock::local_time(), posix_time::seconds(2))
    {
      _question = q;
//      _qbuffer = shared_ptr<net::network_buffer_t>(new net::network_buffer_t(_mbuffer.data(), _mbuffer.size()));
    }
    
    dns_query_t(const dns_query_t& o)
    : _query_start(posix_time::time_period(o._query_start.begin(), o._query_start.last())),
      _query_sent(posix_time::time_period(o._query_sent.begin(), o._query_sent.last()))
    {
      operator=(o);
    }

    virtual ~dns_query_t()
    {
    }
    
    const dns_query_t& operator=(const dns_query_t& o)
    {
      _question_id = o._question_id;
      _dns = o._dns;
      _mbuffer = o._mbuffer;
  //    _qbuffer = o._qbuffer;
      _question = o._question;
      _completion_callback = o._completion_callback;
      _query_start = posix_time::time_period(o._query_start.begin(), o._query_start.last());
      _query_sent = posix_time::time_period(o._query_sent.begin(), o._query_sent.last());
      return *this;
    }

    /// Question ID
    uint16_t        _question_id;
    
    /// Domain Name Server Address to send request to
    ip::udp::endpoint _dns;

    /// DNS Query Buffer
    dns_buffer_t          _mbuffer;
//    shared_dns_buffer_t   _qbuffer;

    /// DNS Query question
    net::dns::question                _question;
    
    /// DNS Completion handler
    dns_handler_base_t                _completion_callback;
    
    /// Time period the request is good for
    posix_time::time_period     _query_start;
            
    /// Time period which the last request was sent
    posix_time::time_period     _query_sent;
    
    bool operator<( const uint16_t& id ) const 
    { 
      return _question_id < id; 
    }
    
    bool expired() const
    {
      posix_time::ptime nowTime = posix_time::second_clock::local_time();
      return !_query_start.contains(nowTime);
    }

    bool resend() const
    {
      posix_time::ptime nowTime = posix_time::second_clock::local_time();
      return _query_sent.contains(nowTime);
    }
  };

  typedef shared_ptr<dns_query_t>   shared_dq_t;

  struct by_question_id{};
  struct by_expired{};
  struct by_resend{};
#if !defined(GENERATING_DOCUMENTATION)
  typedef 
  multi_index_container<
    shared_dq_t,
    indexed_by<
      ordered_non_unique< 
        tag<by_resend>,
        const_mem_fun<dns_query_t, bool, &dns_query_t::resend> 
      >,
      ordered_non_unique< 
        tag<by_expired>,
        const_mem_fun<dns_query_t, bool, &dns_query_t::expired> 
      >,
      ordered_non_unique< 
        tag<by_question_id>, 
        member<dns_query_t, uint16_t, &dns_query_t::_question_id> 
      >
    >
  > query_container_t;
#endif
  typedef query_container_t::index<by_question_id>::type::iterator  question_id_iterator_t;
  typedef query_container_t::index<by_expired>::type::iterator      expired_iterator_t;
  typedef query_container_t::index<by_resend>::type::iterator       resend_iterator_t;


  io_service&       _ios;
  deadline_timer    _timer;
  ip::udp::socket   _socket;
  ep_vector_t       _dnsList;
  query_container_t _query_list;
  boost::mutex      _resolver_mutex;
  mutable bool      _outstanding_read;
  boost::mt19937    _rng;

public: 
  dns_resolver_impl(io_service& ios)
    : _ios(ios),
      _timer(_ios),  
      _socket(_ios),
      _outstanding_read(false)
  {
  }
    
	void destroy() 
	{ 
	}

  void add_nameserver(ip::address addr)
  {
    ip::udp::endpoint endpoint(addr, 53);
    _dnsList.push_back(endpoint);
  }
  
  void cancel()
  {
    _socket.cancel();
  }

  template<typename CallbackHandler>
  void async_resolve(const net::dns::question & question, CallbackHandler handler)
  {
    if( dns_cache_object::instance().exists(question) )
    {
      boost::system::error_code callbackError;
      dns_handler<CallbackHandler>  caller(handler);
    
      rr_list_t record_list = dns_cache_object::instance().get(question);
      for( rr_list_t::iterator iter = record_list.begin();
            iter != record_list.end();
            ++iter )
      {
        caller.invoke(_ios, (*iter), callbackError );
      }
    }
    else
    {
      boost::mutex::scoped_lock scopeLock(_resolver_mutex);
      if( !_socket.is_open() )
      {
        _socket.open(ip::udp::v4());
        _socket.bind(ip::udp::endpoint(ip::udp::v4(), 0));
      }
      
      _timer.expires_from_now(posix_time::seconds(2));
      _timer.async_wait(
        boost::bind(
          &dns_resolver_impl::handle_timeout, 
          this,
          boost::asio::placeholders::error
        )
      );
      
      net::dns::message  qmessage(question);

      // set a few defaults for a message
      qmessage.recursive(true);
      qmessage.action(net::dns::message::query);
      qmessage.opcode(net::dns::message::squery);

      // make our message id unique
      uint16_t quid( (uint16_t)_rng() );
      
      for( ep_vector_t::iterator iter = _dnsList.begin(); iter != _dnsList.end(); ++iter)
      {
        shared_dq_t dq = shared_dq_t(new dns_query_t(question));

        dq->_question_id = (uint16_t)quid;
        dq->_dns = *iter;
        dq->_completion_callback = shared_ptr<dns_handler<CallbackHandler> >(new dns_handler<CallbackHandler>(handler));

        qmessage.id( dq->_question_id );
        qmessage.encode( dq->_mbuffer );

        _query_list.insert(dq);
        send_request(dq);
      }
    }
  }

  template<typename CallbackHandler>
  void async_resolve(const string & domain, const net::dns::type_t rrtype, CallbackHandler handler)
  {
    net::dns::question question(domain, rrtype);
    async_resolve(question, handler);
  }

  boost::asio::io_service & get_io_service()
  {
    return _ios;
  }

  rr_list_t resolve(const net::dns::question & question, boost::system::error_code & ec)
  {
    rr_list_t _list;
    return _list;
  }
  
  rr_list_t resolve(const net::dns::question & question)
  {
    shared_rr_list_t _list(new rr_list_t);
    
    io_service        thisIos;
    dns_resolver_impl thisResolve(thisIos);
    thisResolve._dnsList = _dnsList;

    thisResolve.async_resolve(
      question, 
      bind(
        &dns_resolver_impl::blocking_callback, 
        &thisResolve, 
        _list, _1, _2
      )
    );
    
    thisIos.run();
    
    return *_list.get();
  }
  
  rr_list_t resolve(const string & domain, const net::dns::type_t rrtype)
  {
    net::dns::question question(domain, rrtype);
    return resolve(question);
  }

  rr_list_t resolve(const string & domain, const net::dns::type_t rrtype, boost::system::error_code & ec)
  {
    rr_list_t _list;
    return _list;
  }
  
private:
  void send()
  {
    boost::mutex::scoped_lock scopeLock(_resolver_mutex);
    
    question_id_iterator_t iter;
    for( iter = _query_list.get<by_question_id>().begin(); iter != _query_list.get<by_question_id>().end(); ++iter)
    {
      _ios.post(
        bind(
          &dns_resolver_impl::send_request, 
          this,
          (*iter)
        )
      );
    }
  }

  void send_request(shared_dq_t& dq)
  {
//    cout << "send_request: " << dq->_dns.address().to_string() << endl;
    _socket.async_send_to(
      boost::asio::buffer(
        dq->_mbuffer.data(), 
        dq->_mbuffer.length()
        ), 
      dq->_dns,
      boost::bind(
        &dns_resolver_impl::handle_send, 
        this, 
        dq,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred
        )
      );
  }
  
  void handle_send(shared_dq_t& dq, const boost::system::error_code& ec, size_t bytes_sent)
  {
    if (!ec || ec == boost::asio::error::message_size)
    {
      shared_dns_buffer_t rbuffer(new dns_buffer_t );
      
      _socket.async_receive(
        boost::asio::buffer( 
          *(rbuffer.get()) 
        ),
        boost::bind(
          &dns_resolver_impl::handle_recv, 
          this, 
          rbuffer,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred
        )
      );
    }
  }

  void handle_recv(shared_dns_buffer_t  inBuffer, const boost::system::error_code& ec, std::size_t bytes_transferred)
  {
    boost::mutex::scoped_lock scopeLock(_resolver_mutex);
    _outstanding_read = false;
    
    if( !ec && bytes_transferred ) // || ec == boost::asio::error::message_size)
    {
      inBuffer.get()->length(bytes_transferred);

      net::dns::message  tmpMessage;
      
      uint16_t  qid;
      inBuffer.get()->get(qid);
      
      std::pair<question_id_iterator_t,question_id_iterator_t> range_iter;
      range_iter = _query_list.get<by_question_id>().equal_range(qid);
      if( range_iter.first == range_iter.second )
        return ;
      
      question_id_iterator_t qiter = range_iter.first; 
      
      tmpMessage.decode( *inBuffer.get() );
      boost::system::error_code callbackError;
      if( tmpMessage.result() != net::dns::message::noerror )
      {
        callbackError = error::not_found;
        shared_resource_base_t record;
        (*qiter)->_completion_callback->invoke(_ios, record, callbackError);
      }
      else
      {
        net::dns::rr_list_t* records;
        net::dns::rr_list_t::iterator  iter;

        // Grab all the records, we'll probably need more info for additional queries
        if( tmpMessage.additionals()->size() )
        {
          records = tmpMessage.additionals();
          dns_cache_object::instance().reserve( records->size(), (*qiter)->_question );
          for( iter = records->begin(); iter != records->end(); iter++ )
            dns_cache_object::instance().add( (*iter) );
        }
      
        if( tmpMessage.authorites()->size() )
        {
          records = tmpMessage.authorites();
          dns_cache_object::instance().reserve( records->size(), (*qiter)->_question );
          for( iter = records->begin(); iter != records->end(); iter++ )
            dns_cache_object::instance().add( (*iter) );
        }
      
        if( tmpMessage.answers()->size() )
        {
          records = tmpMessage.answers();
          dns_cache_object::instance().reserve( records->size(), (*qiter)->_question );
          for( iter = records->begin(); iter != records->end(); iter++ )
          {
            dns_cache_object::instance().add( (*iter) );
            (*qiter)->_completion_callback->invoke(_ios, (*iter), callbackError);
          }
        }
      }

      _query_list.get<by_question_id>().erase(range_iter.first, range_iter.second);  
      if( !_query_list.size() )
      {
        _timer.cancel();
        _socket.close();
      }
    }
    else if (ec == error::operation_aborted)
    {
      inBuffer.get()->length(bytes_transferred);
      
      uint16_t  qid;
      inBuffer.get()->get(qid);
      
      std::pair<question_id_iterator_t,question_id_iterator_t> range_iter;
      range_iter = _query_list.get<by_question_id>().equal_range(qid);

      if( range_iter.first != range_iter.second )
      {
        shared_resource_base_t  record;
        (*range_iter.first)->_completion_callback->invoke(_ios, record, error::operation_aborted);
      }
      
      _query_list.get<by_question_id>().erase(range_iter.first, range_iter.second);  
      if( !_query_list.size() )
      {
        _timer.cancel();
        _socket.close();
      }
    }    
    else
    {
    }
  }

  void handle_timeout(const boost::system::error_code& ec)
  {
    if( !ec && ec != error::operation_aborted )
    {
      // Lock the list so we can manipulate it
      boost::mutex::scoped_lock scopeLock(_resolver_mutex);
      
      if( _query_list.size() )
      {
        shared_resource_base_t  record;
        
        expired_iterator_t  expired_iter = _query_list.get<by_expired>().find(true);
        for( ; expired_iter != _query_list.get<by_expired>().end(); ++expired_iter )
        {
          if( (*expired_iter)->expired() )
          {
            (*expired_iter)->_completion_callback->invoke(_ios, record, error::timed_out);
            _query_list.get<by_expired>().erase(expired_iter);
          }
        }
      }
      
      if( _query_list.size() )
      {
        _timer.expires_from_now(posix_time::seconds(2));
        _timer.async_wait(
          boost::bind(
            &dns_resolver_impl::handle_timeout, 
            this,
            boost::asio::placeholders::error
          )
        );
        
        _ios.post( bind(&dns_resolver_impl::send, this) );
      }
      else
      {
        _socket.close();
      }
    }
  }

  void blocking_callback(shared_rr_list_t& list, const shared_resource_base_t& record, const boost::system::error_code& ec)
  {
    // we're doing a asynch operation for a sync request
    if( record )
    {
      list->push_back(record);
    }
    else
      cout << "blocking_callback: " << ec.message() << endl;
  }
};

    } // namespace dns
  } // namespace net
} // namespace boost

#endif  // BOOST_NET_DNS_RESOLVER_IMPL_HPP
