#pragma once

#include "queue"
#include "string"
#include "vector"
#include "fstream"
#include "list"

namespace Huffman {
    const int byte_size = 8;

    class Node {
    public:
        Node(std::vector<unsigned char> chars, long long frequency, Node* left_child = nullptr, Node* right_child = nullptr);
        ~Node();
        std::vector<unsigned char> chars;
        long long frequency;
        Node* left_child = nullptr;
        Node* right_child = nullptr;
    };

    struct NodePtrComp
    {
        bool operator()(const Node* first, const Node* second) const {
            return first->frequency > second->frequency;
        }
    };

    class BitWriter {
    public:
        explicit BitWriter(std::ofstream& out);
        void flush();

        template<class T>
        friend BitWriter& operator<<(BitWriter& w, T& var) {
            w.flush();
            w.out.write((char*) &var, sizeof(var));
            return w;
        }

        friend BitWriter& operator<<(BitWriter& w, bool& b) {
            if (!b)
                w.byte &= ~(1 << (byte_size - 1 - w.byte_index));
            else
                w.byte |= (1 << (byte_size - 1 - w.byte_index));
            w.byte_index++;
            if (w.byte_index == byte_size) {
                w.out.write((char*) &w.byte, sizeof(char));
                w.byte_index = 0;
                w.byte = 0;
            }
            return w;
        }

    private:
        unsigned char byte = 0;
        int byte_index = 0;
        std::ofstream& out;
    };

    class BitReader {
    public:
        explicit BitReader(std::ifstream& in);

        template<class T>
        friend BitReader& operator>>(BitReader& r, T& var) {
            r.in.read((char*) &var, sizeof(var));
            return r;
        }

        friend BitReader& operator>>(BitReader& r, bool& b) {
            if (r.byte_index == 0) r >> r.byte;
            b = r.byte & (1 << (byte_size - 1 - r.byte_index++));
            if (r.byte_index == byte_size) {
                r.byte_index = 0;
            }
            return r;
        }

        operator bool() const;


    private:
        unsigned char byte = 0;
        int byte_index = 0;
        std::ifstream& in;
    };


    class Tree {
    public:

        Tree();
        ~Tree();
        void encodeFile(std::string& input_file_name, std::string& output_file_name, bool print_stat = false, bool clear_on_exit = true);
        void decodeFile(std::string& input_file_name, std::string& output_file_name, bool print_stat = false, bool clear_on_exit = true);

        void clear();

        static const int max_chars = 256;
        static constexpr int extra_bytes = (1 + max_chars) * sizeof (long long);
        Node* root = nullptr;
        std::vector<bool> codes[max_chars];


    private:
#ifdef MY_TESTS
        public:
#endif
        long long input_size = 0;
        long long output_size = 0;
        std::priority_queue<Node*, std::vector<Node*>, NodePtrComp> nodes;
        long long entries[max_chars];
        long long count = 0;
        void loadRawEntries(std::ifstream& in);
        void buildTree();
        void mergeTree();
        void encodeAndWriteCompressed (std::ifstream& in, std::ofstream& out);
        void loadEncodedTree(std::ifstream& in);
        void decodeAndWriteText(std::ifstream& in, std::ofstream& out);
    };
}







