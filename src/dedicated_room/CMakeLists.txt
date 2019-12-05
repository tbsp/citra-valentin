set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_executable(citra-room
    citra-room.cpp
    citra-room.rc
)

create_target_directory_groups(citra-room)

# So Discord doesn't say it's a official build
set_target_properties(citra-room PROPERTIES OUTPUT_NAME "citra-valentin-room")

target_link_libraries(citra-room PRIVATE common core network web_service)
target_link_libraries(citra-room PRIVATE cryptopp glad semver)

if (MSVC)
    target_link_libraries(citra-room PRIVATE getopt)
endif()

target_link_libraries(citra-room PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)
