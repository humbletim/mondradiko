set(MSVC_DISABLED_WARNINGS_LIST
  C4061
  C4099
  C4100
  C4189
  C4191
  C4201
  C4242
  C4244 #conversion from 'double' to 'T', possible loss of data
  C4245
  C4265
  C4267
  C4305
  C4365
  C4458
  C4464 # relative include path contains '..'
  C4514 #unreferenced inline function has been removed
  C4582
  C4583
  C4623
  C4625
  C4626
  C4668
  C4702
  C4710 # function '<x>' not inlined
  C4711 # function <x> selected for inline expansion
  C4774
  C4820
  C4866
  C4868
  C5026
  C5027
  C5039
  C5045
  C5204
  C5219
)
message(WARNING "FIXME: DISABLING LOTS OF C++ WARNINGS IN ORDER TO TROUBLESHOOT GHA CI... UNDO! ${MSVC_DISABLED_WARNINGS_LIST}")
string(REPLACE "C" " -wd" MSVC_DISABLED_WARNINGS_STR ${MSVC_DISABLED_WARNINGS_LIST})
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
