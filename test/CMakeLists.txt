# a simple test to save and fetch a couchdb document
add_executable( erf-couchdb-test-couchdb-api
  test_couchdb_api.cpp)
pods_use_pkg_config_packages( erf-couchdb-test-couchdb-api 
  erf-couchdb
  boost-1.54.0
  cppnetlib-client-connections-0.10.1
  cppnetlib-uri-0.10.1 
  openssl-1.0.1e
  unittest++)
pods_install_executables( erf-couchdb-test-couchdb-api )
