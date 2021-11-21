#define MY_TESTS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "iostream"
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
#include "huffman.h"

std::string resource_path(const std::string& filename) {
    static std::string resources_folder = "./test/resources/";
    return resources_folder + filename;
}

bool files_are_same(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false;
    }

    if (f1.tellg() != f2.tellg()) {
        return false;
    }

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

void encode_decode_compare(std::string input) {
    std::string encoded = resource_path("encoded.bin");
    std::string actual = resource_path("actual.txt");
    Huffman::Tree t;
    t.encodeFile(input, encoded);
    t.decodeFile(encoded, actual);
    CHECK(files_are_same(input, actual));
    remove(encoded.c_str());
    remove(actual.c_str());
}

TEST_CASE("BitWriter::operator<<") {
    SUBCASE("bool") {
        std::string output = resource_path("out.bin");
        std::ofstream out(output);
        Huffman::BitWriter w(out);
        bool t = 1, f = 0;
        w << f << t << t << f << f << f << f << t; //01100001 = 'a'
        w.flush();
        out.close();
        std::ifstream in(output);
        char actual;
        in >> actual;
        remove(output.c_str());
        CHECK_EQ(actual, 'a');
    }

    SUBCASE("int") {
        std::string output = resource_path("out.bin");
        std::ofstream out(output);
        Huffman::BitWriter w(out);
        int expected = 42;
        w << expected;
        w.flush();
        out.close();
        std::ifstream in(output);
        char actual;
        in >> actual;
        remove(output.c_str());
        CHECK_EQ(actual, expected);
    }
}

TEST_CASE("BitReader::operator>>") {
    SUBCASE("bool") {
        std::string input = resource_path("a.txt");
        std::ifstream in(input);
        Huffman::BitReader r(in);
        bool b;
        r >> b;
        CHECK_EQ(b, 0);
        r >> b;
        CHECK_EQ(b, 1);
    }

    SUBCASE("char") {
        std::string input = resource_path("a.txt");
        std::ifstream in(input);
        Huffman::BitReader r(in);
        char c;
        r >> c;
        CHECK_EQ(c, 'a');
    }
}

TEST_CASE("BitReader + BitWriter") {
    SUBCASE("Writing and reading a full byte and 4 individual bits") {
        std::string output = resource_path("out.bin");
        std::ofstream out(output);
        Huffman::BitWriter w(out);
        bool t = 1, f = 0;
        for (int i = 0; i < 4; i++) {
            w << t;
            w << f;
        }
        for (int i = 0; i < 4; i++) {
            w << t;
        }
        w.flush();
        out.close();

        std::ifstream in(output);
        Huffman::BitReader r(in);
        unsigned char actual;
        r >> actual;
        unsigned char expected = 170;
        CHECK_EQ(expected, actual);
        bool b;
        for (int i = 0; i < 4; i++) {
            r >> b;
            CHECK_EQ(b, 1);
        }
        for (int i = 0; i < 4; i++) {
            r >> b;
            CHECK_EQ(b, 0);
        }
        in.close();
        remove(output.c_str());
    }
}

TEST_CASE("Tree::loadRawEntries") {
    Huffman::Tree t;
    SUBCASE("english letters and numbers") {
        std::string input = resource_path("a-z0-9.txt");
        std::ifstream in(input);
        t.loadRawEntries(in);
        int expected[Huffman::Tree::max_chars];
        std::fill(expected, expected + Huffman::Tree::max_chars, 0);
        for (auto c = (unsigned char)'0'; c <= (unsigned char)'9'; c++) expected[c]++;
        for (auto c = (unsigned char) 'a'; c <= (unsigned char) 'z'; c++) expected[c]++;
        CHECK(std::equal(t.entries, t.entries + Huffman::Tree::max_chars, expected));
        in.close();
    }
    SUBCASE("many of a single letter") {
        std::string input = resource_path("many-a.txt");
        std::ifstream in(input);
        t.loadRawEntries(in);
        int expected[Huffman::Tree::max_chars];
        std::fill(expected, expected + Huffman::Tree::max_chars, 0);
        unsigned char c = 'a';
        expected[c] = 10000;
        CHECK(std::equal(t.entries, t.entries + Huffman::Tree::max_chars, expected));
        in.close();
    }
}

TEST_CASE("Tree::mergeTree") {
    Huffman::Tree t;

    SUBCASE("expected empty tree on empty entries array") {
        t.mergeTree();
        CHECK_EQ(t.root, nullptr);
        CHECK(std::all_of(t.entries, t.entries + t.max_chars, [](int i){ return i == 0; }));
        t.clear();
    }

    SUBCASE("merge tree when all bytes are present once") {
        for (int i = 0; i < 256; i++) {
            t.nodes.push(new Huffman::Node({(unsigned char) i}, 1));
        }
        t.mergeTree();

    }
}


