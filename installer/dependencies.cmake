cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0207)
 cmake_policy(SET CMP0207 NEW)
endif()
# Resolve PE imports recursively; system DLLs stay supplied by Windows.
file(GET_RUNTIME_DEPENDENCIES
 EXECUTABLES "${EXE}"
 DIRECTORIES "${CRT}"
 RESOLVED_DEPENDENCIES_VAR deps
 UNRESOLVED_DEPENDENCIES_VAR missing
 PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
 POST_EXCLUDE_REGEXES ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\].*")
if(missing)
 message(FATAL_ERROR "Unresolved runtime dependencies: ${missing}")
endif()
foreach(dep IN LISTS deps)
 file(COPY "${dep}" DESTINATION "${STAGE}")
endforeach()
