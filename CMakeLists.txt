     

 
file(GLOB SRC_cc "${CMAKE_CURRENT_SOURCE_DIR}/driver/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/embed/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/hijack/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/util/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"  "${CMAKE_CURRENT_SOURCE_DIR}/engine/model/*.cc")  


include_directories( "${CMAKE_CURRENT_SOURCE_DIR}") 
                                
add_library(luna MODULE ${SRC_cpp} ${SRC_cc})



target_link_libraries(luna libs shlwapi.lib ntdll.lib) 
set_target_properties(luna PROPERTIES OUTPUT_NAME "LunaEmbedEngine32")