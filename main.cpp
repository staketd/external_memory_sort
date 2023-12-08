#include <iostream>
#include <fstream>
#include <queue>
#include <filesystem>
#include "ExternalMemorySort.hpp"

using std::cout, std::string, std::vector, std::pair;

int main(int argc, char** argv) {
    if (argc < 5) {
        cout << "Too few arguments\n";
        return 1;
    }

    size_t readBlockSizeBytes = std::atol(argv[1]);
    size_t memorySizeBytes = std::atol(argv[2]);

    string file = argv[3];
    string tmpFolder = argv[4];
    string outputFile = argv[5];

    std::filesystem::remove(outputFile);
    std::filesystem::remove_all(tmpFolder);

    auto sort = ExternalMemorySort(file,
                                   tmpFolder,
                                   outputFile,
                                   readBlockSizeBytes - readBlockSizeBytes % 4,
                                   memorySizeBytes - memorySizeBytes % 4
    );

    sort.Sort();
}
