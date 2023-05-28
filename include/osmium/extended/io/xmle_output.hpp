#pragma once

#include <osmium/io/writer.hpp> // IWYU pragma: export

#if defined(OSMIOEXTENSIONS)

#include <osmium/extended/io/detail/xmle_output_format.hpp> // IWYU pragma: export

#else

#warning "xmle Header used, but the available libosmium does not define OSMIOEXTENSIONS - Area IO is disabled!"

#include <osmium/io/detail/xml_output_format.hpp> // IWYU pragma: export

#endif