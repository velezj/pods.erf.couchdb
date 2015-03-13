
#include "utils.hpp"
#include <iostream>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <set>

namespace couchdb {


  using namespace boost::property_tree;


  //-----------------------------------------------------------------------

  ptree& ensure_substructure( ptree& target, const ptree& source )
  {
    static long _debug_depth = 0;
    
    #ifndef NDEBUG
    std::cout << std::setw(2*_debug_depth) << "" << "ensuring substructure called" << std::endl;
    ++_debug_depth;
    #endif

    // ok, set any current values
    if( source.get_value_optional<std::string>() ) {

      #ifndef NDEBUG
      std::cout << std::setw(2*_debug_depth) << ""
		<< "source["
		<< "] = " << source.get_value<std::string>() << ", putting value onto target" << std::endl;
      #endif
      
      target.put_value( source.get_value<std::string>() );
    }

    // ok, first find the set of unique direct keys for the source
    std::set<std::string> source_unique_direct_keys;
    ptree::const_iterator source_children_iter;
    for( source_children_iter = source.begin();
	 source_children_iter != source.end();
	 ++source_children_iter ) {
      source_unique_direct_keys.insert( source_children_iter->first );
    }

    // now, iterate over the unique children keys of source
    std::set<std::string>::const_iterator source_key_iter;
    for( source_key_iter = source_unique_direct_keys.begin();
	 source_key_iter != source_unique_direct_keys.end();
	 ++source_key_iter ) {

      // grab "a" child from source with this key
      source_children_iter = source.to_iterator( source.find( *source_key_iter ) );
      
      // if target does not even have the named child,
      // create it as a copy of the source's child 
      // (we are then done with this soure child )
      // Othewise, recurse
      if( target.find( source_children_iter->first ) == target.not_found() ) {

	// ok, now we need to put as many children of sources into
	// target directly
	std::pair<ptree::const_assoc_iterator,
		  ptree::const_assoc_iterator> source_children_iters
	  = source.equal_range( *source_key_iter );
	ptree::const_assoc_iterator source_child_iter;
	for( source_child_iter = source_children_iters.first;
	     source_child_iter != source_children_iters.second;
	     ++source_child_iter ) {

          #ifndef NDEBUG
	  std::cout << std::setw(2*_debug_depth) << "" << "source[" << source_child_iter->first << "] not found in taget, putting whole child" << std::endl;
	  #endif
	
	  target.put_child( source_child_iter->first,
			    source_child_iter->second );
	}
	
      } else {

	// ok, the trickyness explodes right now:
	// Firsty, check how many children target and source have with the
	// path of this current source child.
	size_t target_num_children = target.count( *source_key_iter );
	size_t source_num_children = source.count( *source_key_iter );

	// If the counts are the same, then we need to use *insertion order*
	// to map from child to child and recurse with their substructure
	if( target_num_children == source_num_children ) {

	  #ifndef NDEBUG
	  std::cout << std::setw(2*_debug_depth) << ""
		    << "both target and source have same # for path: "
		    << *source_key_iter
		    << ", using insertion order matching!"
		    << std::endl;
	  #endif

	  ptree::const_iterator source_iter = source.begin();
	  ptree::iterator target_iter = target.begin();
	  size_t matches = 0;
	  while( matches < target_num_children ) {
	    if( source_iter->first == *source_key_iter &&
		target_iter->first == *source_key_iter ) {

	      ++matches;
	      #ifndef NDEBUG
	      std::cout << std::setw(2*_debug_depth) << ""
			<< "recurse on path: " << source_children_iter->first << std::endl;
              #endif
	      ensure_substructure( target_iter->second,
				   source_iter->second );

	      continue;
	    }

	    // ok, move foward one or both of iterators to find a next match
	    if( source_iter->first != *source_key_iter &&
		source_iter != source.end() ) {
	      ++source_iter;
	    }
	    if( target_iter->first != *source_key_iter &&
		target_iter != target.end() ) {
	      ++target_iter;
	    }
	    
	  }

	} else {

	  // ok, the number of children are different between
	  // target and source. There really is no "right" answer
	  // here so we just remove all of target's children and
	  // replace them with the source childrens.
	  #ifndef NDEBUG
	  std::cout << std::setw(2*_debug_depth) << ""
		    << "both target and source have different # children for path: "
		    << *source_key_iter
		    << ", erasing target's children and recursing :-)"
		    << std::endl;
	  #endif
	  target.erase( *source_key_iter );
	  ensure_substructure( target, source );

	}
      }
    }

    #ifndef NDEBUG
    --_debug_depth;
    std::cout << std::setw(2*_debug_depth) << "" << "end ensuring substructure" << std::endl;
    #endif

    return target;
  }

  //-----------------------------------------------------------------------


}
