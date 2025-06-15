include(FetchContent)

message(STATUS "Try to find GTX by FetchContent")
FetchContent_Declare(
    gtx
    GIT_REPOSITORY https://github.com/slukasde/gtx
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)
find_package(gtx REQUIRED)

message(STATUS "Try to find RapidJSON by FetchContent")
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/slukasde/rapidjson
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)
find_package(rapidjson REQUIRED)

message(STATUS "Try to find OpenESL by FetchContent")
set(OPENESL_USE_COMMON4ESL ON)
set(OPENESL_USE_LOGBOOK4ESL ON)
set(OPENESL_USE_CURL4ESL ON)
set(OPENESL_USE_MHD4ESL ON)
set(OPENESL_USE_ZSYSTEM4ESL ON)
FetchContent_Declare(
    openesl
    GIT_REPOSITORY https://github.com/slukasde/openesl
    #GIT_TAG        v1.6.0
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)
find_package(openesl REQUIRED)

#find_package(Boost COMPONENTS system filesystem REQUIRED)
#find_package(Boost REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    openesl::openesl
    #Boost::system 
    #Boost::filesystem
    gtx::gtx
    rapidjson::rapidjson
    tinyxml2::tinyxml2)
