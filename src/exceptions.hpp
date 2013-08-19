

#if !defined( __ERF_COUCHDB_exceptions_HPP__ )
#define __ERF_COUCHDB_exceptions_HPP__


#include <boost/network/uri.hpp>
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>

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
    

    // Description:
    // The number of retries tried
    typedef boost::error_info<struct couchdb_num_retries_tag, size_t> couchdb_num_retries_error_info;
    

    //-----------------------------------------------------------------------

    // Description:
    // A general exception base class for couchdb exceptions.
    struct couchdb_exception : virtual boost::exception, virtual std::exception
    {
    };

    //-----------------------------------------------------------------------

    // Description:
    // A couchdb exception from a bad response
    struct couchdb_response_exception : public couchdb_exception
    {
    };

    //-----------------------------------------------------------------------

    // Description:
    // A couchdb exception from a conflict
    struct couchdb_conflict_exception : public couchdb_response_exception
    {
    };

    //-----------------------------------------------------------------------

    // Description:
    // A couchdb exception from an exhausted amount of retries
    struct couchdb_exhausted_retries_exception : public couchdb_exception
    {
    };


    //-----------------------------------------------------------------------
    

  }

#endif


