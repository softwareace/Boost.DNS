/*
 basic_dns_resolver.hpp
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 Copyright (c) 2008 - 2012 Andreas Haberstroh
 (andreas at ibusy dot com)
 (softwareace01 at google dot com)
 (softwareace at yahoo dot com)

 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_NET_BASIC_DNS_RESOLVER_HPP
#define BOOST_NET_BASIC_DNS_RESOLVER_HPP

namespace boost
{
  namespace net
  {
    namespace dns
    {

      template<typename Service>
        class basic_dns_resolver : public boost::asio::basic_io_object< Service >
        {
        public:
          explicit
          basic_dns_resolver ( boost::asio::io_service &io_service ) :
            boost::asio::basic_io_object< Service >(io_service)
          {
          }

          void
          add_nameserver ( ip::address addr )
          {
            this->service.add_nameserver(this->implementation, addr);
          }

          template<typename CallbackHandler>
            void
            async_resolve ( const net::dns::question & question, CallbackHandler handler )
            {
              this->service.async_resolve(this->implementation, question, handler);
            }

          template<typename CallbackHandler>
            void
            async_resolve ( const string & domain, const net::dns::type_t rrtype, CallbackHandler handler )
            {
              this->service.async_resolve(this->implementation, domain, rrtype, handler);
            }

          rr_list_t
          resolve ( const net::dns::question & question )
          {
            return this->service.resolve(this->implementation, question);
          }

          rr_list_t
          resolve ( const string & domain, const net::dns::type_t rrtype )
          {
            return this->service.resolve(this->implementation, domain, rrtype);
          }
        };

#if !defined(GENERATING_DOCUMENTATION)
      typedef basic_dns_resolver< basic_dns_resolver_service< > > dns_resolver;
#endif

    } // namespace dns
  } // namespace net
} // namespace boost

#endif // BOOST_NET_BASIC_DNS_RESOLVER_HPP
