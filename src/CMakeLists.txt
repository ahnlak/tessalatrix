# CMakeLists.txt for building from the Tessalatrix source

set(APP_NAME tessalatrix)

# Add the executable, and list all the source that goes into it
add_executable(
  ${APP_NAME}
  config.c display.c log.c splash.c tessalatrix.c util.c
)

# Make sure it's built in the top level directory
set_target_properties(
  ${APP_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

# Tell CMake the capabilities we need from the compiler (like C version)
target_compile_features(${APP_NAME} PRIVATE c_std_99)
add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")

# Tell CMake where else to look for includes (such as config.h)
target_include_directories(${APP_NAME} PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(${APP_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/vendor/optparse")

# Copy the assets too, into the build directory
add_custom_target(
  copy_assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${PROJECT_BINARY_DIR}/assets"
)
add_dependencies(${APP_NAME} copy_assets)

# Function stolen from the 32Blit SDK, for tracking down escaped SDL libraries.
function (find_sdl_lib lib_name header_name)
  string(TOUPPER ${lib_name} VAR_PREFIX)

  # new enough SDL_image/_net have cmake config support
  find_package(${lib_name} QUIET)
  if(TARGET ${lib_name}::${lib_name})
    set(${VAR_PREFIX}_LIBRARY ${lib_name}::${lib_name} PARENT_SCOPE)
    set(${VAR_PREFIX}_INCLUDE_DIR "" PARENT_SCOPE)

    if(WIN32)
      get_property(LIB_DLL TARGET ${lib_name}::${lib_name} PROPERTY IMPORTED_LOCATION)
      set(${VAR_PREFIX}_DLL ${LIB_DLL} PARENT_SCOPE)
    endif()

    return()
  endif()

  message("find_package(${lib_name}) failed, trying manual search...")

  find_path(${VAR_PREFIX}_INCLUDE_DIR ${header_name}
    HINTS ${SDL2_DIR} ${SDL2_DIR}/../../../
    PATH_SUFFIXES SDL2 include/SDL2 include
  )

  find_library(${VAR_PREFIX}_LIBRARY
    NAMES ${lib_name}
    HINTS ${SDL2_DIR} ${SDL2_DIR}/../../../
    PATH_SUFFIXES lib
  )

  if(NOT ${VAR_PREFIX}_INCLUDE_DIR OR NOT ${VAR_PREFIX}_LIBRARY)
    message(FATAL_ERROR "${lib_name} not found!")
  endif()

endfunction()

# Add in the SDL requirements
if(EMSCRIPTEN)

  target_compile_options(${APP_NAME} PRIVATE -sUSE_SDL=2 -sUSE_SDL_IMAGE=2)
  set(SDL2_LIBRARIES "-sUSE_SDL=2")
  set(SDL2_IMAGE_LIBRARY "-sUSE_SDL_IMAGE=2")

  set_target_properties(${APP_NAME} PROPERTIES
    SUFFIX ".html"
    LINK_FLAGS "-s ENVIRONMENT=web -s SDL2_IMAGE_FORMATS=['png'] --preload-file ${PROJECT_BINARY_DIR}/assets@/assets"
  )
  target_link_libraries(${APP_NAME} PRIVATE "-lidbfs.js") # include the persistent IndexedDB-based filesystem

else()

  find_package(SDL2 REQUIRED)
  find_sdl_lib(SDL2_image SDL_image.h)
  target_include_directories(${APP_NAME} PRIVATE "${SDL2_INCLUDE_DIRS}" "${SDL2_IMAGE_INCLUDE_DIR}")

endif()

target_link_libraries(${APP_NAME} PRIVATE "${SDL2_LIBRARIES}" "${SDL2_IMAGE_LIBRARY}")

# And set the built app as an install target
if(EMSCRIPTEN)
  install(FILES 
    ${PROJECT_BINARY_DIR}/${APP_NAME}.data
    ${PROJECT_BINARY_DIR}/${APP_NAME}.js 
    ${PROJECT_BINARY_DIR}/${APP_NAME}.wasm
    DESTINATION .
  )
else()
  install(DIRECTORY "${PROJECT_BINARY_DIR}/assets" DESTINATION . FILES_MATCHING PATTERN "*.png")
endif()

install(TARGETS ${APP_NAME} DESTINATION .)
