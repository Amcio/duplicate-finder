#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <list>
#include <string>
#include <cstring> // memcmp()

namespace fs = std::filesystem;

class File {
    fs::directory_entry entry;
    std::list<std::string> hashes;
    struct buffer {
        char buffer[512];
    };
    public:
        const fs::directory_entry getEntry() const {
            return entry;
        }
        // GetHashes() should return the cached hashes, and if they're not there it should call calculateHashes().
        // Looks like it will be another iterator, because we want to evaluate one hash at a time.
        // A private property "scanned" should contain the position of the scanner (seek to).
        // Constructor and oprator==
        void calculateHash() {
            std::ifstream f = std::ifstream(entry.path().stem().string(), std::ios::binary | std::ios::in);
            for (auto start = std::istream_iterator<buffer>{ f }, end = std::istream_iterator<buffer>{}; start != end; ++start) {
                
            }
        }
        bool operator==(const File& otherFile) {
            if (entry.file_size() == otherFile.entry.file_size()) {
              return true;
            }
            // Carry out binary comparison (first 2 MB should be more than enough)

            return binaryFileCompare(*this, otherFile);
        }
        /*
        std::istream& operator>>(buffer& bytes) {
            s.read(bytes.buffer, sizeof(bytes.buffer));
            return s;
        }
        */
};

bool binaryFileCompare(const File& file, const File& otherFile) {
    std::ifstream fh = std::ifstream(file.getEntry().path().stem().string(), std::ios::binary | std::ios::in);
    std::ifstream ofh = std::ifstream(otherFile.getEntry().path().stem().string(), std::ios::binary | std::ios::in);
    char fileBuffer[512], otherBuffer[512];
    do {
      fh.read(fileBuffer, 512);
      ofh.read(otherBuffer, 512);
      if (memcmp(fileBuffer, otherBuffer, 512) == 0)
        return true;
      fh.seekg(512, std::ios::cur);
      ofh.seekg(512, std::ios::cur);
    } while (fh.gcount() == 512);
    return false;
}

int main(int argc, char const *argv[]) {
    return 0;
}
