//
// dns_cache.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2008 Andreas Haberstroh (andreas at ibusy dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_NET_DNS_CACHE_HPP
#define BOOST_NET_DNS_CACHE_HPP

#include <vector>
#include <boost/net/dns.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/thread/mutex.hpp>

using namespace boost::multi_index;
using namespace boost::posix_time;
using namespace std;

namespace boost {
  namespace net {
    namespace dns {

      /*!
      Implementation for a simple DNS cache.
      This cache uses two methods to remove a "stale" record.
      
        #1 - Expiration date
        
        #2 - Hit count
        
      */
      class dns_cache_t
      {
      private:
      
        /*!
        
        */
        struct dns_hasher
        {
        public:
          /*!
          */
          static size_t query(const string& domain, const type_t rType, const class_t rClass = class_in)
          {
            boost::hash<string>            hString;
            boost::hash<type_t>  hType;
            boost::hash<class_t> hClass;
            return size_t(hString(domain) + hType(rType) + hClass(rClass));
          }

          /*!
          */
          static std::size_t query(const request_base_t& dnr)
          {
            return dns_hasher::query(dnr.domain(), dnr.rtype(), dnr.rclass());
          }
          
          /*!
          */
          static std::size_t query(const shared_resource_base_t& dnr)
          {
            return dns_hasher::query(dnr->domain(), dnr->rtype(), dnr->rclass());
          }

          /*!
          */
          static std::size_t record(const shared_resource_base_t& rr)
          {
            std::size_t hashCode(dns_hasher::query(rr));

            boost::hash<uint32_t>     h32;
            boost::hash<string>       hString;

            switch( rr->rtype() )
            {
            case type_a:
              hashCode += h32( ((a_resource*)rr.get())->address().to_ulong() );
              break;
            case type_ns:
              hashCode += hString( ((ns_resource*)rr.get())->nameserver() );
              break;
            case type_cname:
              hashCode += hString( ((cname_resource*)rr.get())->canonicalname() );
              break;
            case type_soa:
              hashCode += h32( ((soa_resource*)rr.get())->serial_number() );
              break;
            case type_ptr:
              hashCode += hString( ((ptr_resource*)rr.get())->pointer() );
              break;
            case type_mx:
              hashCode += hString( ((mx_resource*)rr.get())->exchange() ) + h32( ((mx_resource*)rr.get())->preference() );
              break;

            case type_a6:
            case type_srv:
              break;
              
            case type_none:
            case type_hinfo:
            case type_txt:
            case type_axfr:
            case type_all:
              break;
            
            }

            return hashCode;
          }
        };

        /*!
        */
        struct rr_cache
        {
          size_t      _rHash;
          size_t      _qHash;
          size_t      _dHash;
          uint32_t    _hits;
          time_period _expirationTime;
          ptime       _timeRetrieved;
          bool        _perm;
          
          shared_resource_base_t record;

          /*!
          */
          rr_cache(const shared_resource_base_t& rr, const bool perm)
          : _hits(0),
            _expirationTime(second_clock::local_time(), seconds(rr->ttl())),
            _timeRetrieved(second_clock::local_time()),
            _perm(perm),
            record(rr)
          {
            _rHash = dns_hasher::record(rr);
            _qHash = dns_hasher::query(rr);
            
            boost::hash<string> hString;
            _dHash = hString(record.get()->domain());
          }

          /*!
          */
          virtual ~rr_cache()
          {
          }

          /*!
          */
          bool expired() const
          {
            if( _perm )
              return false;
              
            ptime nowTime = second_clock::local_time();
            return (!_expirationTime.contains(nowTime));
          }
          
          uint32_t hits() const
          {
            if( _perm )
              return 0xFFFFFFFF;

            return _hits;
          }

        };      
        /*!
        */
        typedef shared_ptr<rr_cache>  shared_rr_cache;

        struct by_d{};
        struct by_r{};
        struct by_q{};
        struct by_hits{};
        struct by_ttl{};
#if !defined(GENERATING_DOCUMENTATION)
        /*!
        */
        typedef boost::multi_index::multi_index_container<
          shared_rr_cache,
          boost::multi_index::indexed_by<
            boost::multi_index::hashed_non_unique<
              boost::multi_index::tag<by_d>, 
              boost::multi_index::member<rr_cache, std::size_t, &rr_cache::_dHash>
            >,
            boost::multi_index::hashed_unique<
              boost::multi_index::tag<by_r>, 
              boost::multi_index::member<rr_cache, std::size_t, &rr_cache::_rHash>
            >,
            boost::multi_index::hashed_non_unique<
              boost::multi_index::tag<by_q>, 
              boost::multi_index::member<rr_cache, std::size_t, &rr_cache::_qHash>
            >,
            boost::multi_index::ordered_non_unique< 
              boost::multi_index::tag<by_hits>, 
              boost::multi_index::const_mem_fun<rr_cache, uint32_t, &rr_cache::hits> 
            >,
            boost::multi_index::ordered_non_unique< 
              boost::multi_index::tag<by_ttl>, 
              boost::multi_index::const_mem_fun<rr_cache, bool, &rr_cache::expired> 
            >
          >
        > rr_container_t;
#endif
        typedef rr_container_t::index<by_d>::type::iterator     d_iter_t;
        typedef rr_container_t::index<by_r>::type::iterator     r_iter_t;
        typedef rr_container_t::index<by_q>::type::iterator     q_iter_t;
        typedef rr_container_t::index<by_hits>::type::iterator  hits_iter_t;
        typedef rr_container_t::index<by_ttl>::type::iterator   ttl_iter_t;

