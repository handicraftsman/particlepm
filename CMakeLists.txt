cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(ParticlePM VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr/ CACHE PATH "Default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(libparticlepm SHARED ${CMAKE_CURRENT_LIST_DIR}/particlepm.cpp)
set_target_properties(libparticlepm PROPERTIES OUTPUT_NAME particlepm)

add_executable(particlepm ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(libparticlepm)

install(
  TARGETS libparticlepm particlepm
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/particlepm.hpp
  DESTINATION include
)