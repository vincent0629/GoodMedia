include_directories($(PROJECT_SOURCE_DIR)../graph $(PROJECT_SOURCE_DIR)../sources)
add_executable(MediaParser MediaInfo.cpp MediaParser.cpp)
target_link_libraries(MediaParser graph sources ConvertUTF)
