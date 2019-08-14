#------------------------------------------------------------------------
# Function to help generate the C++ flatbuffers
#------------------------------------------------------------------------

set(Flatbuffers_DIR "/snap/flatbuffers/current/lib/cmake/flatbuffers")
find_package(Flatbuffers CONFIG REQUIRED)

#-----------------------------------------------------------------------------
# Set the appropriate Flatbuffers directories
#-----------------------------------------------------------------------------
get_filename_component(Flatbuffers_BASE_DIR "${Flatbuffers_DIR}/../../.." ABSOLUTE)
set(Flatbuffers_INCLUDE_DIR "${Flatbuffers_BASE_DIR}/include")
set(Flatbuffers_FLATC_EXE "${Flatbuffers_BASE_DIR}/bin/flatc")


#-----------------------------------------------------------------------------
# a function to generate language specific flatbuffer files
#-----------------------------------------------------------------------------

function(flatbuffers_generate_headers NAME OUTPUT_DIR)
  set(FLATC_OUTPUTS)
  set(${NAME})
  foreach(INFILE ${ARGN})
    get_filename_component(BASENAME ${INFILE} NAME_WE)
    set(OUTFILE "${OUTPUT_DIR}/${BASENAME}_generated.h")

    message("Adding flatc generation: ${INFILE} => ${OUTFILE}")

    list(APPEND FLATC_OUTPUTS ${OUTFILE})
    add_custom_command(
      COMMAND ${Flatbuffers_FLATC_EXE} -c -o ${OUTPUT_DIR} ${INFILE}
      DEPENDS ${INFILE}
      OUTPUT ${OUTFILE}
      COMMENT "Generating C++ header for ${INFILE}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
  set(${NAME} ${FLATC_OUTPUTS} PARENT_SCOPE)
endfunction()
