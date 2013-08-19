
#if !defined( __ERF_COUCHDB_couchdb_HPP__ )
#define __ERF_COUCHDB_couchdb_HPP__


#include "erf-couchdb/exceptions.hpp"

namespace erf {
  namespace couchdb {


    //-----------------------------------------------------------------------

    // Description:
    // The interface object for talking to/from a couchdb instance
    // running on some host machine
    class Couchdb {

    public:

      // Description:
      // Creates a new connection to a couchdb instance
      Couchdb( const boost::network::uri::uri & database_url );
	       
      // Description:
      // Saves a document to the couch db
      // THe document *must* be a boost::property_tree instance.
      // If the id is not given, a uuid version 1 will be created
      // for the document.
      // This will return the response from couchdb as a ptree
      //
      // Will throw an exception if a communication/network error happens
      // or the given ptree cannot be converted to JSON (as per write_json
      // under the ptree boost documentation).
      //
      // Will also throw an exception if the response from couchdb
      // is an error response.
      boost::property_tree::ptree save( const boost::property_tree::ptree& doc );

      // Description:
      // Fetch a particularly named document form thie couchdb.
      // The name is the id.
      // Note that we use URI's here since couchdb names are often
      // URIs because of the _design / _view stuff.
      //
      // Will throw an exception of network/communication issues
      // or if the document is not found.
      boost::property_tree::ptree fetch( const boost::network::uri::uri& doc_id );
      
      
      
    protected:

    private:

    };


    //-----------------------------------------------------------------------


  }
}


#endif

