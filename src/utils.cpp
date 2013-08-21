
#include "utils.hpp"

namespace couchdb {


  using namespace boost::property_tree;


  //-----------------------------------------------------------------------

  ptree& ensure_substructure( ptree& target, const ptree& source )
  {
    
    ptree::const_iterator source_children_iter;
    for( source_children_iter = source.begin();
	 source_children_iter != source.end();
	 ++source_children_iter ) {

      // if target does not even have the named child,
      // create it as a copy of the source's child 
      // (we are then done with this soure child )
      // Othewise, recurse
      if( target.find( source_children_iter->first ) == target.not_found() ) {
	target.put_child( source_children_iter->first,
			  source_children_iter->second );
      } else {
	if( source_children_iter->second.get_value<std::string>().empty() == false ) {
	  target.put_value( source_children_iter->second.get_value<std::string>() );
	}

	// This is tricky, here we assume that target hs only one
	// such unique path in it's children, and hence we recurse on
	// the result of find().
	// If we wanted to, we would instead recurse on all results from
	// calling equal_range().
	// Even more amusing, we completely ignore the fact that the source
	// may have multiple chilcren with the same key, and they may all
	// in fact be "ensured" on the same child of the target.
	ensure_substructure( target.find( source_children_iter->first )->second,
			     source_children_iter->second );
      }
    }

    return target;
  }

  //-----------------------------------------------------------------------


}
