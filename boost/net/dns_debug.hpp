//
// dns_debug.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 1998-2006 Andreas Haberstroh (andreas at ibusy dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_NET_DNS_DEBUG_HPP
#define BOOST_NET_DNS_DEBUG_HPP

#include <boost/asio/detail/push_options.hpp>

#include <boost/net/dns.hpp>

namespace boost {
  namespace net {
    namespace dns {

/*!
The debug class offers some quick functions used for debuging DNS records
*/
class debug
{
public:
  /*!
  Returns a string representation of a class_t type.
  \param qclass Class to retrieve the string name of
  \returns  String name of the class
  */
  static const char *get_class_string(const dns::class_t qclass)
  {

	  switch( qclass )
	  {
    case dns::class_in:
		  return "IN";

	  case dns::class_cs:
		  return "CSNET";

	  case dns::class_ch:
		  return "CHAOS";

	  case dns::class_hs:
		  return "Hesiod";

	  case dns::class_all:
		  return "All";

    case dns::class_none:
      return "!CLASS NOT SET!";
      break;
    }

	  return "!INVALID CLASS!";
  }

  /*!
  Returns a string representation of a type_t type.
  \param qtype Type to retrieve the string name of
  \returns  String name of the type
  */
  static const char *get_type_string(const dns::type_t qtype)
  {
	  switch( qtype )
	  {
    case dns::type_a:
		  return "A";

	  case dns::type_ns:
		  return "NS";

	  case dns::type_cname:
		  return "CNAME";

	  case dns::type_soa:
		  return "SOA";

	  case dns::type_ptr:
		  return "PTR";

	  case dns::type_hinfo:
		  return "HINFO";

	  case dns::type_mx:
		  return "MX";

	  case dns::type_txt:
		  return "TXT";

    case dns::type_a6:
      return "AAAA";

    case dns::type_srv:
      return "SRV";

	  case dns::type_axfr:
      return "AXFR";

	  case dns::type_all:
      return "ALL";

	  case dns::type_none:
      return "!TYPE NOT SET!";
    }

	  return "!INVALID TYPE!";
  }

  static void dump_a(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
	    	<< ((dns::a_resource*)Ptr)->domain() << "\t"
			  << ((dns::a_resource*)Ptr)->ttl() << "\t"
			  << get_class_string(((dns::a_resource*)Ptr)->rclass()) << "\t"
			  << get_type_string(((dns::a_resource*)Ptr)->rtype()) << "\t"
			  << ((dns::a_resource*)Ptr)->address().to_string() << endl;
  }

  static void dump_a6(ostream& strm, dns::resource_base_t* Ptr)
  {
    try
    {
    	strm
	      << ((dns::a6_resource*)Ptr)->domain() << "\t"
			  << ((dns::a6_resource*)Ptr)->ttl() << "\t"
			  << get_class_string(((dns::a6_resource*)Ptr)->rclass()) << "\t"
			  << get_type_string(((dns::a6_resource*)Ptr)->rtype()) << "\t"
			  << ((dns::a6_resource*)Ptr)->address().to_string() << endl;
    }
    catch(boost::system::error_code& ec)
    {
      strm << ec.message() << endl;
    }
  }

  static void dump_ns(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
	      << ((dns::ns_resource*)Ptr)->domain() << "\t"
			  << ((dns::ns_resource*)Ptr)->ttl() << "\t"
			  << get_class_string(((dns::ns_resource*)Ptr)->rclass()) << "\t"
			  << get_type_string(((dns::ns_resource*)Ptr)->rtype()) << "\t"
			  << ((dns::ns_resource*)Ptr)->nameserver() << endl;
  }

  static void dump_mx(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
		  << ((dns::mx_resource*)Ptr)->domain() << "\t"
		  << ((dns::mx_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::mx_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::mx_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::mx_resource*)Ptr)->preference() << "\t"
		  << ((dns::mx_resource*)Ptr)->exchange() << endl;
  }

