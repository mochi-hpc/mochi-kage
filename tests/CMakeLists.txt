set (backend_sources echo/EchoBackend.cpp passthrough/PassThroughBackend.cpp)

file (GLOB test-sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach (test-source ${test-sources})
    get_filename_component (test-target ${test-source} NAME_WE)
    add_executable (${test-target} ${test-source} ${backend_sources})
    target_link_libraries (${test-target} PRIVATE
        Catch2::Catch2WithMain kage::server spdlog::spdlog fmt::fmt)
    add_test (NAME ${test-target} COMMAND timeout 60s ./${test-target})
endforeach ()
