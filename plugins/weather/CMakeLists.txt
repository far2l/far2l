project(weather)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(CURL)
if (CURL_FOUND)
    message(STATUS "Found CURL: ${CURL_LIBRARIES}")
    add_compile_definitions(USE_CURL)
endif()

set(SOURCES
    src/Config.cpp
    src/WeatherCore.cpp
    src/Weather.cpp
)

add_library(${PROJECT_NAME} MODULE ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE utils far2l)
if (CURL_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DWINPORT_DIRECT
    -DUNICODE
    -DFAR_DONT_USE_INTERNALS
)

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    ${CMAKE_SOURCE_DIR}/far2l/far2sdk
    ${CMAKE_SOURCE_DIR}/WinPort
    ${CMAKE_SOURCE_DIR}/utils/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DIR}/Plugins/${PROJECT_NAME}/plug"
    PREFIX ""
    SUFFIX ".far-plug-wide"
)

file(GLOB_RECURSE RESOURCE_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/configs
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/*"
)
setup_target_auxiliary_files(${PROJECT_NAME}
    RESOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/configs
    "${INSTALL_DIR}/Plugins/${PROJECT_NAME}/"
)
add_dependencies(${PROJECT_NAME} copy_aux_files_for_${PROJECT_NAME})
