# a simple test to save and fetch a couchdb document
add_executable( erf-couchdb-test-couchdb-api
  test_couchdb_api.cpp)
pods_use_pkg_config_packages( erf-couchdb-test-couchdb-api 
  erf-couchdb)
pods_install_executables( erf-couchdb-test-couchdb-api )
