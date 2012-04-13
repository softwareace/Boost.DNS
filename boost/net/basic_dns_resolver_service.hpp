/*
 basic_dns_resolver_service.hpp
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 Copyright (c) 2008 - 2012 Andreas Haberstroh
 (andreas at ibusy dot com)
 (softwareace01 at google dot com)
 (softwareace at yahoo dot com)

 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_NET_BASIC_DNS_RESOLVER_SERVICE_HPP
#define BOOST_NET_BASIC_DNS_RESOLVER_SERVICE_HPP

#include <boost/scoped_ptr.hpp>
#include <boost/net/dns.hpp>
#include <boost/thread/thread.hpp>

namespace boost
{
  namespace net
  {
    namespace dns
    {

      template<typename DnsResolverImplementation = dns_resolver_impl>
        class basic_dns_resolver_service : public boost::asio::io_service::service
        {
        public:
          static boost::asio::io_service::id id;

          explicit
          basic_dns_resolver_service ( boost::asio::io_service &io_service ) :
            boost::asio::io_service::service(io_service), work_(new boost::asio::io_service::work(work_io_service_)),
                work_thread_(boost::bind(&boost::asio::io_service::run, &work_io_service_))
          {
          }

          virtual
          ~basic_dns_resolver_service ()
          {
            work_.reset();
            work_io_service_.stop();
            work_thread_.join();
          }

          typedef boost::shared_ptr< DnsResolverImplementation > implementation_type;

          void
          construct ( implementation_type &impl )
          {
            impl.reset(new DnsResolverImplementation(this->get_io_service()));
          }

          void
          destroy ( implementation_type &impl )
          {
            impl->destroy();
            impl.reset();
          }

          rr_list_t
          resolve ( implementation_type &impl, const net::dns::question & question )
          {
            return impl->resolve(question);
          }

          rr_list_t
          resolve ( implementation_type &impl, const string & domain, const net::dns::type_t rrtype )
          {
            return impl->resolve(domain, rrtype);
          }

          template<typename CallbackHandler>
            void
            async_resolve ( implementation_type &impl, const net::dns::question & question, CallbackHandler handler )
            {
              impl->async_resolve(question, handler);
            }

          template<typename CallbackHandler>
            void
            async_resolve (
                implementation_type &impl,
                const string & domain,
                const net::dns::type_t rrtype,
                CallbackHandler handler )
            {
              net::dns::question question(domain, rrtype);
              impl->async_resolve(question, handler);
            }

          void
          add_nameserver ( implementation_type &impl, ip::address addr )
          {
            impl->add_nameserver(addr);
          }

        private:
          void
          shutdown_service ()
          {
          }

          boost::asio::io_service work_io_service_;
          boost::scoped_ptr< boost::asio::io_service::work > work_;
          boost::thread work_thread_;
        };
#if !defined(GENERATING_DOCUMENTATION)
      template<typename DnsResolverImplementation>
        boost::asio::io_service::id basic_dns_resolver_service< DnsResolverImplementation >::id;
#endif
    } // namespace dns
  } // namespace net
} // namespace boost

#endif // BOOST_NET_BASIC_DNS_RESOLVER_SERVICE_HPP
