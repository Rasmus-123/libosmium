#ifndef OSMIUM_IO_DETAIL_PROTOBUF_EXTENDED_TAGS_HPP
#define OSMIUM_IO_DETAIL_PROTOBUF_EXTENDED_TAGS_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2023 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <protozero/types.hpp>

namespace osmium::extended {

    namespace io {

        namespace detail {

            // directly translated from
            // https://github.com/openstreetmap/OSM-binary/blob/master/src/fileformat.proto

            namespace FileFormat {

                enum class Blob : protozero::pbf_tag_type {
                    optional_bytes_raw       = 1,
                    optional_int32_raw_size  = 2,
                    optional_bytes_zlib_data = 3,
                    optional_bytes_lzma_data = 4,
                    optional_bytes_lz4_data  = 6,
                    optional_bytes_zstd_data = 7
                };

                enum class BlobHeader : protozero::pbf_tag_type {
                    required_string_type     = 1,
                    optional_bytes_indexdata = 2,
                    required_int32_datasize  = 3
                };

            } // namespace FileFormat

            // directly translated from
            // https://github.com/openstreetmap/OSM-binary/blob/master/src/osmformat.proto

            namespace OSMFormat {

                enum class HeaderBlock : protozero::pbf_tag_type {
                    optional_HeaderBBox_bbox          =  1,
                    repeated_string_required_features =  4,
                    repeated_string_optional_features =  5,
                    optional_string_writingprogram    = 16,
                    optional_string_source            = 17,
                    optional_int64_osmosis_replication_timestamp       = 32,
                    optional_int64_osmosis_replication_sequence_number = 33,
                    optional_string_osmosis_replication_base_url       = 34
                };

                enum class HeaderBBox : protozero::pbf_tag_type {
                    required_sint64_left   = 1,
                    required_sint64_right  = 2,
                    required_sint64_top    = 3,
                    required_sint64_bottom = 4
                };

                enum class PrimitiveBlock : protozero::pbf_tag_type {
                    required_StringTable_stringtable       =  1,
                    repeated_PrimitiveGroup_primitivegroup =  2,
                    optional_int32_granularity             = 17,
                    optional_int32_date_granularity        = 18,
                    optional_int64_lat_offset              = 19,
                    optional_int64_lon_offset              = 20
                };

                enum class PrimitiveGroup : protozero::pbf_tag_type {
                    unknown                       = 0,
                    repeated_Node_nodes           = 1,
                    optional_DenseNodes_dense     = 2,
                    repeated_Way_ways             = 3,
                    repeated_Relation_relations   = 4,
                    repeated_ChangeSet_changesets = 5,
                    repeated_Area_areas           = 6
                };

                enum class StringTable : protozero::pbf_tag_type {
                    repeated_bytes_s = 1
                };

                enum class Info : protozero::pbf_tag_type {
                    optional_int32_version   = 1,
                    optional_int64_timestamp = 2,
                    optional_int64_changeset = 3,
                    optional_int32_uid       = 4,
                    optional_uint32_user_sid = 5,
                    optional_bool_visible    = 6
                };

                enum class DenseInfo : protozero::pbf_tag_type {
                    packed_int32_version    = 1,
                    packed_sint64_timestamp = 2,
                    packed_sint64_changeset = 3,
                    packed_sint32_uid       = 4,
                    packed_sint32_user_sid  = 5,
                    packed_bool_visible     = 6
                };

                enum class Node : protozero::pbf_tag_type {
                    required_sint64_id  = 1,
                    packed_uint32_keys  = 2,
                    packed_uint32_vals  = 3,
                    optional_Info_info  = 4,
                    required_sint64_lat = 8,
                    required_sint64_lon = 9
                };

                enum class DenseNodes : protozero::pbf_tag_type {
                    packed_sint64_id             =  1,
                    optional_DenseInfo_denseinfo =  5,
                    packed_sint64_lat            =  8,
                    packed_sint64_lon            =  9,
                    packed_int32_keys_vals       = 10
                };

                enum class Way : protozero::pbf_tag_type {
                    required_int64_id  = 1,
                    packed_uint32_keys = 2,
                    packed_uint32_vals = 3,
                    optional_Info_info = 4,
                    packed_sint64_refs = 8,
                    packed_sint64_lat  = 9,
                    packed_sint64_lon  = 10
                };

                enum class Relation : protozero::pbf_tag_type {
                    required_int64_id       =  1,
                    packed_uint32_keys      =  2,
                    packed_uint32_vals      =  3,
                    optional_Info_info      =  4,
                    packed_int32_roles_sid  =  8,
                    packed_sint64_memids    =  9,
                    packed_MemberType_types = 10
                };

                enum class InnerRing : protozero::pbf_tag_type {
                    packed_sint64_refs = 1,
                    packed_sint64_lat  = 2,
                    packed_sint64_lon  = 3
                };

                enum class OuterRing : protozero::pbf_tag_type {
                    packed_sint64_refs = 1,
                    packed_sint64_lat  = 2,
                    packed_sint64_lon  = 3,
                    repeated_InnerRing_innerrings = 4
                };

                enum class Area : protozero::pbf_tag_type {
                    required_int64_id               =  1,
                    packed_uint32_keys              =  2,
                    packed_uint32_vals              =  3,
                    optional_Info_info              =  4,
                    repeated_OuterRing_outerrings   =  5
                };

            } // namespace OSMFormat

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_PROTOBUF_EXTENDED_TAGS_HPP
