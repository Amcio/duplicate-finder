#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <list>
#include <string>

namespace fs = std::filesystem;

class File {
    fs::directory_entry entry;
    std::list<std::string> hashes;
    struct buffer {
        char buffer[512];
    };
    public:
        // GetHashes() should return the cached hashes, and if they're not there it should call calculateHashes().
        // Looks like it will be another iterator, because we want to evaluate one hash at a time.
        // A private property "scanned" should contain the position of the scanner (seek to).
        // Constructor and oprator==
        void calculateHash() {
            std::ifstream f = std::ifstream(entry.path().stem().string(), std::ios::binary | std::ios::in);
            for (auto start = std::istream_iterator<buffer>{ f }, end = std::istream_iterator<buffer>{}; start != end; ++start) {
                
            }
        }

        std::istream& operator>>(std::istream& s, buffer& bytes) {
            s.read(bytes.buffer, sizeof(bytes.buffer));
            return s;
        }
};

int main(int argc, char const *argv[]) {
    
    return 0;
}
