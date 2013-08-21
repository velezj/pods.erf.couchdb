
#if !defined( __ERF_COUCHDB_utils_HPP__ )
#define __ERF_COUCHDB_utils_HPP__


#include "erf-couchdb/exceptions.hpp"

  namespace couchdb {


    //-----------------------------------------------------------------------

    // Description:
    // Ensures that the target ptree (the first arg) ha sat least
    // the substructure defined in the source ptree (the second arg).
    // This basically means that the target will have all keys in
    // the source equal to the source, but any other keys remain
    // unchanged in the target.
    //
    // Returns the target
    //
    // ASSUMPTION: source and target each have unique children.
    //             so no duplicate paths/subpaths!
    //             Otherwise we don't even know what it means to
    //             "ensure" a substructure .....
    boost::property_tree::ptree& 
    ensure_substructure( boost::property_tree::ptree& target,
			 const boost::property_tree::ptree& source );
    

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------



  }

#endif
