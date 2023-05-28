#pragma once

#include <osmium/io/pbf.hpp> // IWYU pragma: export
#include <osmium/io/writer.hpp> // IWYU pragma: export

#if defined(OSMIOEXTENSIONS)

#include <osmium/extended/io/detail/pbfe_output_format.hpp> // IWYU pragma: export

#else

#warning "PBFE Header used, but the available libosmium does not define OSMIOEXTENSIONS - Area IO is disabled!"

#include <osmium/io/detail/pbf_output_format.hpp> // IWYU pragma: export

#endif