// simple.cpp : Simple console based application to query some DNS servers
//
#include <boost/net/resolve.hpp>
#include <iostream>

using namespace std;
using namespace boost;
using namespace boost::net;

void show_message(dns::message & dns_message);

void request( dns::message & dns_message )
{
  boost::asio::io_service      ioservice;

  dns::resolve resolver;

  ip::address SpeakEasyDNS1( ip::address::from_string("64.81.45.2") );
  ip::address SpeakEasyDNS2( ip::address::from_string("216.231.41.2") );
  ip::address TelepacificDNS1( ip::address::from_string("208.57.0.10") );
  ip::address TelepacificDNS2( ip::address::from_string("208.57.0.11") );
  resolver.addServer( SpeakEasyDNS1 );
  resolver.addServer( SpeakEasyDNS2 );
  resolver.addServer( TelepacificDNS1 );
  resolver.addServer( TelepacificDNS2 );

  // Resolve the message and show the results
  show_message( resolver.query(dns_message) );
}

int main(int argc, char* argv[])
{
  dns::message test_a("cnn.com", dns::type_a );
  request( test_a );
/*
  dns::message test_rbl_a("214.63.37.122.sbl-xbl.spamhaus.org", dns::type_a );
  request( test_rbl_a );

  dns::message test_rbl_txt("214.63.37.122.sbl-xbl.spamhaus.org", dns::type_txt );
  request( test_rbl_txt );

  dns::message test_ns("cnn.com", dns::type_ns );
  request( test_ns );

  dns::message test_cname("cnn.com", dns::type_cname );
  request( test_cname );

  dns::message test_soa("cnn.com", dns::type_soa );
  request( test_soa );

  dns::message test_ptr("181.132.57.208.in-addr.arpa.", dns::type_ptr );
  request( test_ptr );

  dns::message test_hinfo("cnn.com", dns::type_hinfo );
  request( test_hinfo );

  dns::message test_mx("mpowercom.com", dns::type_mx );
  request( test_mx );

  dns::message test_txt("mpowercom.com", dns::type_txt );
  request( test_txt );

  // Service Records aren't supported, *yet*
  dns::message test_srv("mpowercom.com", dns::type_srv );
  request( test_srv );

  // Won't do anything ...
  dns::message test_axfr("mpowercom.com", dns::type_axfr );
  request( test_axfr );

  // Won't do anything ...
  dns::message test_all("mpowercom.com", dns::type_all );
  request( test_all );

  // this last one will assert
  dns::message test_none("cnn.com", dns::type_none );
  request( test_none );
*/
  return 0;
}
