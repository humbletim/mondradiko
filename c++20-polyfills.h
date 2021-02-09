#ifndef cpp20_polyfills_h
#define cpp20_polyfills_h

// FIXME(humbletim) - replace with() occurences with proper "designated structure initializers"
//   ... which require >= std:c++20 -- for now this approach lets windows builds proceed and should
//   ... be relatively easy to revert back to real designated initializers later on
//   ... see: https://stackoverflow.com/a/49572324/1684079
#define with(T, ...) \
    ([&]{ T ${}; __VA_ARGS__; return $; }())

#endif