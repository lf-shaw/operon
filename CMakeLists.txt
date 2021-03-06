cmake_minimum_required(VERSION 3.11)

include(FetchContent)
include(FeatureSummary)

project(
    Operon
    VERSION 0.1
    DESCRIPTION "Genetic Programming"
    LANGUAGES CXX
)

find_package(Git)
find_package(cxxopts)
find_package(fmt)
find_package(Eigen3)
find_package(Ceres)
find_package(Threads)
find_package(TBB)
find_package(Python3 COMPONENTS Development Interpreter)
find_package(pybind11)
find_package(doctest)

# operon library
set_package_properties(Git     PROPERTIES TYPE REQUIRED)
set_package_properties(fmt     PROPERTIES TYPE REQUIRED)
set_package_properties(Eigen3  PROPERTIES TYPE REQUIRED)
set_package_properties(Tbb     PROPERTIES TYPE REQUIRED)
set_package_properties(Threads PROPERTIES TYPE REQUIRED)

set_package_properties(Ceres   PROPERTIES TYPE OPTIONAL) # non-linear least squares
set_package_properties(doctest PROPERTIES TYPE OPTIONAL) # unit tests
set_package_properties(cxxopts PROPERTIES TYPE OPTIONAL) # command-line parser

# python binding
set_package_properties(Python3 PROPERTIES TYPE OPTIONAL)
set_package_properties(pybind11 PROPERTIES TYPE OPTIONAL)

# get revision information from git
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    INCLUDE(cmake/GetGitRevisionDescription.cmake)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD OUTPUT_VARIABLE SHORT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(REVISION ${SHORT_SHA} CACHE STRING "git short sha" FORCE)

    # only use the plugin to tie the configure state to the sha to force rebuilds
    # of files that depend on version.h
    get_git_head_revision(REFSPEC COMMITHASH.cmake)
else()
    message(WARNING "Git not found, cannot set version info")
    SET(REVISION "unknown")
endif()


FetchContent_Declare(
    gsl
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG master
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/GSL
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/GSL
)

FetchContent_Declare(
    csv
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/CSV/include
    URL https://raw.githubusercontent.com/d99kris/rapidcsv/master/src/rapidcsv.h
    DOWNLOAD_NO_EXTRACT 1
)

FetchContent_Declare(
    nanobench
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/nanobench/include
    URL https://raw.githubusercontent.com/martinus/nanobench/master/src/include/nanobench.h
    DOWNLOAD_NO_EXTRACT 1
)

FetchContent_Declare(
    xxhash
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/xxhash
    URL https://raw.githubusercontent.com/Cyan4973/xxHash/dev/xxhash.h
    DOWNLOAD_NO_EXTRACT 1
)

FetchContent_Declare(
    ceres_tiny_solver
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/tiny_solver
    URL https://raw.githubusercontent.com/ceres-solver/ceres-solver/master/include/ceres/tiny_solver.h
    DOWNLOAD_NO_EXTRACT 1
)

FetchContent_Declare(
    ceres_jet
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/tiny_solver
    URL https://raw.githubusercontent.com/ceres-solver/ceres-solver/master/include/ceres/jet.h
    PATCH_COMMAND sed -i /port\.h/d ${PROJECT_SOURCE_DIR}/thirdparty/tiny_solver/jet.h
    DOWNLOAD_NO_EXTRACT 1
)

