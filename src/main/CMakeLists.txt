include(FetchContent)

message(STATUS "Try to find OpenESL by FetchContent")
set(OPENESL_USE_COMMON4ESL ON)
set(OPENESL_USE_LOGBOOK4ESL ON)
set(OPENESL_USE_CURL4ESL ON)
set(OPENESL_USE_MHD4ESL ON)
set(OPENESL_USE_ZSYSTEM4ESL ON)
FetchContent_Declare(
    openesl
    GIT_REPOSITORY https://github.com/slukasde/open-esl
    #GIT_TAG        v1.6.0
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)
find_package(openesl REQUIRED)

find_package(Boost COMPONENTS system filesystem REQUIRED)
#find_package(Boost REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    openesl::openesl
    Boost::system 
    Boost::filesystem
    tinyxml2::tinyxml2)
