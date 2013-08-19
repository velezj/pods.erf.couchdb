
#include "couchdb.hpp"
#include <boost/network/uri.hpp>
#include <boost/network/protocol/http.hpp>
#include <boost/network/protocol/http/client.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <sstream>



namespace couchdb {

    using namespace boost::network;
    using namespace boost::property_tree;
    using namespace boost;


  //========================================================================
  
  // Description:
  // The client to use (sync simple http requests
  typedef http::basic_client<http::tags::http_default_8bit_tcp_resolve, 1, 1> client;


    //========================================================================

    Couchdb::Couchdb( const uri::uri& database_uri )
      : _couchdb_database_uri( database_uri )
    {}

    //========================================================================

    ptree Couchdb::save( const ptree& doc,
			 const optional<std::string>& id ) const
    {

      // create an http client to eventually POST to the couchdb uri
      client c;

      // generate an id if none given
      std::string the_id;
      if( id ) {
	the_id = id.get();
      } else {
	uuids::uuid u = uuids::random_generator()();
	the_id = to_string(u);
      }

      // debug
      //std::cout << "Couchdb::save using id: " << the_id << std::endl;

      // convert ptree to json
      std::ostringstream oss;
      json_parser::write_json( oss, doc );
      std::string json_doc = oss.str();

      // debug
      //std::cout << "Couchdb::save json_doc=" << json_doc << std::endl;
      
      // ok, now POST to the couchdb url the document
      uri::uri post_uri = this->_couchdb_database_uri;
      post_uri.append( the_id );
      client::request request( post_uri );
      //request << header( "Referer", "localhost" );
      request << header( "Connection", "close" );
      
      client::response response;
      try {
	response = c.put(request, json_doc, std::string("application/json"));
      } catch (boost::exception& e ) {
	e << couchdb_request_uri_error_info( post_uri );
	throw;
      }
      
      // debug
      //std::cout << "Couchdb::save response=" << body(response) << std::endl;
      
      // now, turn the response body into a ptree since it is json
      ptree response_ptree;
      try {
	std::istringstream iss( body(response) );
	json_parser::read_json( iss, response_ptree );
      } catch (boost::exception& e ) {
	e << couchdb_request_uri_error_info( post_uri )
	  << couchdb_raw_response_error_info( body(response) );
	throw;
      }

      // Ok, check if the response is an error response, and if
      // it is throw an exception
      if( response_ptree.get( "error", "" ).size() > 0 ) {
	if( response_ptree.get<std::string>( "error" ) == "conflict" ) {
	  throw couchdb_conflict_exception()
	    << couchdb_request_uri_error_info( post_uri )
	    << couchdb_raw_response_error_info( body(response) )
	    << couchdb_response_error_info( response_ptree );
	} else {
	  throw couchdb_response_exception() 
	    << couchdb_request_uri_error_info( post_uri )
	    << couchdb_raw_response_error_info( body(response) )
	    << couchdb_response_error_info( response_ptree );
	}
      }
      
      return response_ptree;
    }

    //========================================================================

    ptree Couchdb::fetch( const uri::uri& doc_id ) const
    {

      // create an http client to eventually POST to the couchdb uri
      client c;
      
      // ok, now GET to the couchdb url the document
      uri::uri get_uri = this->_couchdb_database_uri;
      get_uri.append( "/" + doc_id.string() );
      client::request request( get_uri );
      client::response response = c.get(request);
      
      // parse the response as json into a ptree
      ptree response_ptree;
      std::istringstream iss( body(response) );
      json_parser::read_json( iss, response_ptree );
      
      return response_ptree;
    }

    //========================================================================

  ptree 
  Couchdb::try_update( const uri::uri& doc_id,
		       const std::vector<std::pair<std::string,std::string> >& puts,
		       const size_t num_retries ) const
  {
    
    for( size_t i = 0; i < num_retries; ++i ) {

      try {
      
	// fetch the document from couchdb
	ptree doc;
	try {
	  doc = this->fetch( doc_id );
	} catch ( couchdb_response_exception& e ) {
	  // eat up this exceptio nand just try again
	  continue;
	}
	
	// ok, now apply the puts wanted
	for( size_t put_i = 0; put_i < puts.size(); ++put_i ) {
	  doc.put( puts[put_i].first, 
		   puts[put_i].second );
	}
	
	// try to save the document
	ptree response;
	try {
	  response = this->save( doc, doc_id.string() );
	} catch ( couchdb_response_exception& e ) {
	  // ignore this exception and just try again
	  continue;
	}

	// if we got here, we successfully update so return
	// successful response
	return response;

      } catch ( boost::exception& e ) {
      
	// ok, we got a non-response related exception,
	// so add information we know of and re-throw since
	// we were not expecting this
	e << couchdb_request_uri_error_info( doc_id )
	  << couchdb_num_retries_error_info( i );
	throw;
      }
      
    }
    
    // num retries exhausted, throw exception
    throw couchdb_exhausted_retries_exception()
      << couchdb_request_uri_error_info( doc_id )
      << couchdb_num_retries_error_info( num_retries );
  }
      

    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================
    //========================================================================

  }