# populate gsl this way so we can pass EXCLUDE_FROM_ALL
# (since by default it specifies an install cmake target polluting our install dir)
if(NOT gsl_POPULATED)
    FetchContent_Populate(gsl)
    add_subdirectory(${gsl_SOURCE_DIR} ${gsl_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# populate content
FetchContent_MakeAvailable(xxhash csv nanobench)

# populate optional content
FetchContent_MakeAvailable(ceres_tiny_solver ceres_jet)

set(THIRDPARTY_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/thirdparty/GSL/include
    ${PROJECT_SOURCE_DIR}/thirdparty/CSV/include
    ${PROJECT_SOURCE_DIR}/thirdparty/nanobench/include
    ${PROJECT_SOURCE_DIR}/thirdparty/xxhash
    ${PROJECT_SOURCE_DIR}/thirdparty/tiny_solver
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/operon/core/buildinfo.hpp.in
               ${CMAKE_BINARY_DIR}/buildinfo.hpp)

# compile and link-time options
set(BUILD_TESTS_DESCRIPTION          "Build unit tests [default=OFF].")
set(BUILD_PYBIND_DESCRIPTION         "Build python bindings [default=OFF].")
set(OPENLIBM_DESCRIPTION             "Link against Julia's openlibm, a high performance mathematical library [default=OFF].")
set(JEMALLOC_DESCRIPTION             "Link against jemalloc, a general purpose malloc(3) implementation that emphasizes fragmentation avoidance and scalable concurrency support [default=OFF].")
set(TCMALLOC_DESCRIPTION             "Link against tcmalloc (thread-caching malloc), a malloc(3) implementation that reduces lock contention for multi-threaded programs [default=OFF].")
set(MIMALLOC_DESCRIPTION             "Link against mimalloc, a general purpose allocator with excellent performance characteristics [default=OFF].")
set(USE_SINGLE_PRECISION_DESCRIPTION "Perform model evaluation using floats (single precision) instead of doubles. Great for reducing runtime, might not be appropriate for all purposes [default=OFF].")
set(CERES_TINY_SOLVER_DESCRIPTION    "Use the tiny solver included in Ceres, intended for solving small dense problems with low latency and low overhead [default=OFF].")
set(CERES_ALWAYS_DOUBLE_DESCRIPTION  "Always use double-precision for the scalar part of a jet. If not set then the value of USE_SINGLE_PRECISION is used [default=ON].")

# option descriptions
option(BUILD_TESTS          ${BUILD_TESTS_DESCRIPTION}          OFF)
option(BUILD_PYBIND         ${BUILD_PYBIND_DESCRIPTION}         OFF)
option(USE_OPENLIBM         ${OPENLIBM_DESCRIPTION}             OFF)
option(USE_JEMALLOC         ${JEMALLOC_DESCRIPTION}             OFF)
option(USE_TCMALLOC         ${TCMALLOC_DESCRIPTION}             OFF)
option(USE_MIMALLOC         ${MIMALLOC_DESCRIPTION}             OFF)
option(USE_SINGLE_PRECISION ${USE_SINGLE_PRECISION_DESCRIPTION} OFF)
option(CERES_TINY_SOLVER    ${CERES_TINY_SOLVER_DESCRIPTION}    OFF)
option(CERES_ALWAYS_DOUBLE  ${CERES_ALWAYS_DOUBLE_DESCRIPTION}  OFF)

add_feature_info(BUILD_TESTS          BUILD_TESTS          ${BUILD_TESTS_DESCRIPTION})
add_feature_info(BUILD_PYBIND         BUILD_PYBIND         ${BUILD_PYBIND_DESCRIPTION})
add_feature_info(USE_OPENLIBM         USE_OPENLIBM         ${OPENLIBM_DESCRIPTION})
add_feature_info(USE_JEMALLOC         USE_JEMALLOC         ${JEMALLOC_DESCRIPTION})
add_feature_info(USE_TCMALLOC         USE_TCMALLOC         ${TCMALLOC_DESCRIPTION})
add_feature_info(USE_MIMALLOC         USE_MIMALLOC         ${MIMALLOC_DESCRIPTION})
add_feature_info(USE_SINGLE_PRECISION USE_SINGLE_PRECISION ${USE_SINGLE_PRECISION_DESCRIPTION})
add_feature_info(CERES_TINY_SOLVER    CERES_TINY_SOLVER    ${CERES_TINY_SOLVER_DESCRIPTION})
add_feature_info(CERES_ALWAYS_DOUBLE  CERES_ALWAYS_DOUBLE  ${CERES_ALWAYS_DOUBLE_DESCRIPTION})

if(USE_OPENLIBM)
    find_library(OPENLIBM openlibm)
    if(NOT OPENLIBM)
        message(WARNING "Option USE_OPENLIBM was specified, but openlibm could not be found.")
        set(OPENLIBM "")
    else ()
        message(STATUS "Option USE_OPENLIBM was specified, found openlibm at ${OPENLIBM}.")
    endif()
endif()

set(COUNTER 0)
if(USE_JEMALLOC)
    math(EXPR COUNTER "${COUNTER}+1")
    find_library(JEMALLOC jemalloc)
    if(NOT JEMALLOC)
        message(WARNING "Option USE_JEMALLOC was specified, but jemalloc could not be found.")
        set(JEMALLOC "")
    else()
        message(STATUS "Option USE_JEMALLOC was specified, found jemalloc at ${JEMALLOC}.")
    endif()
endif()

if(USE_TCMALLOC)
    math(EXPR COUNTER "${COUNTER}+1")
    find_library(TCMALLOC tcmalloc)
    if(NOT TCMALLOC)
        message(WARNING "Option USE_TCMALLOC was specified, but tcmalloc could not be found.")
        set(TCMALLOC "")
    else()
        message(STATUS "Option USE_TCMALLOC was specified, found tcmalloc at ${TCMALLOC}.")
    endif()
endif()

if(USE_MIMALLOC)
    math(EXPR COUNTER "${COUNTER}+1")
    find_library(MIMALLOC mimalloc)
    if(NOT MIMALLOC)
        message(WARNING "Option USE_MIMALLOC was specified, but mimalloc could not be found.")
        set(MIMALLOC "")
    else()
        message(STATUS "Option USE_MIMALLOC was specified, found mimalloc at ${MIMALLOC}.")
    endif()
endif()

if(COUNTER GREATER 1)
    message(FATAL_ERROR "Options USE_JEMALLOC, USE_TCMALLOC and USE_MIMALLOC are mutually exclusive. Please specify only one.")
endif()

set(MALLOC_LIB ${JEMALLOC} ${TCMALLOC} ${MIMALLOC})

if(USE_SINGLE_PRECISION)
    message(STATUS "Option USE_SINGLE_PRECISION was specified, single-precision model evaluation will be used.")
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# print a status of what we found
feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:" QUIET_ON_EMPTY)
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:" QUIET_ON_EMPTY)

add_library(
    operon
    STATIC
    src/core/tree.cpp
    src/core/problem.cpp
    src/core/dataset.cpp
    src/core/pset.cpp
    src/hash/metrohash64.cpp
    src/operators/crossover.cpp
    src/operators/mutation.cpp
    src/operators/creator/balanced.cpp
    src/operators/creator/koza.cpp
    src/operators/creator/ptc2.cpp
    src/operators/selector/proportional.cpp
    src/operators/selector/tournament.cpp
    src/stat/meanvariance.cpp
    src/stat/pearson.cpp
)
target_compile_features(operon PRIVATE cxx_std_17)
target_link_libraries(operon PRIVATE fmt::fmt ${OPENLIBM} ${MALLOC_LIB} ${CERES_LIBRARIES} Threads::Threads TBB::tbb)
target_include_directories(operon PRIVATE ${PROJECT_SOURCE_DIR}/include/operon ${PROJECT_BINARY_DIR})
target_include_directories(operon SYSTEM PRIVATE ${EIGEN3_INCLUDE_DIR} ${CERES_INCLUDE_DIRS} ${THIRDPARTY_INCLUDE_DIRS})
target_compile_definitions(operon PRIVATE
    "$<$<BOOL:${USE_SINGLE_PRECISION}>:USE_SINGLE_PRECISION>"
    "$<$<BOOL:${CERES_TINY_SOLVER}>:CERES_TINY_SOLVER>"
    "$<$<BOOL:${CERES_ALWAYS_DOUBLE}>:CERES_ALWAYS_DOUBLE>"
    "$<$<BOOL:${Ceres_FOUND}>:HAVE_CERES>"
    )

add_executable(
    operon-example-gp
    examples/gp.cpp
    )

target_compile_features(operon-example-gp PRIVATE cxx_std_17)
target_link_libraries(operon-example-gp PRIVATE operon fmt::fmt ${OPENLIBM} ${MALLOC_LIB} ${CERES_LIBRARIES} TBB::tbb)
target_include_directories(operon-example-gp SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/include/operon ${THIRDPARTY_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} ${CERES_INCLUDE_DIRS})
target_compile_definitions(operon-example-gp PRIVATE "$<$<BOOL:${USE_SINGLE_PRECISION}>:USE_SINGLE_PRECISION>")
set_target_properties(operon-example-gp PROPERTIES EXCLUDE_FROM_ALL TRUE)

if(MSVC)
    target_compile_options(operon PRIVATE /W4 "$<$<CONFIG:Release>:/O3;/std:c++latest>")
    # ugly hack to set the include dir correctly
    find_path(FMT_INCLUDE_DIR fmt/core.h)
    include_directories(${FMT_INCLUDE_DIR})
else()
    set(MYFLAGS -Wall -Werror -Wextra -Wshadow -Wsign-compare -Wshadow -Wwrite-strings -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion)
    target_compile_options(operon PRIVATE ${MYFLAGS} "$<$<CONFIG:Debug>:-g;>$<$<CONFIG:Release>:-O3;-g;-march=native>;-fPIC")
    target_compile_options(operon-example-gp PRIVATE ${MYFLAGS} "$<$<CONFIG:Debug>:-g>$<$<CONFIG:Release>:-O3;-g;-march=native>")
endif(MSVC)

if (cxxopts_FOUND)
    #binary for GP algorithm cli version
    add_executable(
        operon-gp
        src/cli/operon_gp.cpp
    )
    target_compile_features(operon-gp PRIVATE cxx_std_17)
    target_link_libraries(operon-gp PRIVATE operon fmt::fmt cxxopts::cxxopts ${OPENLIBM} ${MALLOC_LIB} ${CERES_LIBRARIES} TBB::tbb)
    target_include_directories(operon-gp SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/include/operon
        ${THIRDPARTY_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
        ${CERES_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        )
    target_compile_definitions(operon-gp PRIVATE
        "$<$<BOOL:${USE_SINGLE_PRECISION}>:USE_SINGLE_PRECISION>"
        "$<$<BOOL:${CERES_TINY_SOLVER}>:CERES_TINY_SOLVER>"
        "$<$<BOOL:${CERES_ALWAYS_DOUBLE}>:CERES_ALWAYS_DOUBLE>"
        "$<$<BOOL:${Ceres_FOUND}>:HAVE_CERES>"
        )
    set_target_properties(operon-gp PROPERTIES EXCLUDE_FROM_ALL TRUE)

    if(MSVC)
        target_compile_options(operon-gp PRIVATE /W4 "$<$<CONFIG:Release>:/O3;/std:c++latest>")
    else()
        target_compile_options(operon-gp PRIVATE ${MYFLAGS} "$<$<CONFIG:Debug>:-g>$<$<CONFIG:Release>:-O3;-g;-march=native>")
    endif()
endif()

if(Python3_FOUND AND pybind11_FOUND AND BUILD_PYBIND)
    pybind11_add_module(
        pyoperon
        SHARED
        src/python/operon.cpp
        src/python/algorithm.cpp
        src/python/creator.cpp
        src/python/crossover.cpp
        src/python/dataset.cpp
        src/python/eval.cpp
        src/python/generator.cpp
        src/python/mutation.cpp
        src/python/node.cpp
        src/python/problem.cpp
        src/python/pset.cpp
        src/python/reinserter.cpp
        src/python/selection.cpp
        src/python/tree.cpp
        )

    target_compile_features(pyoperon PRIVATE cxx_std_17)
    target_link_libraries(pyoperon PRIVATE operon)
    if(MSVC)
        target_compile_options(pyoperon PRIVATE /W4 "$<$<CONFIG:Release>:/O3;/std:c++latest>")
    else()
        target_compile_options(pyoperon PRIVATE ${MYFLAGS} "$<$<CONFIG:Debug>:-g;-fPIC>$<$<CONFIG:Release>:-O3;-g;-fPIC;-march=native>")
    endif()
    target_include_directories(pyoperon PRIVATE ${PROJECT_SOURCE_DIR}/python)
    target_include_directories(
        pyoperon
        SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/include/operon
        ${THIRDPARTY_INCLUDE_DIRS}
        ${Python3_INCLUDE_DIRS}
        ${CERES_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
        ${CMAKE_BINARY_DIR}
    )
    target_compile_definitions(pyoperon PRIVATE
        "$<$<BOOL:${USE_SINGLE_PRECISION}>:USE_SINGLE_PRECISION>"
        "$<$<BOOL:${CERES_TINY_SOLVER}>:CERES_TINY_SOLVER>"
        "$<$<BOOL:${CERES_ALWAYS_DOUBLE}>:CERES_ALWAYS_DOUBLE>"
        "$<$<BOOL:${Ceres_FOUND}>:HAVE_CERES>"
        )
    set_target_properties(pyoperon PROPERTIES OUTPUT_NAME "_operon")

    # for installation we want to detect the python path and then copy our package over
    # find python path
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "if True:
        from distutils import sysconfig as sc
        print(sc.get_python_lib(prefix='', plat_specific=True))"
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(PYOPERON_INSTALL_PATH ${Python3_SITELIB})
    else()
        set(PYOPERON_INSTALL_PATH ${CMAKE_INSTALL_PREFIX})
    endif()
    message(STATUS "Python bindings will be installed in ${PYOPERON_INSTALL_PATH}")
    # install python packages
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/python/operon
        DESTINATION ${PYOPERON_INSTALL_PATH}
        )
    # install python bindings library
    install(
        TARGETS pyoperon
        DESTINATION ${PYOPERON_INSTALL_PATH}/operon
        )
endif()

if(doctest_FOUND AND BUILD_TESTS)
    add_executable(
        operon-test
        test/test.cpp
        test/nanobench.cpp
        test/performance/evaluation.cpp
        test/performance/initialization.cpp
        test/performance/evolution.cpp
        test/performance/hashing.cpp
        test/performance/distance.cpp
        test/performance/math.cpp
        test/performance/random.cpp
        test/performance/stat.cpp
        #test/implementation/evaluation.cpp
        test/implementation/details.cpp
        test/implementation/hashing.cpp
        test/implementation/initialization.cpp
        test/implementation/mutation.cpp
        test/implementation/random.cpp
        test/implementation/stat.cpp
        #test/implementation/selection.cpp
        #        test/codegen/irbuilder.cpp
        )
    target_compile_features(operon-test PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(operon-test PRIVATE /W4 "$<$<CONFIG:Release>:/O3;/std:c++latest>")
    else()
        target_compile_options(operon-test PRIVATE ${MYFLAGS} "$<$<CONFIG:Debug>:-g>$<$<CONFIG:Release>:-O3;-g;-march=native>")
    endif(MSVC)
    target_link_libraries(operon-test PRIVATE operon fmt::fmt ${OPENLIBM} ${MALLOC_LIB} ${CERES_LIBRARIES} TBB::tbb doctest::doctest)
    target_include_directories(
        operon-test
        SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/include/operon
        ${THIRDPARTY_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
        ${CERES_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        )
    target_compile_definitions(operon-test PRIVATE
        "$<$<BOOL:${USE_SINGLE_PRECISION}>:USE_SINGLE_PRECISION>"
        "$<$<BOOL:${CERES_TINY_SOLVER}>:CERES_TINY_SOLVER>"
        "$<$<BOOL:${CERES_ALWAYS_DOUBLE}>:CERES_ALWAYS_DOUBLE>"
        "$<$<BOOL:${Ceres_FOUND}>:HAVE_CERES>"
        )
    set_target_properties(operon-test PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

