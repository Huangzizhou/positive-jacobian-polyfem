# Robust Bezier subdivision (https://gitlab.com/fsichetti/robust-bezier-subdivision)
# License: MIT

if(TARGET bezier)
    return()
endif()

message(STATUS "Third-party: creating target 'bezier'")


option(PARAVIEW_OUTPUT "Export elements to Paraview" OFF)
option(HDF5_INTERFACE "Process HDF5 datasets" OFF)
option(IPRED_ARITHMETIC "Use the efficient Indirect Predicates library" ON)
if (IPRED_ARITHMETIC)
    add_compile_definitions(IPRED_ARITHMETIC)
endif()

include(CPM)
CPMAddPackage("https://gitlab.com/fsichetti/robust-bezier-subdivision.git#9241344df7694375f6a9b5ea5cd2d5be2a756ef9")

set_target_properties(bezier PROPERTIES CXX_STANDARD 20)
