cmake_minimum_required(VERSION 3.14)
project(brazier-orm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    add_compile_options(/utf-8 /bigobj)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(brazier-logger CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(LIBRARIES
    PostgreSQL::PostgreSQL
    nlohmann_json::nlohmann_json
)

file(GLOB_RECURSE BRAZIER_ORM_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(NOT BRAZIER_ORM_SOURCES)
    message(FATAL_ERROR
        "No sources found in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
endif()

add_library(brazier_orm ${BRAZIER_ORM_SOURCES})

set_target_properties(brazier_orm PROPERTIES EXPORT_NAME orm)

add_library(brazier::orm ALIAS brazier_orm)

target_include_directories(brazier_orm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(brazier_orm PUBLIC cxx_std_20)

target_link_libraries(brazier_orm PUBLIC 
    brazier::logger
    ${LIBRARIES}
)

if(MSVC)
    set_target_properties(brazier_orm PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

option(BUILD_DEMO "Build brazierORM demo executable" ON)

if(BUILD_DEMO)
    add_executable(brazierORM brazierORM.cpp)
    target_include_directories(brazierORM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(brazierORM PRIVATE brazier_orm)
endif()

install(TARGETS brazier_orm
    EXPORT brazier-ormTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES brazierORM.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brazier/orm)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brazier/orm)

install(EXPORT brazier-ormTargets
    NAMESPACE brazier::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/brazier-orm
    FILE brazier-ormTargets.cmake
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/brazier-ormConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/brazier-ormConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/brazier-orm
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/brazier-ormConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/brazier-ormConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/brazier-ormConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/brazier-orm
)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    message(STATUS "Building tests")

    find_package(GTest CONFIG REQUIRED)

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "tests/*.cpp"
    )

    add_executable(brazier-orm_tests ${TEST_SOURCES})

    target_include_directories(brazier-orm_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${PostgreSQL_INCLUDE_DIRS}
    )

    target_compile_definitions(brazier-orm_tests PRIVATE
        BOOST_ASIO_HAS_CO_AWAIT
        BOOST_ASIO_HAS_STD_COROUTINE
    )

    set_target_properties(brazier_orm PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(brazier-orm_tests PRIVATE
        brazier_orm
        brazier::logger
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
        ${LIBRARIES}
    )

    add_custom_target(check
        COMMAND $<TARGET_FILE:brazier-orm_tests>
        DEPENDS brazier-orm_tests
        COMMENT "Running tests..."
    )
endif()