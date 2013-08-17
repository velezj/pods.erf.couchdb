

#if !defined( __ERF_COUCHDB_exceptions_HPP__ )
#define __ERF_COUCHDB_exceptions_HPP__


#include <boost/network/uri.hpp>
#include <boost/exception/all.hpp>

namespace erf {
  namespace couchdb {


    //-----------------------------------------------------------------------
    
    // Description:
    // The response from couchdb (ptree format)
    typedef boost::error_info<struct couchdb_response_tag, boost::property_tree::ptree > couchdb_response_error_info;
    
    // Description:
    // The response from couchdb (raw)
    typedef boost::error_info<struct couchdb_raw_response_tag, std::string > couchdb_raw_response_error_info;
    
    // Description:
    // The couchdb uri for the rewuest
    typedef boost::error_info<struct couchdb_request_uri_tag, boost::network::uri::uri> couchdb_request_uri_error_info;
    

    //-----------------------------------------------------------------------

    // Description:
    // A general exception base class for couchdb exceptions.
    class couchdb_exception : virtual boost::exception, virtual std::exception { }; 
    {
    };

    //-----------------------------------------------------------------------

    // Description:
    // A couchdb exception from a bad response
    class couchdb_response_exception : public couchdb_exception
    {
    };

    //-----------------------------------------------------------------------

    // Description:
    // A couchdb exception from a conflict
    class couchdb_conflict_exception : public couchdb_response_exception
    {
    };


    //-----------------------------------------------------------------------
    

  }
}