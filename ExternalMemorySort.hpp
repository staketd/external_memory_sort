#ifndef EXTERNALMEMORYSORT_HPP
#define EXTERNALMEMORYSORT_HPP

#include "TmpFile.h"

using std::string, std::vector;


class ExternalMemorySort {
public:
    ExternalMemorySort(
        string filename,
        string tmp_folder,
        string output_file,
        size_t read_block_size_bytes,
        size_t memory_size_bytes
    );

    void Sort() const;

private:
    vector<TmpFile> MergeLayer(vector<TmpFile>&& tmpFiles) const;

    TmpFile MergeFiles(const vector<TmpFile>& files) const;

    static vector<int> SortInMemory(std::ifstream& input, size_t readInts);

    static void AppendValues(std::ofstream& out, const vector<int>& values, size_t writeSize);

    static vector<int> ReadNextValues(std::ifstream& file, const size_t& readInts);

    static void WriteValues(const std::string& outputPath, const vector<int>& values);

    string Filename_;
    string TmpFolder_;
    string OutputFilename;
    size_t ReadBlockSizeBytes_;
    size_t MemorySizeBytes_;
    size_t InputSizeBytes_;

    size_t MaxBlocksInMemory_;
    size_t Branching_;
};


#endif //EXTERNALMEMORYSORT_HPP
