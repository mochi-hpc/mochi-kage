add_executable (ClientTest ClientTest.cpp)
target_link_libraries (ClientTest PRIVATE Catch2::Catch2WithMain kage::server kage::client)
add_test (NAME ClientTest COMMAND ./ClientTest)

add_executable (ProxyTest ProxyTest.cpp)
target_link_libraries (ProxyTest PRIVATE Catch2::Catch2WithMain kage::server kage::client)
add_test (NAME ProxyTest COMMAND ./ProxyTest)
