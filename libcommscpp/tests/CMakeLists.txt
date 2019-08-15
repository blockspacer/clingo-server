find_package(Threads REQUIRED)

set(source
  "${CMAKE_CURRENT_SOURCE_DIR}/main_test1.cpp"
  )

message("------------------------------------------------------")
message("COMMSCPP_INLCUDE_DIR: ${COMMSCPP_INCLUDE_DIR}")
message("CLINGOSERVER_SOURCE_DIR: ${CLINGOSERVER_SOURCE_DIR}")
message("------------------------------------------------------")

set(fbs_test_sources "${CMAKE_CURRENT_SOURCE_DIR}/test_schema.fbs")
flatbuffers_generate_headers(GENERATED_TEST_HEADERS ${CMAKE_CURRENT_BINARY_DIR} ${fbs_test_sources})
set_source_files_properties(${GENERATED_TEST_HEADERS} PROPERTIES GENERATED TRUE)
add_custom_target(build_test_messages ALL DEPENDS ${GENERATED_TEST_HEADERS})

#message("GENERATED_TEST_HEADERS: ${GENERATED_TEST_HEADERS}")

add_executable(main_test1 ${source})
add_dependencies(main_test1 build_test_messages)
target_link_libraries(main_test1 commscpp ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(main_test1 PUBLIC
  ${COMMSCPP_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CLINGOSERVER_SOURCE_DIR}/catch2"
  )
set_target_properties(main_test1 PROPERTIES FOLDER tests)

add_executable(server_test "${CMAKE_CURRENT_SOURCE_DIR}/server_test.cpp")
add_dependencies(server_test build_test_messages)
target_link_libraries(server_test commscpp ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(server_test PUBLIC
  ${COMMSCPP_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  )

add_executable(client_test "${CMAKE_CURRENT_SOURCE_DIR}/client_test.cpp")
add_dependencies(client_test build_test_messages)
target_link_libraries(client_test commscpp ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(client_test PUBLIC
  ${COMMSCPP_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  )


add_test(NAME main_test1 COMMAND main_test1)
