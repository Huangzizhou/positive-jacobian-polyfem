# Robust Bezier subdivision (https://gitlab.com/fsichetti/robust-bezier-subdivision)
# License: MIT

if(TARGET bezier)
    return()
endif()

message(STATUS "Third-party: creating target 'bezier'")


option(IPRED_ARITHMETIC "Use the efficient Indirect Predicates library" ON)
option(UNIT_TESTS "Run unit tests" ON)
option(LAGVEC_GCC_O0 "Disable optimization for some complicated functions" ON)
option(HDF5_INTERFACE "Process HDF5 datasets" OFF)
if (IPRED_ARITHMETIC)
    add_compile_definitions(IPRED_ARITHMETIC)
endif()
add_compile_definitions(EIGEN_INTERFACE)

include(CPM)
CPMAddPackage("https://gitlab.com/fsichetti/robust-bezier-subdivision.git#5d4fa77df6eab48c1da368e06dce1d388be0e5a1")

set_target_properties(bezier PROPERTIES CXX_STANDARD 17)
