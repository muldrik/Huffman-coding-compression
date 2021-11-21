#include <iostream>
#include "huffman.h"
#include "cstring"
#include "cassert"

int main(int argc, char* argv[]) {
    std::string input_file_name, output_file_name;
    int mode = -1;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-c")) mode = 0;
        else if (!strcmp(argv[i], "-u")) mode = 1;
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
            input_file_name = argv[i+1];
            i++;
        }
        else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            output_file_name = argv[i+1];
            i++;
        }
    }
    Huffman::Tree t;
    assert(mode != -1 && !input_file_name.empty() && !output_file_name.empty());
    if (mode == 0)
        t.encodeFile(input_file_name, output_file_name, true);
    else
        t.decodeFile(input_file_name, output_file_name, true);
}
