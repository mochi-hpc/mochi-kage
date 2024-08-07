add_executable (ProxyTest ProxyTest.cpp)
target_link_libraries (ProxyTest PRIVATE Catch2::Catch2WithMain kage::server)
add_test (NAME ProxyTest COMMAND ./ProxyTest)
