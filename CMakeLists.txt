cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(ParticlePM VERSION 0.1.5 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr/ CACHE PATH "Default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.65 REQUIRED COMPONENTS program_options)

include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB ppm_SRC ${CMAKE_CURRENT_LIST_DIR}/particlepm/*.cpp)
file(GLOB ppm_HDR ${CMAKE_CURRENT_LIST_DIR}/particlepm/*.hpp)

add_library(libparticlepm SHARED ${ppm_SRC})
set_target_properties(libparticlepm PROPERTIES OUTPUT_NAME particlepm)
target_link_libraries(libparticlepm dl)

add_executable(particlepm ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(particlepm libparticlepm dl ${Boost_LIBRARIES})
target_include_directories(particlepm PRIVATE ${Boost_INCLUDE_DIRS})

install(
  TARGETS libparticlepm particlepm
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/particlepm.hpp
  DESTINATION include
)

install(
  FILES ${ppm_HDR}
  DESTINATION include/particlepm
)