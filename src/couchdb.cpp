
#include "couchdb.hpp"
#include "utils.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <curl/curl.h>

#include <iostream>
#include <sstream>



namespace couchdb {
  
  using namespace boost::property_tree;
  using namespace boost;
  
  
  //========================================================================

  // Global curl state
  bool _curl_init = false;
  
  //========================================================================
  
  Couchdb::Couchdb( const std::string& database_uri )
    : _couchdb_database_uri( database_uri ), _ensured_db(false)
  {
    if( !_curl_init ) {
      curl_global_init(CURL_GLOBAL_ALL);
    }
  }
  
  //========================================================================

  static size_t
  AppendToStream( void* contents, size_t size, size_t nmemb, void* userp )
  {
    size_t realsize = size * nmemb;
    std::stringstream *ss = (std::stringstream*)userp;
    ss->write( (char*)contents, realsize );
    return realsize;
  }

  static size_t
  ReadFromStream( char* buffer, size_t size, size_t nitems, void* userp )
  {
    size_t realsize = size * nitems;
    std::istringstream* iss = (std::istringstream*)userp;
    return iss->readsome( buffer, realsize );
  }

  //========================================================================
  
  ptree Couchdb::save( const ptree& doc,
		       const optional<std::string>& id ) const
  {
    if( !_ensured_db ) {
      ensure_db_exists( _couchdb_database_uri );
      _ensured_db = true;
    }

    CURL *curl_handle = curl_easy_init();
    
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
    #ifndef NDEBUG
    std::cout << "about to save: ";
    xml_parser::write_xml( std::cout, doc );
    #endif
    
    // convert ptree to json
    std::ostringstream oss;
    json_parser::write_json( oss, doc );
    std::string json_doc = oss.str();
    std::istringstream json_stream( json_doc );
    
    // debug
    //std::cout << "Couchdb::save json_doc=" << json_doc << std::endl;
    
    // ok, now setup curl to POST the document to the uri
    std::string post_uri = this->_couchdb_database_uri;
    post_uri.append( the_id );
    std::stringstream response_stream;
    curl_easy_setopt( curl_handle, CURLOPT_URL, post_uri.c_str() );
    curl_easy_setopt( curl_handle, CURLOPT_UPLOAD, 1 );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, AppendToStream );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &response_stream );
    curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
    curl_easy_setopt( curl_handle, CURLOPT_REFERER, "pip" );
    curl_easy_setopt( curl_handle, CURLOPT_READFUNCTION, ReadFromStream );
    curl_easy_setopt( curl_handle, CURLOPT_READDATA, &json_stream );

    // perform the POST, the response will be in response_stream if no error
    CURLcode res = curl_easy_perform(curl_handle);
    if( res != CURLE_OK ) {
      curl_easy_cleanup(curl_handle);
      BOOST_THROW_EXCEPTION( couchdb_exception()
			     << couchdb_request_uri_error_info( post_uri )
			     << couchdb_network_message_error_info( curl_easy_strerror( res ) ) );
    }
    curl_easy_cleanup(curl_handle);
    
    
    // try {
    //   response = c.put(request, json_doc, std::string("application/json"));
    // } catch (boost::exception& e ) {
    //   e << couchdb_request_uri_error_info( post_uri );
    //   throw;
    // }

    
    // debug
    //std::cout << "Couchdb::save response=" << body(response) << std::endl;
    
    // now, turn the response body into a ptree since it is json
    ptree response_ptree;
    std::string raw_response = response_stream.str();
    try {
      json_parser::read_json( response_stream, response_ptree );
    } catch (boost::exception& e ) {
      e << couchdb_request_uri_error_info( post_uri )
	<< couchdb_raw_response_error_info( raw_response );
      throw;
    }
    
    // Ok, check if the response is an error response, and if
    // it is throw an exception
    if( this->is_response_exception( response_ptree ) ) {
      try {
	this->throw_exception_from_response( response_ptree );
      } catch ( boost::exception& e ) {
	e
	  << couchdb_request_uri_error_info( post_uri )
	  << couchdb_raw_response_error_info( raw_response );
	throw;
      }
    }
    
    return response_ptree;
  }
  
  //========================================================================
  
  ptree Couchdb::fetch( const std::string& doc_id ) const
  {
    if( !_ensured_db ) {
      ensure_db_exists( _couchdb_database_uri );
      _ensured_db = true;
    }
    
    // get CURL handle
    CURL* curl_handle = curl_easy_init();
    
    // ok, now GET to the couchdb url the document
    std::string get_uri = this->_couchdb_database_uri;
    get_uri.append( "/" + doc_id );
    std::stringstream response_stream;
    curl_easy_setopt( curl_handle, CURLOPT_URL, get_uri.c_str() );
    curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );    
    curl_easy_setopt( curl_handle, CURLOPT_REFERER, "pip" );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, AppendToStream );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &response_stream );

    CURLcode res = curl_easy_perform(curl_handle);
    if( res != CURLE_OK ) {
      curl_easy_cleanup(curl_handle);
      BOOST_THROW_EXCEPTION( couchdb_exception()
			     << couchdb_request_uri_error_info( get_uri )
			     << couchdb_network_message_error_info( curl_easy_strerror( res ) ) );
    }
    curl_easy_cleanup(curl_handle);
    
    
    // parse the response as json into a ptree
    ptree response_ptree;
    std::string raw_response = response_stream.str();
    json_parser::read_json( response_stream, response_ptree );
    
    // throw exception if reponse is an error
    if( this->is_response_exception( response_ptree ) ) {
      try {
	this->throw_exception_from_response( response_ptree );
      } catch ( boost::exception& e ) {
	e
	  << couchdb_request_uri_error_info( get_uri )
	  << couchdb_raw_response_error_info( raw_response );
	throw;
      }
    }
    
    return response_ptree;
  }
  
  //========================================================================
  
  ptree 
  Couchdb::try_update( const std::string& doc_id,
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
	  response = this->save( doc, doc_id );
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
    BOOST_THROW_EXCEPTION( couchdb_exhausted_retries_exception()
			   << couchdb_request_uri_error_info( doc_id )
			   << couchdb_num_retries_error_info( num_retries ) );
  }
      

    //========================================================================
  
  ptree
  Couchdb::try_ensure_substructure( const std::string& doc_id,
				    const ptree& structure,
				    const size_t num_retries) const
  {

    for( size_t i = 0; i < num_retries; ++i ) {
      
      try {
	
	// fetch the document from couchdb
	ptree doc;
	try {
	  doc = this->fetch( doc_id );
	} catch ( couchdb_document_does_not_exist_exception& de ) {
	  // this is ok, we'll just assume empty document
	  //std::cout << "ignoring doc not existing..." << std::endl;
	} catch ( couchdb_response_exception& e ) {
	  // eat up this exceptio nand just try again
	  continue;
	}
	
	// ok, now ensure the given structure
	ensure_substructure( doc, structure );
	
	// try to save the document
	ptree response;
	try {
	  response = this->save( doc, doc_id );
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
    BOOST_THROW_EXCEPTION( couchdb_exhausted_retries_exception()
			   << couchdb_request_uri_error_info( doc_id )
			   << couchdb_num_retries_error_info( num_retries ) );
    
  }
  
    //========================================================================

  bool Couchdb::is_response_exception( const ptree& response ) const
  {
    return ( response.get( "error", "" ).size() > 0 );
  }

    //========================================================================

  void 
  Couchdb::throw_exception_from_response( const ptree& response ) const
  {
    if( response.get<std::string>( "error", "" ) == "conflict" ) {
      BOOST_THROW_EXCEPTION( couchdb_conflict_exception()
			     << couchdb_response_error_info( response ) );
    } else if( response.get<std::string>( "error", "" ) == "not_found" ) {
      BOOST_THROW_EXCEPTION( couchdb_document_does_not_exist_exception()
			     << couchdb_response_error_info( response ) );
    } else {
      BOOST_THROW_EXCEPTION( couchdb_response_exception()
			     << couchdb_response_error_info( response ) );
    }
    
  }

    //========================================================================

  void Couchdb::ensure_db_exists( const std::string& db_url ) const
  {
    // first perform a HED request on the url to see if it is there
    // then perform a PUT if not there to create the db

    // get CURL handle
    CURL* curl_handle = curl_easy_init();
    
    // ok, now request a HEAD (GET with no body)
    // to the couchdb url the document
    curl_easy_setopt( curl_handle, CURLOPT_URL, db_url.c_str() );
    curl_easy_setopt( curl_handle, CURLOPT_HTTPGET, 1 );
    curl_easy_setopt( curl_handle, CURLOPT_NOBODY, 1 );
    curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );    

    CURLcode res = curl_easy_perform(curl_handle);
    if( res != CURLE_OK ) {
      curl_easy_cleanup(curl_handle);
      BOOST_THROW_EXCEPTION( couchdb_exception()
			     << couchdb_request_uri_error_info( db_url )
			     << couchdb_network_message_error_info( curl_easy_strerror( res ) ) );
    }

    // check if HEAD returned a 404 (doc not found)
    long http_code = -1;
    res = curl_easy_getinfo( curl_handle, CURLINFO_RESPONSE_CODE, &http_code );
    if( res != CURLE_OK ) {
      curl_easy_cleanup(curl_handle);
      BOOST_THROW_EXCEPTION( couchdb_exception()
			     << couchdb_request_uri_error_info( db_url )
			     << couchdb_network_message_error_info( curl_easy_strerror( res ) ) );
    }

    // ok, if it was a 404, request a PUT with the db to create it
    if( http_code == 404 ) {

      // ok, now setup curl to POST the document to the uri
      std::stringstream response_stream;
      std::istringstream json_stream("");
      curl_easy_setopt( curl_handle, CURLOPT_URL, db_url.c_str() );
      curl_easy_setopt( curl_handle, CURLOPT_UPLOAD, 1 );
      curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, AppendToStream );
      curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &response_stream );
      curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
      curl_easy_setopt( curl_handle, CURLOPT_READFUNCTION, ReadFromStream );
      curl_easy_setopt( curl_handle, CURLOPT_READDATA, &json_stream );
      
      // perform the POST, the response will be in response_stream if no error
      res = curl_easy_perform(curl_handle);
      if( res != CURLE_OK ) {
	curl_easy_cleanup(curl_handle);
	BOOST_THROW_EXCEPTION( couchdb_exception()
			       << couchdb_request_uri_error_info( db_url )
			       << couchdb_network_message_error_info( curl_easy_strerror( res ) ) );
      }
      
      // now, turn the response body into a ptree since it is json
      ptree response_ptree;
      std::string raw_response = response_stream.str();
      try {
	json_parser::read_json( response_stream, response_ptree );
      } catch (boost::exception& e ) {
	curl_easy_cleanup(curl_handle);
	e << couchdb_request_uri_error_info( db_url )
	  << couchdb_raw_response_error_info( raw_response );
	throw;
      }
      
      // Ok, check if the response is an error response, and if
      // it is throw an exception
      if( this->is_response_exception( response_ptree ) ) {
	try {
	  this->throw_exception_from_response( response_ptree );
	} catch ( boost::exception& e ) {
	  curl_easy_cleanup(curl_handle);
	  e
	    << couchdb_request_uri_error_info( db_url )
	    << couchdb_raw_response_error_info( raw_response );
	  throw;
	}
      } 
    }
    
    curl_easy_cleanup(curl_handle);
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

  }

