#pragma once

#include <osmium/io/reader.hpp> // IWYU pragma: export

#if defined(OSMIOEXTENSIONS)

#include <osmium/extended/io/detail/xmle_input_format.hpp> // IWYU pragma: export

#else

#warning "xmle Header used, but the available libosmium does not define OSMIOEXTENSIONS - Area IO is disabled!"

#include <osmium/io/detail/xml_input_format.hpp> // IWYU pragma: export

#endif