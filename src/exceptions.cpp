
#include "exceptions.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <sstream>


namespace couchdb {

  //-----------------------------------------------------------------------

  // Description:
  // Determines how we print out responses in diagnostic information
  // messages
  
  std::string to_string( const couchdb_response_error_info& info )
  {
    std::ostringstream oss;
    boost::property_tree::json_parser::write_json( oss, info.value() );
    return oss.str();
  }
  
    
//-----------------------------------------------------------------------


}
