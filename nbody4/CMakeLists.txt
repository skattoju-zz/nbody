CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(oclSimpleGL)

#need to include FindOPENCL.cmake to
SET( CMAKE_MODULE_PATH ${oclSimpleGL_SOURCE_DIR}/cmake_modules)

message("CMake module path: ${CMAKE_MODULE_PATH}\n")

FIND_PACKAGE (OPENCL)
#Need some OpenGL packages
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(GLUT)
FIND_PACKAGE(GLEW)

INCLUDE_DIRECTORIES( 
    ${GLUT_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${oclSimpleGL_SOURCE_DIR}/opencl10
)

message("include directories: \n")
get_directory_property(includes INCLUDE_DIRECTORIES)
message("${includes}\n")

#build in debug mode
ADD_DEFINITIONS(-g)

#the source files we want to compile into the library
set (LIBCXXFILES utils.cpp)
#set a CMake variable to name the library
SET(utillib oclSimpleGL)
ADD_LIBRARY(${utillib} ${LIBCXXFILES})


#create the executable
SET(EXEC oclNbody.x)
ADD_EXECUTABLE(${EXEC} oclSimpleGL.cpp)

TARGET_LINK_LIBRARIES (${EXEC}
   ${utillib}
   ${GLUT_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${GLEW_LIBRARY}
   ${OPENCL_LIBRARIES}
)