        ///
        rr_container_t  _cache;
        ///
        const uint32_t  _max_elements;
        ///
        boost::mutex    _mutex;

      public:
        /*!
        */
        dns_cache_t()
          : _max_elements(16)
        {
        }

        /*!
        */
        bool exists(const question& q)
        {
          boost::mutex::scoped_lock scopeLock(_mutex);
          
          size_t qHash = dns_hasher::query(q);

          q_iter_t rrIter = _cache.get<by_q>().find(qHash);
          if( rrIter == _cache.get<by_q>().end() )
            return false;

          return true;
        }
        
        /*!
        */
        bool exists(const std::string& domain, const type_t rType)
        {
          question q(domain,rType);
          return exists(q);
        }

        /*!
        */
        rr_list_t get(const question& q)
        {
          boost::mutex::scoped_lock scopeLock(_mutex);

          rr_list_t retList; 
        
          std::pair<q_iter_t,q_iter_t> rrIter = _cache.get<by_q>().equal_range(dns_hasher::query(q));    
          while( rrIter.first != rrIter.second )
          {
            (*rrIter.first)->_hits++;
            (*rrIter.first)->_timeRetrieved = second_clock::local_time();
           
            retList.push_back( (*rrIter.first)->record );
            rrIter.first++;
          }
          
          return retList;
        }
        
        /*!
        */
        rr_list_t get(const std::string& domain, const type_t rType)
        {
          question q(domain,rType);
          return get(q);
        }
        
        /*!
        */
        void add(const shared_resource_base_t& rr, const bool perm = false)
        {
          if( _cache.size() > _max_elements )
            reserve(4, *rr.get() );

          boost::mutex::scoped_lock scopeLock(_mutex);
          shared_rr_cache rrItem(new rr_cache(rr,perm));
          _cache.insert(rrItem);
        }

        /*!
        */
        void reserve(const size_t reserve_count, request_base_t& q)
        {
          size_t  cacheSize = _cache.size();
          if( cacheSize < (_max_elements - reserve_count) )
            return ;
          
          expiredCleanup(reserve_count, q);
          cacheSize = _cache.size();
          if( reserve_count < (_max_elements - _cache.size()) )
            return;

          uint32_t  lowMark(0);
          while( reserve_count > (_max_elements - _cache.size()) )
            lowestHitCleanup(reserve_count, q, lowMark++);
        }
          
        /*!
        */
        void show_cache()
        {
          d_iter_t  iter;
          
          for( iter = _cache.get<by_d>().begin(); iter != _cache.get<by_d>().end(); ++iter )
          {
            cout << "+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+" << endl
                 << "          Hits: " << (*iter)->_hits << endl
                 << "   Lifetime at: " << to_simple_string((*iter)->_expirationTime) << endl
                 << "Last Retrieved: " << to_simple_string((*iter)->_timeRetrieved) << endl;
            debug::dump_record(cout, (*iter)->record.get() );
          }
        }
        
      private:
        /*!
        */
        int expiredCleanup(size_t reserve_count, request_base_t& q)
        {
          boost::mutex::scoped_lock scopeLock(_mutex);
          int count(0);
          std::pair<ttl_iter_t,ttl_iter_t> range_iter;
          for( 
            range_iter = _cache.get<by_ttl>().equal_range(true);
            range_iter.first != range_iter.second;
            ++range_iter.first, ++count )
          {
            if( q.domain() != (*range_iter.first)->record->domain() )
            {
              _cache.get<by_ttl>().erase(range_iter.first);
              if( --reserve_count == 0 )
                break;
            }
          }
          
          return count;
        }
          
        /*!
        */
        uint32_t lowestHitCleanup(size_t reserve_count, request_base_t& q, uint32_t lowMark = 0)
        {
          boost::mutex::scoped_lock scopeLock(_mutex);
          int count(0);
          
          std::pair<hits_iter_t,hits_iter_t> range_iter;
          for( 
            range_iter = _cache.get<by_hits>().equal_range(lowMark);
            range_iter.first != range_iter.second;
            ++range_iter.first, ++count )
          {
            if( q.domain() != (*range_iter.first)->record->domain() )
            {
              _cache.get<by_hits>().erase(range_iter.first);
              if( --reserve_count == 0 )
                break;
            }
          }
          
          return count;
        }
      };

    } // namespace dns
  } // namespace net
} // namespace boost

#endif  // BOOST_NET_DNS_CACHE_HPP
