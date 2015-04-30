
#include <test.hpp>

TEST_CASE("int32") {

    SECTION("zero") {
        std::string buffer = get_file_data("test/t/int32/data-zero.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(item.varint<int32_t>() == 0l);
        REQUIRE(!item.next());
    }

    SECTION("positive") {
        std::string buffer = get_file_data("test/t/int32/data-pos.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(item.varint<int32_t>() == 1l);
        REQUIRE(!item.next());
    }

    SECTION("negative") {
        std::string buffer = get_file_data("test/t/int32/data-neg.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(item.varint<int32_t>() == -1l);
        REQUIRE(!item.next());
    }

    SECTION("max") {
        std::string buffer = get_file_data("test/t/int32/data-max.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(item.varint<int32_t>() == std::numeric_limits<int32_t>::max());
        REQUIRE(!item.next());
    }

    SECTION("min64") {
        std::string buffer = get_file_data("test/t/int32/data-min.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(static_cast<int32_t>(item.varint<int64_t>()) == std::numeric_limits<int32_t>::min());
        REQUIRE(!item.next());
    }

    SECTION("min") {
        std::string buffer = get_file_data("test/t/int32/data-min.bin");

        mapbox::util::pbf item(buffer.data(), buffer.size());

        REQUIRE(item.next());
        REQUIRE(item.varint<int32_t>() == std::numeric_limits<int32_t>::min());
        REQUIRE(!item.next());
    }

}
