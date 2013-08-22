
#if !defined( __ERF_COUCHDB_couchdb_HPP__ )
#define __ERF_COUCHDB_couchdb_HPP__


#include "erf-couchdb/exceptions.hpp"

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
      boost::property_tree::ptree 
      save( const boost::property_tree::ptree& doc,
	    const boost::optional<std::string>& id = boost::optional<std::string>() ) const;

      // Description:
      // Fetch a particularly named document form thie couchdb.
      // The name is the id.
      // Note that we use URI's here since couchdb names are often
      // URIs because of the _design / _view stuff.
      //
      // Will throw an exception of network/communication issues
      // or if the document is not found.
      boost::property_tree::ptree fetch( const boost::network::uri::uri& doc_id ) const;


      // Description:
      // Try to update a given document with the given list of
      // additions (as per argumetns to ptree.put).
      // This will try for a number of givne retries (default ot 10)
      // before failing
      //
      // Returns the response from the successfull update
      boost::property_tree::ptree 
      try_update( const boost::network::uri::uri& doc_id,
		  const std::vector<std::pair<std::string,std::string> >& puts,
		  const size_t num_retries = 10 ) const;
      

      // Description:
      // Try to ensure the given document (by id ) constians the given 
      // substructure.
      // This will try for a number of retries (default of 10) before
      // failing
      //
      // This will create the document if it does not exist
      //
      // Returns the response from hte successful update
      boost::property_tree::ptree
      try_ensure_substructure( const boost::network::uri::uri& doc_id,
			       const boost::property_tree::ptree& structure,
			       const size_t num_retries = 10 ) const;
      
    protected:

      // Description:
      // The URI pointintg to the couchdb databse
      boost::network::uri::uri _couchdb_database_uri;


      // Description:
      // Builds up an exception from a couchdb response and throws it
      void throw_exception_from_response( const boost::property_tree::ptree& response ) const;

      // Description:
      // Returns true iff the given response should be translated to
      // and exception
      bool is_response_exception( const boost::property_tree::ptree& response ) const;

    private:

    };


    //-----------------------------------------------------------------------


  }


#endif

