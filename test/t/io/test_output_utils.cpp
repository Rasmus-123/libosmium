#include "catch.hpp"

#include <osmium/io/detail/string_util.hpp>

#include <locale>

TEST_CASE("output formatted with small results") {
    std::string out;
    osmium::io::detail::append_printf_formatted_string(out, "%d", 17);
    REQUIRE(out == "17");
}

TEST_CASE("output formatted with several parameters in format string") {
    std::string out;
    osmium::io::detail::append_printf_formatted_string(out, "%d %s", 17, "foo");
    REQUIRE(out == "17 foo");
}

TEST_CASE("output formatted into string already containing something") {
    std::string out{"foo"};
    osmium::io::detail::append_printf_formatted_string(out, " %d", 23);
    REQUIRE(out == "foo 23");
}

TEST_CASE("output formatted with large results") {
    const char* str =
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

    std::string out;
    osmium::io::detail::append_printf_formatted_string(out, "%s", str);
    REQUIRE(out == str);
}

TEST_CASE("UTF8 encoding: append to string") {
    std::string out{"1234"};
    osmium::io::detail::append_utf8_encoded_string(out, "abc");
    REQUIRE(out == "1234abc");
}

TEST_CASE("UTF8 encoding: don't encode alphabetic characters") {
    const char* s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string out;
    osmium::io::detail::append_utf8_encoded_string(out, s);
    REQUIRE(out == s);
}

TEST_CASE("UTF8 encoding: don't encode numeric characters") {
    const char* s = "0123456789";
    std::string out;
    osmium::io::detail::append_utf8_encoded_string(out, s);
    REQUIRE(out == s);
}

TEST_CASE("UTF8 encoding: don't encode lots of often used characters characters") {
    const char* s = ".-;:_#+";
    std::string out;
    osmium::io::detail::append_utf8_encoded_string(out, s);
    REQUIRE(out == s);
}

TEST_CASE("UTF8 encoding: encode characters that are special in OPL") {
    std::string out;
    osmium::io::detail::append_utf8_encoded_string(out, " \n,=@");
    REQUIRE(out == "%20%%0a%%2c%%3d%%40%");
}

// workaround for missing support for u8 string literals on Windows
#if !defined(_MSC_VER)
TEST_CASE("UTF8 encoding: encode multibyte character") {
    std::string out;
    osmium::io::detail::append_utf8_encoded_string(out, u8"\u30dc_\U0001d11e_\U0001f6eb");
    REQUIRE(out == "%30dc%_%1d11e%_%1f6eb%");
}
#endif

TEST_CASE("html encoding does not encode normal characters") {
    const char* s = "abc123,.-";
    std::string out;
    osmium::io::detail::append_xml_encoded_string(out, s);
    REQUIRE(out == s);
}

TEST_CASE("html encoding encodes special XML characters") {
    const char* s = "& \" \' < > \n \r \t";
    std::string out;
    osmium::io::detail::append_xml_encoded_string(out, s);
    REQUIRE(out == "&amp; &quot; &apos; &lt; &gt; &#xA; &#xD; &#x9;");
}

TEST_CASE("debug encoding does not encode normal characters") {
    const char* s = "abc123,.-";
    std::string out;
    osmium::io::detail::append_debug_encoded_string(out, s, "[", "]");
    REQUIRE(out == s);
}

TEST_CASE("debug encoding encodes some unicode characters") {
    const char* s = u8"\n_\u30dc_\U0001d11e_\U0001f6eb";
    std::string out;
    osmium::io::detail::append_debug_encoded_string(out, s, "[", "]");
    REQUIRE(out == "[<U+000A>]_[<U+30DC>]_[<U+1D11E>]_[<U+1F6EB>]");
}

TEST_CASE("utf8 encoding of non-printable characters in the first 127 characters") {
    std::locale cloc{"C"};
    char s[] = "a\0";

    for (char c = 1; c < 0x7f; ++c) {
        std::string out;
        s[0] = c;
        osmium::io::detail::append_utf8_encoded_string(out, s);
        if (!std::isprint(c, cloc)) {
            REQUIRE(out[0] == '%');
        }
    }
}

TEST_CASE("debug encoding of non-printable characters in the first 127 characters") {
    std::locale cloc{"C"};
    char s[] = "a\0";

    for (char c = 1; c < 0x7f; ++c) {
        std::string out;
        s[0] = c;
        osmium::io::detail::append_debug_encoded_string(out, s, "", "");
        if (!std::isprint(c, cloc)) {
            REQUIRE(out[0] == '<');
        }
    }
}

