file(GLOB_RECURSE UTILS_HEADERS "${CLOUD_STORAGE_INCLUDE_DIR}/utils/*.hpp")
file(GLOB_RECURSE UTILS_SOURCES **.cpp)

# For debug
message(building '${CLOUD_STORAGE_CORE_NAME}-utils' ...)
message('${CLOUD_STORAGE_CORE_NAME}-utils': include files: ${UTILS_HEADERS})
message('${CLOUD_STORAGE_CORE_NAME}-utils': sources files: ${UTILS_SOURCES})
message("cloud-storage's path to 'include' directory: ${CLOUD_STORAGE_INCLUDE_DIR}")

find_package(Boost REQUIRED)

add_library("${CLOUD_STORAGE_CORE_NAME}-utils" ${UTILS_HEADERS} ${UTILS_SOURCES})
target_link_libraries(
  "${CLOUD_STORAGE_CORE_NAME}-utils"
  pthread
  rt
  Boost::boost
)

target_include_directories(
  "${CLOUD_STORAGE_CORE_NAME}-utils" PUBLIC
  "${CLOUD_STORAGE_INCLUDE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_BINARY_DIR}"
)

add_library(FLK::CoreUtils ALIAS "${CLOUD_STORAGE_CORE_NAME}-utils")

set_target_properties(
  "${CLOUD_STORAGE_CORE_NAME}-utils"
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  LINKER_LANGUAGE CXX
  EXPORT_NAME CoreUtils
)
