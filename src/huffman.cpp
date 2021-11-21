#include "huffman.h"
#include <utility>
#include <algorithm>
#include "iostream"

namespace Huffman {

    void Tree::buildTree() {
        for (int i = 0; i < max_chars; i++) {
            if (entries[i] != 0) {
                nodes.push(new Node({(unsigned char) i}, entries[i]));
            }
        }
        mergeTree();

    }

    void Tree::mergeTree() {
        if (nodes.empty()) return;

        if (nodes.size() == 1) {
            Node *v = nodes.top();
            nodes.pop();
            root = new Node(v->chars, v->frequency, v, nullptr);
            codes[v->chars.front()].push_back(0);
            return;
        }

        while (nodes.size() > 1) {
            Node *v = nodes.top();
            nodes.pop();
            Node *u = nodes.top();
            nodes.pop();
            auto united_chars = v->chars;
            united_chars.insert(united_chars.end(), u->chars.begin(), u->chars.end());
            nodes.push(new Node(united_chars, v->frequency + u->frequency, v, u));
            for (auto c: v->chars) {
                codes[c].push_back(0);
            }
            for (auto c: u->chars) {
                codes[c].push_back(1);
            }
        }
        for (auto &code : codes) {
            std::reverse(code.begin(), code.end());
        }
        root = nodes.top();
        nodes.pop();
    }

    Tree::~Tree() {
        delete root;
    }

    void Tree::encodeAndWriteCompressed(std::ifstream &in, std::ofstream &out) {
        long long total_bits = 0;
        auto writer = BitWriter(out);
        auto reader = BitReader(in);
        writer << count;
        for (auto cnt: entries)
            writer << cnt;
        output_size += extra_bytes;
        unsigned char c;
        while (reader >> c) {
            for (bool bit: codes[c]) {
                writer << bit;
                total_bits++;
            }
        }
        writer.flush();
        if (total_bits != 0)
            output_size += (total_bits - 1) / byte_size + 1;
    }

    void Tree::loadEncodedTree(std::ifstream &in) {
        auto reader = BitReader(in);
        if (!(reader >> count) || count < 0) throw std::invalid_argument("Header data not found");
        for (long long &entry : entries) {
            if (!(reader >> entry) || count < 0) throw std::invalid_argument("Header data not found");
        }
        input_size += extra_bytes;
    }

    void Tree::loadRawEntries(std::ifstream &in) {
        auto reader = BitReader(in);
        std::fill(entries, entries + max_chars, 0);
        unsigned char c;
        while (reader >> c) {
            entries[c]++;
            count++;
            input_size++;
        }
    }

    void Tree::decodeAndWriteText(std::ifstream &in, std::ofstream &out) {
        long long total_bits = 0;
        auto reader = BitReader(in);
        bool bit;
        Node *v = root;
        unsigned char c;
        for (long long i = 0; i < count; i++) {
            while (v != nullptr && v->left_child != nullptr) {
                if (!(reader >> bit)) throw std::invalid_argument("Unable to read expected bits");
                total_bits++;
                if (!bit)
                    v = v->left_child;
                else
                    v = v->right_child;
            }
            if (v == nullptr)
                throw std::invalid_argument("Invalid bit sequence");
            if (v->chars.size() != 1) throw std::logic_error("Something went wrong");
            c = v->chars.front();
            out << c;
            v = root;
            output_size++;
        }
        if (total_bits != 0)
            input_size += (total_bits - 1) / byte_size + 1;
    }

    void
    Tree::encodeFile(std::string &input_file_name, std::string &output_file_name, bool print_stat, bool clear_on_exit) {
        std::ifstream in = std::ifstream(input_file_name);
        std::ofstream out = std::ofstream(output_file_name);
        try {
            if (!in) throw std::invalid_argument("Unable to open input file");
            if (!out) throw std::invalid_argument("Unable to open output file");
            loadRawEntries(in);
            buildTree();
            in.clear();
            in.seekg(0);
            encodeAndWriteCompressed(in, out);
            in.close();
            out.close();
            if (print_stat)
                std::cout << input_size << std::endl << output_size - extra_bytes << std::endl << extra_bytes
                          << std::endl;
            if (clear_on_exit)
                clear();
        }
        catch (std::invalid_argument &e) {
            in.close();
            out.close();
            throw e;
        }
        catch (std::exception &e) {
            in.close();
            out.close();
            throw e;
        }
    }

    void
    Tree::decodeFile(std::string &input_file_name, std::string &output_file_name, bool print_stat, bool clear_on_exit) {
        std::ifstream in = std::ifstream(input_file_name);
        std::ofstream out = std::ofstream(output_file_name);
        try {
            if (!in) throw std::invalid_argument("Unable to open input file");
            if (!out) throw std::invalid_argument("Unable to open output file");
            loadEncodedTree(in);
            buildTree();
            decodeAndWriteText(in, out);
            in.close();
            out.close();
            if (print_stat)
                std::cout << input_size - extra_bytes << std::endl << output_size << std::endl << extra_bytes << std::endl;
            if (clear_on_exit)
                clear();
        }
        catch (std::invalid_argument &e) {
            in.close();
            out.close();
            throw e;
        }
        catch (std::exception &e) {
            in.close();
            out.close();
            throw e;
        }
    }

    void Tree::clear() {
        for (auto &code: codes)
            code.clear();
        delete root;
        root = nullptr;
        std::fill(entries, entries + max_chars, 0);
        count = 0;
        input_size = 0;
        output_size = 0;
    }

    Tree::Tree() {
        std::fill(entries, entries + max_chars, 0);
    }


    Node::Node(std::vector<unsigned char> chars, long long frequency, Node *left_child, Node *right_child)
            : chars(std::move(chars)), frequency(frequency), left_child(left_child), right_child(right_child) {}


    Node::~Node() {
        delete left_child;
        delete right_child;
    }

    BitWriter::BitWriter(std::ofstream &out) : out(out) {}


    void BitWriter::flush() {
        if (byte_index > 0) {
            out.write((char *) &byte, sizeof(char));
        }
    }

    BitReader::BitReader(std::ifstream &in) : in(in) {}

    BitReader::operator bool() const {
        return in.operator bool();
    }

}
