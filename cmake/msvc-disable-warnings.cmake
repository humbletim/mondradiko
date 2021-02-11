set(MSVC_DISABLED_WARNINGS_LIST
  "C4710" # function '<x>' not inlined
  "C4711" # function <x> selected for inline expansion
  "C4464" # relative include path contains '..'
  "C4514" #unreferenced inline function has been removed
  "C4244" #conversion from 'double' to 'T', possible loss of data
  "C5045" "C4820" "C5027" "C4623" "C5026" "C4265" "C4625" "C4626" "C4365" "C4583" "C4267" "C4774" "C4061" "C4201" "C4242" "C4100" "C5204" "C4702" "C4668" "C4305" "C5039" "C5219" "C4191" "C4458" "C4582"
)
message(WARNING "FIXME: DISABLING LOTS OF C++ WARNINGS IN ORDER TO TROUBLESHOOT GHA CI... UNDO! ${MSVC_DISABLED_WARNINGS_LIST}")
string(REPLACE "C" " -wd" MSVC_DISABLED_WARNINGS_STR ${MSVC_DISABLED_WARNINGS_LIST})
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
