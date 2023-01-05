# CMakeLists.txt for building from the Tessalatrix source

set(APP_NAME tessalatrix)

# Add the executable, and list all the source that goes into it
add_executable(
  ${APP_NAME}
  config.c log.c tessalatrix.c
)

# Make sure it's built in the top level directory
set_target_properties(
  ${APP_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

# Tell CMake where else to look for includes (such as config.h)
target_include_directories(${APP_NAME} PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(${APP_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/vendor/optparse")

# Add in the SDL requirements
find_package(SDL2 REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE "${SDL2_LIBRARIES}")
target_include_directories(${APP_NAME} PRIVATE "${SDL2_INCLUDE_DIRS}")
