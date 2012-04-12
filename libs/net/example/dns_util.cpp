// util.cpp : Outputs records to 'cout'
//
#include <iostream>
#include <boost/net/dns.hpp>
#include <boost/net/dns_debug.hpp>

using namespace std;
using namespace boost;
using namespace boost::net;

void show_message(dns::message & dns_packet)
{
	// print out the header info
  cout << "; ASIO::DNS Test Application" << endl;
	cout << "; received answer" << endl;
	cout << "; opcode: " ;
	switch( dns_packet.opcode() )
	{
  case dns::message::squery:
		cout << "QUERY, ";
		break;

	case dns::message::iquery:
		cout << "INVERSERVE QUERY, ";
		break;

	case dns::message::status:
		cout << "STATUS, ";
		break;

  case dns::message::no_opcode:
    cout << "!NO OPCODE!, ";
    break;
	}

	cout << "status: ";
	switch( dns_packet.result() )
	{
	case dns::message::noerror:
		cout << "noerror, ";
		break;

	case dns::message::format_error:
		cout << "format error, ";
		break;

	case dns::message::server_error:
		cout << "server error, ";
		break;

	case dns::message::name_error:
		cout << "name error, ";
		break;

	case dns::message::not_implemented:
		cout << "not implemented, ";
		break;

	case dns::message::refused:
		cout << "refused, ";
		break;
	default:
		cout << "unknown result, ";
		break;
	}

	cout << "id: " << dns_packet.id() << endl;

	cout << "; flags: [";
	if( dns_packet.is_authority() )
		cout << " AA";

	if( dns_packet.is_truncated() )
		cout << " TC";
		
	if( dns_packet.is_recursive() )
		cout << " RD";

	if( dns_packet.is_recursion_avail() )
		cout << " RA";

  dns::message::questions_t* questions = dns_packet.questions();
  dns::rr_list_t* answers = dns_packet.answers();
  dns::rr_list_t* authorites = dns_packet.authorites();
  dns::rr_list_t* additionals = dns_packet.additionals();

  cout << " ], QUERY: " << questions->size() << 
			", ANSWER: " << answers->size() << 
			", AUTHORITY: " << authorites->size() << 
			", ADDITIONAL: " << additionals->size() << 
			endl;


	// reiterate the question to the user
	cout << endl << ";; QUESTION SECTION:" << endl;

  for( dns::message::questions_t::iterator qiter = questions->begin(); qiter != questions->end(); ++qiter )
	  cout	<< "; " 
      << (*qiter).domain() << "\t" 
      << dns::debug::get_class_string((*qiter).rclass()) << "\t" 
      << dns::debug::get_type_string((*qiter).rtype())
      << endl;

	
  dns::rr_list_t::iterator  riter;

	// process the answers. This is directly related to the question!
	if( answers->size() )
  {
		cout << endl << ";;ANSWER SECTION:" << endl;
    for( riter = answers->begin(); riter != answers->end(); ++riter )
      dns::debug::dump_record(cout, (*riter).get() );
  }

	// now, dump the authority section, if there is one.
	if( authorites->size() )
  {
		cout << endl << ";; AUTHORITY SECTION:" << endl;
    for( riter = authorites->begin(); riter != authorites->end(); ++riter )
      dns::debug::dump_record(cout, (*riter).get() );
  }
	
  // and lastly, dump the additional information section, if there is one.
	if( additionals->size() )
  {
  	cout << endl << ";; ADDITIONAL SECTION:" << endl;
    for( riter = additionals->begin(); riter != additionals->end(); ++riter )
      dns::debug::dump_record(cout, (*riter).get() );
  }

  cout << endl << endl ;
}
