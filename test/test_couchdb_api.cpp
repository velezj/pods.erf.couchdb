
#include <erf-couchdb/couchdb.hpp>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>

using namespace boost;
using namespace boost::network;
using namespace boost::property_tree;
using namespace couchdb;

int main( int argn, char** argv )
{

  // create a new couchdb instance
  Couchdb couch( uri::uri("http://localhost:5984/test_cpp_api/") );
  
  // store a document and then retrieve it
  ptree doc;
  doc.put( "a.b.c.d", "hehehe!" );
  ptree save_response = couch.save( doc, std::string("test_doc_0001") );

  std::cout << "SAVE response: " << std::endl;
  json_parser::write_json( std::cout, save_response );
  std::cout << std::endl;
  
  // retrieve the document
  ptree fetched_doc;
  fetched_doc = couch.fetch( uri::uri("test_doc_0001") );
  std::cout << "FETCH doc: " << std::endl;
  json_parser::write_json( std::cout, fetched_doc );
  std::cout << std::endl;
  

  return 0;
}
