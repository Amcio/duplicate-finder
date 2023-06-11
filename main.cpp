#include <iostream>
#include <fstream>
#include <filesystem>
#include <list>
#include <string>
#include <cstring> // memcmp()
#include <functional> // std::hash()
namespace fs = std::filesystem;

class File;

bool binaryFileCompare(const File& file, const File& otherFile, uint32_t limit = 8192);

class File {
    fs::directory_entry entry;
    std::size_t hash = 0;
    struct buffer {
        char buffer[512];
    };
    public:
        File(fs::directory_entry _entry) : entry(_entry) {}
        const fs::directory_entry getEntry() const {
            return entry;
        }
        // GetHashes() should return the cached hashes, and if they're not there it should call calculateHashes().
        // Looks like it will be another iterator, because we want to evaluate one hash at a time.
        // A private property "scanned" should contain the position of the scanner (seek to).
        // Constructor and oprator==
        std::size_t calculateHash() {
            // TODO: Either switch to a single hash, or provide consecutive hashes of larger chunks of the file.
            // The latter option requires tracking how much of the file we hashed. Otherwise you will compare 2048 bytes
            // of one file to 4096 bytes of another.
            if (!hash) {
                return hash;
            }
            std::ifstream f = std::ifstream(entry.path().stem().string(), std::ios::binary | std::ios::in);
            std::size_t to_hash = 2048;
            if (entry.file_size() < to_hash) {
                to_hash = entry.file_size();
            }
            char* buf = new char[to_hash];
            f.seekg(0);
            f.read(buf, to_hash);
            hash = std::hash<char>{}(*buf);
            delete[] buf;
            return hash;
            //for (auto start = std::istream_iterator<buffer>{ f }, end = std::istream_iterator<buffer>{}; start != end; ++start) {}
        }
        bool operator==(File& otherFile) {
            if (entry.file_size() != otherFile.entry.file_size()) {
                return false;
            }
            // Compare hashes
            if (this->calculateHash() != otherFile.calculateHash()) {
                return false;
            }
            // Carry out binary comparison (first 8 MB should be more than enough)
            return binaryFileCompare(*this, otherFile);
        }
};

bool binaryFileCompare(const File& file, const File& otherFile, uint32_t limit) {
    std::ifstream fh = std::ifstream(file.getEntry().path().stem().string(), std::ios::binary | std::ios::in);
    std::ifstream ofh = std::ifstream(otherFile.getEntry().path().stem().string(), std::ios::binary | std::ios::in);
    char fileBuffer[512], otherBuffer[512];
    uint32_t scanned = 0;
    do {
      fh.read(fileBuffer, 512);
      ofh.read(otherBuffer, 512);
      if (memcmp(fileBuffer, otherBuffer, 512) != 0)
        return false;
      fh.seekg(512, std::ios::cur);
      ofh.seekg(512, std::ios::cur);
    } while (((fh.gcount() == ofh.gcount()) && (fh.gcount() == 512)) && (scanned < limit));
    return true;
}

std::vector<fs::path> scanDirectory(const fs::path& directoryPath) {
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry)) {
            // actually, maybe use a hash map?
            // Comparing each file to each file seems like a lot of work (O(n^2) ?
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc == 1) {
      std::cout << "Usage: dupfinder DIRECTORY\n";
      return 0;
    }
    std::unordered_map<fs::path, File>();
    fs::path directory(argv[1]);
    
    return 0;
}
