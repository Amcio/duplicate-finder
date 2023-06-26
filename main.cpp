#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstring> // memcmp()
#include <functional> // std::hash()
#include <unordered_set>
namespace fs = std::filesystem;

class File;

bool binaryFileCompare(const File& file, const File& otherFile, uint32_t limit = 4096);

class File {
    fs::directory_entry entry;
    mutable std::size_t hash = 0;
    public:
        File() {}
        File(fs::directory_entry _entry) : entry(_entry) {}

        const fs::directory_entry getEntry() const {
            return entry;
        }

        const std::string toString() const {
            return entry.path().string();
        }

        std::size_t calculateHash() const {
            if (hash) {
                return hash;
            }
            std::ifstream f = std::ifstream(entry.path().string(), std::ios::binary | std::ios::in);
            std::size_t to_hash = 2048;
            if (entry.file_size() < to_hash) {
                to_hash = entry.file_size();
            }
            char* buf = new char[to_hash];
            f.seekg(0);
            f.read(buf, to_hash);
            hash = std::hash<std::string>{}(std::string(buf, to_hash));
            delete[] buf;
            return hash;
        }

        bool operator==(const File& otherFile) const {
            if (&otherFile == this) {
                return false; // A file is always a duplicate of itself, that's a false-positive
            }
            if (entry.file_size() != otherFile.entry.file_size()) {
                return false;
            }
            // Compare hashes
            if (this->calculateHash() != otherFile.calculateHash()) {
                return false;
            }
            // Carry out binary comparison (first 4 MB should be more than enough)
            return binaryFileCompare(*this, otherFile);
        }
        struct FileComparator {
            bool operator()(const File& a, const File& b) const {
                return a == b;
            }
        };
};

// std::hash implementation for File, used by hashmaps in the STL
template<>
struct std::hash<File> {
    std::size_t operator()(const File& f) const {
        return std::hash<fs::path>{}(f.getEntry().path());
    }
};

struct PairUtilities {
    struct PairComparator {
        bool operator()(const std::pair<fs::path, fs::path>& a, const std::pair<fs::path, fs::path>& b) const {
            return (a.first == b.first && a.second == b.second) || (a.first == b.second && a.second == b.first);
        }
    };

    struct PairHasher {
        std::size_t operator()(const std::pair<fs::path, fs::path>& pair) const {
            return std::hash<fs::path>{}(pair.first) ^ std::hash<fs::path>{}(pair.second); // Combine hashes of two elements (binary and)
        }
    };
};

bool binaryFileCompare(const File& file, const File& otherFile, uint32_t limit) { 
    std::ifstream fh = std::ifstream(file.getEntry().path().string(), std::ios::binary | std::ios::in);
    std::ifstream ofh = std::ifstream(otherFile.getEntry().path().string(), std::ios::binary | std::ios::in);
    char fileBuffer[512], otherBuffer[512];
    uint32_t scanned = 0;
    do {
      fh.read(fileBuffer, 512);
      ofh.read(otherBuffer, 512);
      // If we are here it means the files have the same size, thus we should be able to read the same amount of data from them.
      // Therefore it doesn't matter which gcount is used.
      std::streamsize bytesRead = fh.gcount();
      if (memcmp(fileBuffer, otherBuffer, bytesRead) != 0) {
        return false;
      }
      // Check if EOF here
      fh.seekg(512, std::ios::cur);
      ofh.seekg(512, std::ios::cur);
    } while (((fh.gcount() == ofh.gcount()) && (fh.gcount() == 512)) && (scanned < limit));
    return true;
}

bool cmp(File a, File b) { return a == b; }

std::unordered_set<File, std::hash<File>, File::FileComparator> scanDirectory(const fs::path& directoryPath) {
    std::unordered_set<File, std::hash<File>, File::FileComparator> fileSet;
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry)) {
            File file(entry);
            if (auto [iter, success] = fileSet.insert(file); !success) {
                std::cout << "[*] Found duplicate: " << iter->toString() << " and " << file.toString();
            }
        }
    }
    return fileSet;
}

std::unordered_set<std::pair<fs::path, fs::path>, PairUtilities::PairHasher, PairUtilities::PairComparator> findDuplicates(std::unordered_map<fs::path, File> fileMap) {
    std::unordered_set<std::pair<fs::path, fs::path>, PairUtilities::PairHasher, PairUtilities::PairComparator> duplicateList;
    for (auto& pair : fileMap) {
        for (auto& otherPair : fileMap) {
            std::pair<fs::path, fs::path> p(pair.first, otherPair.first);
            if (duplicateList.count(p) == 1) {
                continue; // Don't waste time comparing B to A if A was already compared to B
            }
            if (pair.second == otherPair.second) {
                duplicateList.insert(p); // Store paths to duplicates
            }
        }
    }
    return duplicateList;
}

void compareTest() {
    fs::path path1("test.txt"), path2("test1.txt");
    fs::directory_entry entry1(path1), entry2(path2);
    File file1(entry1), file2(entry2);
    if (file1 == file2) {
        std::cout << "files are equal\n";
    }
}

int main(int argc, char const *argv[]) {
    if (argc == 1 || fs::is_regular_file(fs::path(argv[1]))) {
      std::cout << "Usage: dupfinder DIRECTORY\n";
      return 0;
    }
    fs::path directory(argv[1]);
    std::unordered_set<File, std::hash<File>, File::FileComparator> fileSet = scanDirectory(directory);
    // std::unordered_set<std::pair<fs::path, fs::path>, PairUtilities::PairHasher, PairUtilities::PairComparator> duplicates = findDuplicates(fileMap);
    // for (const auto& filePair : duplicates) {
    //     std::cout << "[*] Found duplicate: " << filePair.first << " and " << filePair.second << "\n";
    // }
    return 0;
}
