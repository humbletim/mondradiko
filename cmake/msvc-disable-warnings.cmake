set(MSVC_DISABLED_WARNINGS_LIST
  C4061 # enumerator '...' in switch of enum '...' is not explicitly handled by a case label
  C4099 # '...': type name first seen using '...' now seen using '...'
  C4100 # '...': unreferenced formal parameter
  C4189 # '...': local variable is initialized but not referenced
  C4191 # '...': unsafe conversion from '...' to '...'
  C4201 # nonstandard extension used: nameless struct/union
  C4242 # '...': conversion from '...' to '...', possible loss of data
  C4244 # '...': conversion from '...' to '...', possible loss of data
  C4245 # '...': conversion from '...' to '...', signed/unsigned mismatch
  C4265 # '...': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
  C4267 # '...': conversion from '...' to '...', possible loss of data
  C4305 # '...': truncation from '...' to '...'
  C4365 # '...': conversion from '...' to '...', signed/unsigned mismatch
  C4458 # declaration of '...' hides class member
  C4464 # relative include path contains '...'
  C4514 # '...': unreferenced inline function has been removed
  C4582 # '...': constructor is not implicitly called
  C4583 # '...': destructor is not implicitly called
  C4623 # '...': default constructor was implicitly defined as deleted
  C4625 # '...': copy constructor was implicitly defined as deleted
  C4626 # '...': assignment operator was implicitly defined as deleted
  C4668 # '...' is not defined as a preprocessor macro, replacing with '...' for '...'
  C4702 # unreachable code
  C4710 # '...': function not inlined
  C4710 # '...'::`2'...'::`2'...': function not inlined
  C4711 # function '...' selected for automatic inline expansion
  C4774 # '...' : format string expected in argument 3 is not a string literal
  C4820 # '...': '...' bytes padding added after data member '...'
  C4866 # compiler may not enforce left-to-right evaluation order for call to '...'
  C4868 # compiler may not enforce left-to-right evaluation order in braced initializer list
  C5026 # '...': move constructor was implicitly defined as deleted
  C5027 # '...': move assignment operator was implicitly defined as deleted
  C5039 # '...': pointer or reference to potentially throwing function passed to '...' function under -EHc. Undefined behavior may occur if this function throws an exception.
  C5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
  C5204 # '...': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
  C5219 # implicit conversion from '...' to '...', possible loss of data
)
LIST(JOIN MSVC_DISABLED_WARNINGS_LIST " " MSVC_MSG )
if (MSVC_DISABLE_KNOWN_WARNINGS)
  message(STATUS "FIXME: DISABLING LOTS OF C++ WARNINGS IN ORDER TO TROUBLESHOOT GHA CI\n ${MSVC_MSG}")
  string(REPLACE "C" " -wd" MSVC_DISABLED_WARNINGS_STR ${MSVC_DISABLED_WARNINGS_LIST})
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
endif()
