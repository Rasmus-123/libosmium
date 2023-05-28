#pragma once

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

#include <osmium/handler.hpp>
#include <osmium/io/detail/output_format.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/detail/string_util.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/memory/item_iterator.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/metadata_options.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/thread/pool.hpp>
#include <osmium/visitor.hpp>

#include <iterator>
#include <memory>
#include <string>
#include <utility>

#ifndef OSMIOEXTENSIONS
static_assert(false, "libosmium requires changes for xmle to be supported!");
#endif

namespace osmium::extended::io
{
    namespace detail
	{

        struct XMLEWriteError {};

        struct xmle_output_options {

            /// Which metadata of objects should be added?
            osmium::metadata_options add_metadata;

            /// Should the visible flag be added to all OSM objects?
            bool add_visible_flag = false;

            /**
             * Should <create>, <modify>, <delete> "operations" be added?
             * (This is used for .osc files.)
             */
            bool use_change_ops = false;

            /// Should node locations be added to ways?
            bool locations_on_ways = false;

        }; // struct xmle_output_options

        namespace detail {

            inline void append_lat_lon_attributes(std::string& out, const char* lat, const char* lon, const osmium::Location& location) {
                out += ' ';
                out += lat;
                out += "=\"";
                osmium::detail::append_location_coordinate_to_string(std::back_inserter(out), location.y());
                out += "\" ";
                out += lon;
                out += "=\"";
                osmium::detail::append_location_coordinate_to_string(std::back_inserter(out), location.x());
                out += "\"";
            }

        } // namespace detail

        class XMLEOutputBlock : public osmium::io::detail::OutputBlock {

            // operation (create, modify, delete) for osc files
            enum class operation {
                op_none = 0,
                op_create = 1,
                op_modify = 2,
                op_delete = 3
            }; // enum class operation

            operation m_last_op{ operation::op_none };

            xmle_output_options m_options;

            void write_spaces(int num) {
                for (; num != 0; --num) {
                    *m_out += ' ';
                }
            }

            int prefix_spaces() const noexcept {
                return m_options.use_change_ops ? 4 : 2;
            }

            void write_prefix() {
                write_spaces(prefix_spaces());
            }

            template <typename T>
            void write_attribute(const char* name, T value) {
                *m_out += ' ';
                *m_out += name;
                *m_out += "=\"";
                output_int(value);
                *m_out += '"';
            }

            void write_meta(const osmium::OSMObject& object) {
                write_attribute("id", object.id());

                if (m_options.add_metadata.version() && object.version()) {
                    write_attribute("version", object.version());
                }

                if (m_options.add_metadata.timestamp() && object.timestamp()) {
                    *m_out += " timestamp=\"";
                    *m_out += object.timestamp().to_iso_all();
                    *m_out += "\"";
                }

                if (m_options.add_metadata.uid() && object.uid()) {
                    write_attribute("uid", object.uid());
                }

                if (m_options.add_metadata.user() && object.user()[0] != '\0') {
                    *m_out += " user=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, object.user());
                    *m_out += "\"";
                }

                if (m_options.add_metadata.changeset() && object.changeset()) {
                    write_attribute("changeset", object.changeset());
                }

                if (m_options.add_visible_flag) {
                    if (object.visible()) {
                        *m_out += " visible=\"true\"";
                    }
                    else {
                        *m_out += " visible=\"false\"";
                    }
                }
            }

            void write_tags(const osmium::TagList& tags, int spaces) {
                for (const auto& tag : tags) {
                    write_spaces(spaces);
                    *m_out += "  <tag k=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, tag.key());
                    *m_out += "\" v=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, tag.value());
                    *m_out += "\"/>\n";
                }
            }

