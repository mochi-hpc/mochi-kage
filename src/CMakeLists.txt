# set source files
set (server-src-files
     Provider.cpp
     Backend.cpp
     margo/MargoBackend.cpp)

if (ENABLE_ZMQ)
    list (APPEND server-src-files zmq/ZMQBackend.cpp)
    set (OPTIONAL_ZMQ cppzmq)
endif ()

set (module-src-files
     BedrockModule.cpp)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (kage-pkg "share/cmake/kage")

# set version fo shared library
set (kage-vers "${KAGE_VERSION_MAJOR}.${KAGE_VERSION_MINOR}")

# server library
add_library (kage-server ${server-src-files} ${echo-src-files})
add_library (kage::server ALIAS kage-server)
target_link_libraries (kage-server
    PUBLIC thallium nlohmann_json::nlohmann_json nlohmann_json_schema_validator::validator
    PRIVATE spdlog::spdlog fmt::fmt coverage_config ${OPTIONAL_ZMQ})
target_include_directories (kage-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (kage-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (kage-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (kage-server
    PROPERTIES VERSION ${KAGE_VERSION}
    SOVERSION ${KAGE_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock module library
add_library (kage-bedrock-module ${module-src-files})
add_library (kage::bedrock ALIAS kage-bedrock-module)
target_link_libraries (kage-bedrock-module kage-server bedrock::module-api coverage_config)
target_include_directories (kage-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (kage-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (kage-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (kage-bedrock-module
    PROPERTIES VERSION ${KAGE_VERSION}
    SOVERSION ${KAGE_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file (
    "kage-config-version.cmake"
    VERSION ${KAGE_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set (INCLUDE_INSTALL_DIR include/)
set (LIB_INSTALL_DIR lib/)
configure_package_config_file (kage-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/kage-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/kage/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lkage-server")
configure_file ("kage-server.pc.in" "kage-server.pc" @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS kage-server
         EXPORT kage-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS kage-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT kage-targets
         DESTINATION ${kage-pkg}
         FILE "kage-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/kage-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kage-config-version.cmake"
         DESTINATION ${kage-pkg} )
install (DIRECTORY ../include/kage
         DESTINATION include
         FILES_MATCHING PATTERN "*.hpp")
install (DIRECTORY ../include/kage
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/kage-server.pc"
         DESTINATION "lib/pkgconfig/")