TEST_CASE("Tree::buildTree") {
    Huffman::Tree t;

    SUBCASE("Expected empty tree from an empty plain text file") {
        std::string input = resource_path("empty.txt");
        std::ifstream in(input);
        t.loadRawEntries(in);
        t.buildTree();
        CHECK_EQ(t.root, nullptr);
        CHECK(std::all_of(t.entries, t.entries + t.max_chars, [](int i){ return i == 0; }));
        in.close();
        t.clear();
    }
    SUBCASE("Expected empty tree from an empty encoded file") {
        std::string input = resource_path("encoded-empty.bin");
        std::ifstream in(input);
        t.loadEncodedTree(in);
        t.buildTree();
        CHECK_EQ(t.root, nullptr);
        CHECK(std::all_of(t.entries, t.entries + t.max_chars, [](int i){ return i == 0; }));
        in.close();
        t.clear();
    }

    SUBCASE("Check tree loaded correctly from a simple plain text") {
        std::string input = resource_path("wiki-frequency-test.txt");
        std::ifstream in(input);
        t.loadRawEntries(in);
        t.buildTree();
        std::vector<bool> expected_codes[Huffman::Tree::max_chars];
        std::fill(expected_codes, expected_codes + Huffman::Tree::max_chars, std::vector<bool>());
        expected_codes[(unsigned char) 'a'] = {0};
        expected_codes[(unsigned char) 'b'] = {1, 1, 1};
        expected_codes[(unsigned char) 'c'] = {1, 0, 1};
        expected_codes[(unsigned char) 'd'] = {1, 1, 0};
        expected_codes[(unsigned char) 'e'] = {1, 0, 0};
        CHECK(std::equal(t.codes, t.codes + Huffman::Tree::max_chars, expected_codes));
        in.close();
        t.clear();
    }

    SUBCASE("Check that the tree from plain text file and encoded file is the same") {
        std::string input = resource_path("lorem-ipsum.txt"),
            binOut = resource_path("ipsum.bin"),
            decoded = resource_path("decoded.txt");
        t.encodeFile(input, binOut);
        t.decodeFile(binOut, decoded);
        std::ifstream in1(input);
        std::ifstream in2(decoded);
        t.clear();
        Huffman::Tree t2;
        t.loadRawEntries(in1);
        t.buildTree();
        t2.loadRawEntries(in2);
        t2.buildTree();
        CHECK(std::equal(t.codes, t.codes + Huffman::Tree::max_chars, t2.codes));
        in1.close();
        in2.close();
        t.clear();
        remove(binOut.c_str());
        remove(decoded.c_str());
    }
}

TEST_CASE("Tree::input_size, Tree::output_size") {
    SUBCASE("Size 0 expected on empty files") {
        std::string input = resource_path("empty.txt");
        std::string output = resource_path("output.txt");
        Huffman::Tree t;
        t.encodeFile(input, output, false, false);
        CHECK_EQ(t.input_size, 0);
        CHECK_EQ(t.output_size - t.extra_bytes, 0);
        t.clear();
        std::string decoded = resource_path("decoded.txt");
        t.decodeFile(output, decoded, false, false);
        CHECK_EQ(t.input_size - t.extra_bytes, 0);
        CHECK_EQ(t.output_size, 0);
        remove(output.c_str());
        remove(decoded.c_str());
    }
    SUBCASE("Check that size before encoding equals the size after decoding") {
        std::string input = resource_path("lorem-ipsum.txt");
        std::string output = resource_path("output.txt");
        Huffman::Tree t;
        t.encodeFile(input, output, false, false);
        long long init_input_size = t.input_size;
        long long init_output_size = t.output_size;
        t.clear();
        std::string decoded = resource_path("decoded.txt");
        t.decodeFile(output, decoded, false, false);
        CHECK_EQ(t.input_size, init_output_size);
        CHECK_EQ(t.output_size, init_input_size);
        remove(output.c_str());
        remove(decoded.c_str());
    }
}

TEST_CASE("Tree::encodeFile + Tree::decodeFile. Check contents are the same before encoding and after decoding") {
    SUBCASE("small english text") {
        std::string input = resource_path("small.txt");
        encode_decode_compare(input);
    }
    SUBCASE("empty file") {
        std::string input = resource_path("empty.txt");
        encode_decode_compare(input);
    }
    SUBCASE("russian language") {
        std::string input = resource_path("russian.txt");
        encode_decode_compare(input);
    }
    SUBCASE("Bigger english text") {
        std::string input = resource_path("lorem-ipsum.txt");
        encode_decode_compare(input);
    }
    SUBCASE("Many of a single letter") {
        std::string input = resource_path("many-a.txt");
        encode_decode_compare(input);
    }
}

TEST_CASE("Exceptions") {
    SUBCASE("Invalid encoded header") {
        std::string input = resource_path("invalid-header.bin");
        std::string output = resource_path("invalid.out");
        Huffman::Tree t;
        CHECK_THROWS_WITH_AS(t.decodeFile(input, output), "Header data not found", std::invalid_argument);
        remove(output.c_str());
    }

    SUBCASE("Invalid encoded bits") {
        std::string input = resource_path("invalid-bits.bin");
        std::string output = resource_path("invalid-bits.out");
        Huffman::Tree t;
        CHECK_THROWS_WITH_AS(t.decodeFile(input, output), "Invalid bit sequence", std::invalid_argument);
        remove(output.c_str());
    }

    SUBCASE("More bits expected") {
        std::string input = resource_path("not-enough-bits.bin");
        std::string output = resource_path("not-enough-bits.out");
        Huffman::Tree t;
        CHECK_THROWS_WITH_AS(t.decodeFile(input, output), "Unable to read expected bits", std::invalid_argument);
        remove(output.c_str());
    }

    SUBCASE("file does not exist") {
        std::string input = resource_path("does-not-exist");
        std::string output = resource_path("does-not-exist.out");
        Huffman::Tree t;
        CHECK_THROWS_WITH_AS(t.encodeFile(input, output), "Unable to open input file", std::invalid_argument);
        remove(output.c_str());
    }
}