            void write_discussion(const osmium::ChangesetDiscussion& comments) {
                *m_out += "  <discussion>\n";
                for (const auto& comment : comments) {
                    *m_out += "   <comment";
                    write_attribute("uid", comment.uid());
                    *m_out += " user=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, comment.user());
                    *m_out += "\" date=\"";
                    *m_out += comment.date().to_iso_all();
                    *m_out += "\">\n";
                    *m_out += "    <text>";
                    osmium::io::detail::append_xml_encoded_string(*m_out, comment.text());
                    *m_out += "</text>\n   </comment>\n";
                }
                *m_out += "  </discussion>\n";
            }

            void open_close_op_tag(const operation op = operation::op_none) {
                if (op == m_last_op) {
                    return;
                }

                switch (m_last_op) {
                case operation::op_none:
                    break;
                case operation::op_create:
                    *m_out += "  </create>\n";
                    break;
                case operation::op_modify:
                    *m_out += "  </modify>\n";
                    break;
                case operation::op_delete:
                    *m_out += "  </delete>\n";
                    break;
                }

                switch (op) {
                case operation::op_none:
                    break;
                case operation::op_create:
                    *m_out += "  <create>\n";
                    break;
                case operation::op_modify:
                    *m_out += "  <modify>\n";
                    break;
                case operation::op_delete:
                    *m_out += "  <delete>\n";
                    break;
                }

                m_last_op = op;
            }

        public:

            XMLEOutputBlock(osmium::memory::Buffer&& buffer, const xmle_output_options& options) :
                OutputBlock(std::move(buffer)),
                m_options(options) {
            }

            std::string operator()() {
                osmium::apply(m_input_buffer->cbegin(), m_input_buffer->cend(), *this);

                if (m_options.use_change_ops) {
                    open_close_op_tag();
                }

                std::string out;
                using std::swap;
                swap(out, *m_out);

                return out;
            }

            void node(const osmium::Node& node) {
                if (m_options.use_change_ops) {
                    open_close_op_tag(node.visible() ? (node.version() == 1 ? operation::op_create : operation::op_modify) : operation::op_delete);
                }

                write_prefix();
                *m_out += "<node";

                write_meta(node);

                if (node.location()) {
                    detail::append_lat_lon_attributes(*m_out, "lat", "lon", node.location());
                }

                if (node.tags().empty()) {
                    *m_out += "/>\n";
                    return;
                }

                *m_out += ">\n";

                write_tags(node.tags(), prefix_spaces());

                write_prefix();
                *m_out += "</node>\n";
            }

            void way(const osmium::Way& way) {
                if (m_options.use_change_ops) {
                    open_close_op_tag(way.visible() ? (way.version() == 1 ? operation::op_create : operation::op_modify) : operation::op_delete);
                }

                write_prefix();
                *m_out += "<way";
                write_meta(way);

                if (way.tags().empty() && way.nodes().empty()) {
                    *m_out += "/>\n";
                    return;
                }

                *m_out += ">\n";

                if (m_options.locations_on_ways) {
                    for (const auto& node_ref : way.nodes()) {
                        write_prefix();
                        *m_out += "  <nd";
                        write_attribute("ref", node_ref.ref());
                        if (node_ref.location()) {
                            detail::append_lat_lon_attributes(*m_out, "lat", "lon", node_ref.location());
                        }
                        *m_out += "/>\n";
                    }
                }
                else {
                    for (const auto& node_ref : way.nodes()) {
                        write_prefix();
                        *m_out += "  <nd";
                        write_attribute("ref", node_ref.ref());
                        *m_out += "/>\n";
                    }
                }

                write_tags(way.tags(), prefix_spaces());

                write_prefix();
                *m_out += "</way>\n";
            }

            void relation(const osmium::Relation& relation) {
                if (m_options.use_change_ops) {
                    open_close_op_tag(relation.visible() ? (relation.version() == 1 ? operation::op_create : operation::op_modify) : operation::op_delete);
                }

                write_prefix();
                *m_out += "<relation";
                write_meta(relation);

                if (relation.tags().empty() && relation.members().empty()) {
                    *m_out += "/>\n";
                    return;
                }

                *m_out += ">\n";

                for (const auto& member : relation.members()) {
                    write_prefix();
                    *m_out += "  <member type=\"";
                    *m_out += item_type_to_name(member.type());
                    *m_out += '"';
                    write_attribute("ref", member.ref());
                    *m_out += " role=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, member.role());
                    *m_out += "\"/>\n";
                }

                write_tags(relation.tags(), prefix_spaces());

                write_prefix();
                *m_out += "</relation>\n";
            }

