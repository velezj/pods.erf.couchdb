
#include <erf-couchdb/couchdb.hpp>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>

using namespace boost;
using namespace boost::property_tree;
using namespace couchdb;

int main( int argn, char** argv )
{

  try {
    
    // create a new couchdb instance
    Couchdb couch( "http://localhost:5984/test_cpp_api/" );
    
    // store a document and then retrieve it
    ptree doc;
    doc.put( "a.b.c.d", "hehehe!" );
    ptree save_response = couch.save( doc, std::string("test_doc_0001") );
    
    std::cout << "SAVE response: " << std::endl;
    json_parser::write_json( std::cout, save_response );
    std::cout << std::endl;
    
    // retrieve the document
    ptree fetched_doc;
    fetched_doc = couch.fetch( "test_doc_0001" );
    std::cout << "FETCH doc: " << std::endl;
    json_parser::write_json( std::cout, fetched_doc );
    std::cout << std::endl;
    
    // save another document with a generated uuid
    doc.put( "0.1.2.3.4", "hahahahah!" );
    save_response = couch.save( doc );
    std::cout << "SAVE response: " << std::endl;
    json_parser::write_json( std::cout, save_response );
    std::cout << std::endl;
    
    // retrieve the document
    fetched_doc = couch.fetch( save_response.get<std::string>("id") );
    std::cout << "FETCH doc: " << std::endl;
    json_parser::write_json( std::cout, fetched_doc );
    std::cout << std::endl;
    
    
    // try to update the first document by adding some stuff
    std::vector< std::pair<std::string,std::string> > updates;
    updates.push_back( std::pair<std::string,std::string>( "x.z", "bwahahaha!" ) );
    ptree update_response;
    update_response = couch.try_update( "test_doc_0001", updates );
    std::cout << "UPDATE response: " << std::endl;
    json_parser::write_json( std::cout, update_response );
    std::cout << std::endl;
    
    // ensure hte substrcuture of the first document
    ptree structure;
    structure.put( "5.4.3.2.1", ":-)" );
    structure.put( "5.a.3.2.1", ":)" );
    update_response = couch.try_ensure_substructure( "test_doc_0001",
						     structure );
    std::cout << "ENSURE response: " << std::endl;
    json_parser::write_json( std::cout, update_response );
    std::cout << std::endl;
    
    
    // ensure hte substrcuture of a non-existant document
    update_response = couch.try_ensure_substructure( "test_doc_0002",
						     structure );
    std::cout << "ENSURE response: " << std::endl;
    json_parser::write_json( std::cout, update_response );
    std::cout << std::endl;
    
  } catch (boost::exception& be ) {
    std::cerr << "DIAGNOSTICS: " << std::endl;
    std::cerr << boost::diagnostic_information( be );
    std::cerr << std::endl;
    throw;
  } catch (std::exception& e ) {
    std::cerr << "DIAGNOSTICS: " << std::endl;
    std::cerr << boost::diagnostic_information( e );
    std::cerr << std::endl;
    throw;
  }

  return 0;
}
