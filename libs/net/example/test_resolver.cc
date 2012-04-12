// test_resolver.cc
//
#include <vector>
#include <iostream>
using namespace std;

#include <boost/net/dns_resolver.hpp>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

using namespace boost;
using namespace asio;

void wait_cycle(int mins)
{
  cout << "Now Time: " << posix_time::to_simple_string(posix_time::second_clock::local_time()) << endl;
  boost::xtime xt;
  for( int x = 0; x < mins; ++x )
  {
    for( int i = 0; i < 12; ++i )
    {
      cout << "Waiting: " << posix_time::to_simple_string(posix_time::second_clock::local_time()) << endl;
      boost::xtime_get(&xt, boost::TIME_UTC); xt.sec += 5;
      boost::thread::sleep(xt);
    }
    cout << (x+1) << "/" << (mins) << " minutes completed" << endl;
  }
}

void resolve_handle(net::dns::shared_resource_base_t record, const boost::system::error_code& ec)
{ 
  if( record )
    net::dns::debug::dump_record(cout, record.get() );
  else
    cout << ec.message() << endl;
} 

#define WAIT_TIME 0

int main(int argc, char* argv[])
{
  io_service  ios;

  boost::net::dns::dns_resolver dnsResolver(ios); 
  
  dnsResolver.add_nameserver( ip::address::from_string("64.81.45.2") );
  
  boost::net::dns::rr_list_t linfo;

  linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
  linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
  linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " ibusy.com:ns" << endl;
    linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " ibusy.com:mx" << endl;
    linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_mx);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " cnn.com:mx" << endl;
    linfo = dnsResolver.resolve("cnn.com.", net::dns::type_mx);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);
  
  cout << to_simple_string(posix_time::second_clock::local_time()) << " ibusy.com:ns" << endl;
    linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " cnn.com:mx" << endl;
    linfo = dnsResolver.resolve("cnn.com.", net::dns::type_mx);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);
 
  cout << to_simple_string(posix_time::second_clock::local_time()) << " cnn.com:ns" << endl;
    linfo = dnsResolver.resolve("cnn.com.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " cnn.com:ns" << endl;
    linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " ibusy.com:ns" << endl;
    linfo = dnsResolver.resolve("ibusy.com.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " boost.org:ns" << endl;
    linfo = dnsResolver.resolve("boost.org.", net::dns::type_ns);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  cout << to_simple_string(posix_time::second_clock::local_time()) << " boost.org:mx" << endl;
    linfo = dnsResolver.resolve("boost.org.", net::dns::type_mx);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);
  
  cout << to_simple_string(posix_time::second_clock::local_time()) << " www.boost.org:a" << endl;
    linfo = dnsResolver.resolve("www.boost.org.", net::dns::type_a);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);
  
  cout << to_simple_string(posix_time::second_clock::local_time()) << " boost.org:a" << endl;
    linfo = dnsResolver.resolve("boost.org.", net::dns::type_a);
    cout << linfo.size() << endl;
    wait_cycle(WAIT_TIME);

  dnsResolver.async_resolve(
    "ibusy.com.", net::dns::type_ns,
    bind(
      &resolve_handle, _1, _2
    )
  );                            

  ios.run();
  
  cout << "#====#====#====#====#====#====#====#====#====#====#====#====#====#====#====#" << endl;
  boost::net::dns::dns_cache_object::instance().show_cache();

  
  return 0;
}