            void area(const osmium::Area& area) {

                /*
                 * Area:
                 *      ~from_way() - bool | True := From Way, False := From Relation | Based on id
                 *      ~orig_id() - i64 | Id of original Way or Relation | Based on id
                 *      ~num_rings() - pair<size_t x2> | outer, inner~
                 *      ~is_multipolygon - bool | := num_outer_rings > 1~
                 *      outer_rings() - iterator
                 *      inner_rings(oring) - iterator
                 *      ~envelope - Box~
                 * Inherited from OSMObject:
                 *      id() - i64
                 *      ~is_compatible_to() - bool~
                 *      ~deleted() - bool~
                 *      ~visible() - bool | !deleted()~
                 *      version() - i32
                 *      changeset() - i32
                 *      uid() - i32
                 *      ~user_is_anonymous() - bool~
                 *      timestamp() - Timestamp
                 *      user() - char*
                 *      tags() - TagList
                 *      get_value_by_key(key) - char*
                 */

                if (m_options.use_change_ops) {
                    open_close_op_tag(area.visible() ? (area.version() == 1 ? operation::op_create : operation::op_modify) : operation::op_delete);
                }

                write_spaces(2);
                *m_out += "<area";
                write_meta(area);

                if (area.tags().empty() && area.num_rings().first == 0) {
                    *m_out += "/>\n";
                    return;
                }

                *m_out += ">\n";
                
                for (const auto& oring : area.outer_rings()) 
                {
                    // OuterRing <=> NodeRefList:

                    write_spaces(4); // Why are there 6?
                    *m_out += "  <outer_ring>\n";
                    for (const auto& node_ref : oring)
                    {
                        write_spaces(6);
                        *m_out += "  <nd";
                        write_attribute("ref", node_ref.ref());
                        if (node_ref.location()) {
                            detail::append_lat_lon_attributes(*m_out, "lat", "lon", node_ref.location());
                        }
                        *m_out += "/>\n";
                    }
                    
                    for (const auto& iring : area.inner_rings(oring))
                    {
                        // InnerRing <=> NodeRefList

						write_spaces(6); // Why are there 8?
						*m_out += "  <inner_ring>\n";
                        for (const auto& node_ref : iring)
                        {
                            write_spaces(8);
                            *m_out += "  <nd";
                            write_attribute("ref", node_ref.ref());
                            if (node_ref.location()) {
                                detail::append_lat_lon_attributes(*m_out, "lat", "lon", node_ref.location());
                            }
                            *m_out += "/>\n";
                        }
                        write_spaces(6);
                        *m_out += "  </inner_ring>\n";
                    }

                    write_spaces(4); // Those are again 4?
                    *m_out += "  </outer_ring>\n";
                }

                write_tags(area.tags(), prefix_spaces());

                write_prefix();
                *m_out += "</area>\n";
            }

