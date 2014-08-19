#include "util.hpp"

#include <fstream>
#include <sstream>

namespace util {
    vector<char> read_file(const char* filename) {
        ifstream ifs(filename, ios::in | ios::binary | ios::ate);

        if (!ifs.good()) {
            throw runtime_error(string("Could not open file ") + filename);
        }

        ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, ios::beg);

        vector<char> bytes(fileSize);
        ifs.read(&bytes[0], fileSize);

        return bytes;
    }
}
