cmake_minimum_required(VERSION 3.0.0)

project(seamless-compression VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})

find_package(Qt5 COMPONENTS Gui REQUIRED)

set(EIGENDIR "${CMAKE_CURRENT_LIST_DIR}/src/eigenlib")

set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/compressed_image.cpp
    src/image.cpp
    src/image_io.cpp
    src/line.cpp
    src/lineareq_eigen.cpp
    src/main.cpp
    src/mesh.cpp
    src/mesh_io.cpp
    src/pyramid.cpp
    src/solver.cpp
)

set(HEADERS
    src/compressed_image.h
    src/image.h
    src/line.h
    src/lineareq.h
    src/mesh.h
    src/metric.h
    src/pyramid.h
    src/solver.h
    src/vec3.h
)


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
#    Qt5::Core
    Qt5::Gui
#    Qt5::Widgets
)

include_directories(
    ${EIGENDIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

