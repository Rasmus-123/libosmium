#pragma once

#include <osmium/io/pbf.hpp> // IWYU pragma: export
#include <osmium/io/reader.hpp> // IWYU pragma: export

#if defined(OSMIOEXTENSIONS)

#include <osmium/extended/io/detail/pbfe_input_format.hpp> // IWYU pragma: export

#else

#warning "PBFE Header used, but the available libosmium does not define OSMIOEXTENSIONS - Area IO is disabled!"

#include <osmium/io/detail/pbf_input_format.hpp> // IWYU pragma: export

#endif