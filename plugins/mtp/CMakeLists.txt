# MTP/PTP plugin module backed by libmtp (vendored static build in libmtp/).
# libusb is also vendored statically — see libusb/VENDORING.md.

# add_subdirectory(libusb)
# add_subdirectory(libmtp)

set(SOURCES
    src/FARPlugin.cpp
    src/MTPPlugin.cpp
    src/MTPDialogs.cpp
    src/MTPLog.cpp
    src/LibMtpBackend.cpp
    src/ProgressBatch.cpp
)

add_library(mtp MODULE ${SOURCES})

# libusb_static is also pulled transitively via vendored libmtp_static, but link it explicitly so
# the plugin (which uses libusb directly — see ReadDeviceSerialViaLibusb, SharedLibusbContext) still
# resolves when libmtp_static is an INTERFACE wrapper around system libmtp.
#target_link_libraries(mtp utils far2l libmtp_static libusb_static)

if(APPLE)
    # CoreFoundation: CFPreferences{SetValue,Synchronize} for icdd bypass.
    target_link_libraries(mtp "-framework CoreFoundation")
endif()

# Convert version to mumeric form
find_path(LIBMTP_INCLUDE_DIR libmtp.h)
file(READ "${LIBMTP_INCLUDE_DIR}/libmtp.h" LIBMTP_HEADER_CONTENT)
string(REGEX MATCH "#define[ \t]+LIBMTP_VERSION[ \t]+([0-9]+)\\.([0-9]+)\\.([0-9]+)" _match "${LIBMTP_HEADER_CONTENT}")
set(LIBMTP_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(LIBMTP_VERSION_MINOR "${CMAKE_MATCH_2}")
set(LIBMTP_VERSION_PATCH "${CMAKE_MATCH_3}")
math(EXPR LIBMTP_VERSION_CODE "${LIBMTP_VERSION_MAJOR} * 10000 + ${LIBMTP_VERSION_MINOR} * 100 + ${LIBMTP_VERSION_PATCH}")

include(CheckSymbolExists)

set(CMAKE_REQUIRED_INCLUDES ${LIBMTP_INCLUDE_DIR})
check_symbol_exists(LIBMTP_FreeMemory "libmtp.h" HAVE_LIBMTP_FREEMEMORY)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(LIBMTP REQUIRED libmtp)

target_include_directories(mtp PRIVATE ${LIBUSB_INCLUDE_DIRS} ${LIBMTP_INCLUDE_DIRS})
target_link_libraries(mtp PRIVATE utils far2l ${LIBUSB_LIBRARIES})
target_link_options(mtp PRIVATE "-l${LIBMTP_LIBRARIES}")

find_path(LIBUSB_INCLUDE_DIR libusb.h)

target_compile_definitions(mtp PRIVATE
    -DWINPORT_DIRECT
    -DUNICODE
    -D_UNICODE
    -DFAR_DONT_USE_INTERNALS
    -DLIBMTP_VERSION_CODE=${LIBMTP_VERSION_CODE}
    # Activate MTPLog DBG() in Debug builds — macro gated on DEBUG/_DEBUG (matches ADBLog), CMake Debug config doesn't set them on Unix.
    "$<$<CONFIG:Debug>:DEBUG>"
)

if(HAVE_LIBMTP_FREEMEMORY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_LIBMTP_FREEMEMORY")
endif()

#if(MTP_SYSTEM_LIBMTP)
    # System libmtp hides its internal PTP symbols (libmtp.sym version-script); ListChildrenFast falls back to the slow path.
    target_compile_definitions(mtp PRIVATE MTP_SYSTEM_LIBMTP)
#endif()

target_include_directories(mtp PRIVATE .)
target_include_directories(mtp PRIVATE ${CMAKE_SOURCE_DIR}/far2l/far2sdk)
target_include_directories(mtp PRIVATE ${CMAKE_SOURCE_DIR}/WinPort)

# target_include_directories(mtp private ${LIBUSB_INCLUDE_DIR})

set_target_properties(mtp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DIR}/Plugins/mtp/plug"
    PREFIX ""
    SUFFIX ".far-plug-wide"
)

set(CURRENT_TARGET "mtp")
file(GLOB_RECURSE RESOURCE_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/configs
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/*"
)
setup_target_auxiliary_files(${CURRENT_TARGET}
    RESOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/configs
    "${INSTALL_DIR}/Plugins/${CURRENT_TARGET}/"
)
add_dependencies(${CURRENT_TARGET} copy_aux_files_for_${CURRENT_TARGET})