            void changeset(const osmium::Changeset& changeset) {
                *m_out += " <changeset";

                write_attribute("id", changeset.id());

                if (changeset.created_at()) {
                    *m_out += " created_at=\"";
                    *m_out += changeset.created_at().to_iso();
                    *m_out += "\"";
                }

                if (changeset.closed_at()) {
                    *m_out += " closed_at=\"";
                    *m_out += changeset.closed_at().to_iso();
                    *m_out += "\" open=\"false\"";
                }
                else {
                    *m_out += " open=\"true\"";
                }

                if (!changeset.user_is_anonymous()) {
                    *m_out += " user=\"";
                    osmium::io::detail::append_xml_encoded_string(*m_out, changeset.user());
                    *m_out += '"';
                    write_attribute("uid", changeset.uid());
                }

                if (!changeset.bounds().bottom_left().is_undefined() ||
                    !changeset.bounds().top_right().is_undefined()) {
                    detail::append_lat_lon_attributes(*m_out, "min_lat", "min_lon", changeset.bounds().bottom_left());
                    detail::append_lat_lon_attributes(*m_out, "max_lat", "max_lon", changeset.bounds().top_right());
                }

                write_attribute("num_changes", changeset.num_changes());
                write_attribute("comments_count", changeset.num_comments());

                // If there are no tags and no comments, we can close the
                // tag right here and are done.
                if (changeset.tags().empty() && changeset.discussion().empty()) {
                    *m_out += "/>\n";
                    return;
                }

                *m_out += ">\n";

                write_tags(changeset.tags(), 0);

                if (!changeset.discussion().empty()) {
                    write_discussion(changeset.discussion());
                }

                *m_out += " </changeset>\n";
            }

        }; // class XMLEOutputBlock

        class XMLEOutputFormat : public osmium::io::detail::OutputFormat, public osmium::handler::Handler {

            xmle_output_options m_options;

        public:

            XMLEOutputFormat(osmium::thread::Pool& pool, const osmium::io::File& file, osmium::io::detail::future_string_queue_type& output_queue) :
                OutputFormat(pool, output_queue) {
                m_options.add_metadata = osmium::metadata_options{ file.get("add_metadata") };
                m_options.use_change_ops = file.is_true("xml_change_format");
                m_options.add_visible_flag = (file.has_multiple_object_versions() || file.is_true("force_visible_flag")) && !m_options.use_change_ops;
                m_options.locations_on_ways = file.is_true("locations_on_ways");
            }

            void write_header(const osmium::io::Header& header) final {
                std::string out{ "<?xmle version='1.0' encoding='UTF-8'?>\n" };

                if (m_options.use_change_ops) {
                    out += "<osmChange version=\"0.6\" generator=\"";
                }
                else {
                    out += "<osm version=\"0.6\"";

                    const std::string xml_josm_upload{ header.get("xml_josm_upload") };
                    if (xml_josm_upload == "true" || xml_josm_upload == "false") {
                        out += " upload=\"";
                        out += xml_josm_upload;
                        out += "\"";
                    }
                    out += " generator=\"";
                }
                osmium::io::detail::append_xml_encoded_string(out, header.get("generator").c_str());
                out += "\">\n";

                for (const auto& box : header.boxes()) {
                    out += "  <bounds";
                    detail::append_lat_lon_attributes(out, "minlat", "minlon", box.bottom_left());
                    detail::append_lat_lon_attributes(out, "maxlat", "maxlon", box.top_right());
                    out += "/>\n";
                }

                send_to_output_queue(std::move(out));
            }

            void write_buffer(osmium::memory::Buffer&& buffer) final {
                m_output_queue.push(m_pool.submit(XMLEOutputBlock{ std::move(buffer), m_options }));
            }

            void write_end() final {
                std::string out;

                if (m_options.use_change_ops) {
                    out += "</osmChange>\n";
                }
                else {
                    out += "</osm>\n";
                }

                send_to_output_queue(std::move(out));
            }

        }; // class XMLEOutputFormat

        // we want the register_output_format() function to run, setting
        // the variable is only a side-effect, it will never be used
        const bool registered_xmle_output = osmium::io::detail::OutputFormatFactory::instance().register_output_format(osmium::io::file_format::xmle,
            [](osmium::thread::Pool& pool, const osmium::io::File& file, osmium::io::detail::future_string_queue_type& output_queue) {
                return new osmium::extended::io::detail::XMLEOutputFormat(pool, file, output_queue);
            });

        // dummy function to silence the unused variable warning from above
        inline bool get_registered_xmle_output() noexcept {
            return registered_xmle_output;
        }

    } // namespace detail

} // namespace osmium::extended
