
project(netpanel)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
src/NetCfg.cpp 
src/NetClass.cpp 
src/NetMix.cpp 
src/NetPanel.cpp
)

add_library (netpanel MODULE ${SOURCES})

target_link_libraries(netpanel utils far2l)

target_compile_definitions(netpanel PRIVATE -DWINPORT_DIRECT -DUNICODE -DFAR_DONT_USE_INTERNALS)

target_include_directories(netpanel PRIVATE .)
target_include_directories(netpanel PRIVATE ../far2l/far2sdk)
target_include_directories(netpanel PRIVATE ../WinPort)

set_target_properties(netpanel
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DIR}/Plugins/netpanel/plug"
        PREFIX ""
        SUFFIX ".far-plug-wide")

add_custom_command(TARGET netpanel POST_BUILD
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/configs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs "${INSTALL_DIR}/Plugins/netpanel"
)
