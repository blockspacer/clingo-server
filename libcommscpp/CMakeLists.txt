cmake_minimum_required(VERSION 3.1)
project(COMMSCPP)

find_package(Boost 1.70.0 REQUIRED system)


#-----------------------------------------------------------------------------
# Generate the Flatbuffers headers
#-----------------------------------------------------------------------------
set(cs_schema_dir "${CLINGOSERVER_SOURCE_DIR}/schema")

set(fbs_sources
  "${cs_schema_dir}/worker_write.fbs"
  "${cs_schema_dir}/application_msg.fbs"
  "${cs_schema_dir}/worker_ready_msg.fbs"
  "${cs_schema_dir}/worker_stopped_msg.fbs"
  )

flatbuffers_generate_headers(GENERATED_HEADERS ${COMMSCPP_BINARY_DIR} ${fbs_sources})
set_source_files_properties(${GENERATED_HEADERS} PROPERTIES GENERATED TRUE)
add_custom_target(build_messages ALL DEPENDS ${GENERATED_HEADERS})

#-----------------------------------------------------------------------------
#
#-----------------------------------------------------------------------------

set(COMMSCPP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" "${COMMSCPP_BINARY_DIR}")

add_library(commscpp INTERFACE)
target_link_libraries(commscpp INTERFACE ${Boost_LIBRARIES} flatbuffers::flatbuffers)




add_subdirectory(tests)
