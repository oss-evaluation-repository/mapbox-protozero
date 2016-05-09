
#define PBF_TYPE_NAME PROTOZERO_TEST_STRING(PBF_TYPE)
#define GET_TYPE PROTOZERO_TEST_CONCAT(get_packed_, PBF_TYPE)
#define ADD_TYPE PROTOZERO_TEST_CONCAT(add_packed_, PBF_TYPE)

using packed_field_type = PROTOZERO_TEST_CONCAT(protozero::packed_field_, PBF_TYPE);

TEST_CASE("read repeated packed field: " PBF_TYPE_NAME) {

    // Run these tests twice, the second time we basically move the data
    // one byte down in the buffer. It doesn't matter how the data or buffer
    // is aligned before that, in at least one of these cases the ints will
    // not be aligned properly. So we test that even in that case the ints
    // will be extracted properly.

    for (std::string::size_type n = 0; n < 2; ++n) {

        std::string abuffer;
        abuffer.reserve(1000);
        abuffer.append(n, '\0');

        SECTION("empty") {
            abuffer.append(load_data("repeated_packed_" PBF_TYPE_NAME "/data-empty"));

            protozero::pbf_reader item(abuffer.data() + n, abuffer.size() - n);

            REQUIRE(!item.next());
        }

        SECTION("one") {
            abuffer.append(load_data("repeated_packed_" PBF_TYPE_NAME "/data-one"));

            protozero::pbf_reader item(abuffer.data() + n, abuffer.size() - n);

            REQUIRE(item.next());
            const auto it_range = item.GET_TYPE();
            REQUIRE(!item.next());

            REQUIRE(it_range.begin() != it_range.end());
            REQUIRE(*it_range.begin() == 17L);
            REQUIRE(std::next(it_range.begin()) == it_range.end());
        }

        SECTION("many") {
            abuffer.append(load_data("repeated_packed_" PBF_TYPE_NAME "/data-many"));

            protozero::pbf_reader item(abuffer.data() + n, abuffer.size() - n);

            REQUIRE(item.next());
            const auto it_range = item.GET_TYPE();
            REQUIRE(!item.next());

            auto it = it_range.begin();
            REQUIRE(it != it_range.end());
            REQUIRE(*it++ ==   17L);
            REQUIRE(*it++ ==  200L);
            REQUIRE(*it++ ==    0L);
            REQUIRE(*it++ ==    1L);
            REQUIRE(*it++ == std::numeric_limits<cpp_type>::max());
#if PBF_TYPE_IS_SIGNED
            REQUIRE(*it++ == -200L);
            REQUIRE(*it++ ==   -1L);
            REQUIRE(*it++ == std::numeric_limits<cpp_type>::min());
#endif
            REQUIRE(it == it_range.end());
        }

        SECTION("end_of_buffer") {
            abuffer.append(load_data("repeated_packed_" PBF_TYPE_NAME "/data-many"));

            for (std::string::size_type i = 1; i < abuffer.size() - n; ++i) {
                protozero::pbf_reader item(abuffer.data() + n, i);
                REQUIRE(item.next());
                REQUIRE_THROWS_AS(item.GET_TYPE(), protozero::end_of_buffer_exception);
            }
        }

    }

}

TEST_CASE("write repeated packed field: " PBF_TYPE_NAME) {

    std::string buffer;
    protozero::pbf_writer pw(buffer);

    SECTION("empty") {
        cpp_type data[] = { 17L };
        pw.ADD_TYPE(1, std::begin(data), std::begin(data) /* !!!! */);

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-empty"));
    }

    SECTION("one") {
        cpp_type data[] = { 17L };
        pw.ADD_TYPE(1, std::begin(data), std::end(data));

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-one"));
    }

    SECTION("many") {
        cpp_type data[] = {
               17L
            , 200L
            ,   0L
            ,   1L
            ,std::numeric_limits<cpp_type>::max()
#if PBF_TYPE_IS_SIGNED
            ,-200L
            ,  -1L
            ,std::numeric_limits<cpp_type>::min()
#endif
        };
        pw.ADD_TYPE(1, std::begin(data), std::end(data));

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-many"));
    }

}

TEST_CASE("write repeated packed field using packed field: " PBF_TYPE_NAME) {

    std::string buffer;
    protozero::pbf_writer pw(buffer);

    SECTION("empty - should do rollback") {
        {
            packed_field_type field{pw, 1};
        }

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-empty"));
    }

    SECTION("one") {
        {
            packed_field_type field{pw, 1};
            field.add_element(17L);
        }

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-one"));
    }

    SECTION("many") {
        {
            packed_field_type field{pw, 1};
            field.add_element(  17L);
            field.add_element( 200L);
            field.add_element(   0L);
            field.add_element(   1L);
            field.add_element(std::numeric_limits<cpp_type>::max());
#if PBF_TYPE_IS_SIGNED
            field.add_element(-200L);
            field.add_element(  -1L);
            field.add_element(std::numeric_limits<cpp_type>::min());
#endif
        }

        REQUIRE(buffer == load_data("repeated_packed_" PBF_TYPE_NAME "/data-many"));
    }

}

TEST_CASE("write from different types of iterators: " PBF_TYPE_NAME) {

    std::string buffer;
    protozero::pbf_writer pw(buffer);

    SECTION("from uint16_t") {
#if PBF_TYPE_IS_SIGNED
        const  int16_t data[] = { 1, 4, 9, 16, 25 };
#else
        const uint16_t data[] = { 1, 4, 9, 16, 25 };
#endif

        pw.ADD_TYPE(1, std::begin(data), std::end(data));
    }

    SECTION("from string") {
        std::string data = "1 4 9 16 25";
        std::stringstream sdata(data);

#if PBF_TYPE_IS_SIGNED
        using test_type =  int32_t;
#else
        using test_type = uint32_t;
#endif

        std::istream_iterator<test_type> eod;
        std::istream_iterator<test_type> it(sdata);

        pw.ADD_TYPE(1, it, eod);
    }

    protozero::pbf_reader item(buffer);

    REQUIRE(item.next());
    const auto it_range = item.GET_TYPE();
    REQUIRE(!item.next());
    REQUIRE(std::distance(it_range.begin(), it_range.end()) == 5);

    auto it = it_range.begin();
    REQUIRE(*it++ ==  1);
    REQUIRE(*it++ ==  4);
    REQUIRE(*it++ ==  9);
    REQUIRE(*it++ == 16);
    REQUIRE(*it++ == 25);
    REQUIRE(it == it_range.end());
}