  static void dump_soa(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
      << ((dns::soa_resource*)Ptr)->domain() << "\t"
      << ((dns::soa_resource*)Ptr)->ttl() << "\t"
      << "SOA\t"
      << get_class_string(((dns::soa_resource*)Ptr)->rclass()) << "\t"
      << get_type_string(((dns::soa_resource*)Ptr)->rtype()) << "\t"
      << ((dns::soa_resource*)Ptr)->master_name() << "\t"
      << ((dns::soa_resource*)Ptr)->responsible_name() << "\t"
      << ((dns::soa_resource*)Ptr)->serial_number() << "\t"
      << ((dns::soa_resource*)Ptr)->refresh() << "\t"
      << ((dns::soa_resource*)Ptr)->retry() << "\t"
      << ((dns::soa_resource*)Ptr)->expire() << "\t"
      << ((dns::soa_resource*)Ptr)->minttl() << endl;
  }

  static void dump_cname(ostream& strm, dns::resource_base_t* Ptr)
  {
    strm
		  << ((dns::cname_resource*)Ptr)->domain() << "\t"
		  << ((dns::cname_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::cname_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::cname_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::cname_resource*)Ptr)->canonicalname() << endl;
  }

  static void dump_hinfo(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
		  << ((dns::hinfo_resource*)Ptr)->domain() << "\t"
		  << ((dns::hinfo_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::hinfo_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::hinfo_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::hinfo_resource*)Ptr)->cpu() << "\t"
      << ((dns::hinfo_resource*)Ptr)->os() << endl;
  }

  static void dump_text(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
		  << ((dns::txt_resource*)Ptr)->domain() << "\t"
		  << ((dns::txt_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::txt_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::txt_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::txt_resource*)Ptr)->text() << endl;
  }

  static void dump_ptr(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
		  << ((dns::ptr_resource*)Ptr)->domain() << "\t"
		  << ((dns::ptr_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::ptr_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::ptr_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::ptr_resource*)Ptr)->pointer() << endl;
  }

  static void dump_srv(ostream& strm, dns::resource_base_t* Ptr)
  {
	  strm
		  << ((dns::srv_resource*)Ptr)->domain() << "\t"
		  << ((dns::srv_resource*)Ptr)->ttl() << "\t"
		  << get_class_string(((dns::srv_resource*)Ptr)->rclass()) << "\t"
		  << get_type_string(((dns::srv_resource*)Ptr)->rtype()) << "\t"
		  << ((dns::srv_resource*)Ptr)->priority() << "\t"
		  << ((dns::srv_resource*)Ptr)->weight() << "\t"
      << ((dns::srv_resource*)Ptr)->port() << "\t"
      << ((dns::srv_resource*)Ptr)->targethost() << endl;
  }

  /*!
  Used to dump out a record to a ostream object.
  \param strm Ostream to write to
  \param ptr Resource base object to dump
  */
  static void dump_record(ostream& strm, dns::resource_base_t * ptr )
  {
      switch( ptr->rtype() )
		  {
      case dns::type_a:
			  dump_a(strm, ptr);
			  break;

		  case dns::type_ns:
			  dump_ns(strm, ptr);
			  break;

		  case dns::type_cname:
			  dump_cname(strm, ptr);
			  break;

		  case dns::type_soa:
			  dump_soa(strm, ptr);
			  break;

		  case dns::type_ptr:
			  dump_ptr(strm, ptr);
			  break;

		  case dns::type_hinfo:
			  dump_hinfo(strm, ptr);
			  break;

		  case dns::type_mx:
			  dump_mx(strm, ptr);
			  break;

		  case dns::type_txt:
			  dump_text(strm, ptr);
			  break;

		  case dns::type_a6:
			  dump_a6(strm, ptr);
			  break;

		  case dns::type_srv:
			  dump_srv(strm, ptr);
			  break;

		  default:
			  strm << "Unhandled record type!" << endl;
			  break;
		  }
  }
};

    } // namespace dns
  } // namespace net
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_NET_DNS_DEBUG_HPP
