# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(RCU-V2)
# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "html/login.html" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "html/upload_script.html" TEXT)
# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "html/scenario.html" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/mqtt_eclipseprojects_io.pem" TEXT)
