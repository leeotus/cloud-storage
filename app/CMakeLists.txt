set(APP_NAME "filekeeper")
add_executable(${APP_NAME} main.cpp)

target_link_libraries(
  "${APP_NAME}"
  FLK::CoreUtils
  FLK::CoreNet
  stdc++fs
  mysqlclient
)

target_include_directories(
  "${APP_NAME}" PUBLIC
  "${CLOUD_STORAGE_INCLUDE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_BINARY_DIR}"
)


