#include "ExternalMemorySort.hpp"
#include <queue>
#include <fstream>

ExternalMemorySort::ExternalMemorySort(
        string filename,
        string tmp_folder,
        string output_file,
        size_t read_block_size_bytes,
        size_t memory_size_bytes
    ): Filename_(std::move(filename)),
       TmpFolder_(std::move(tmp_folder)),
       OutputFilename(std::move(output_file)),
       ReadBlockSizeBytes_(read_block_size_bytes),
       MemorySizeBytes_(memory_size_bytes) {
    InputSizeBytes_ = std::filesystem::file_size(Filename_);
    MaxBlocksInMemory_ = memory_size_bytes / ReadBlockSizeBytes_;
    Branching_ = MaxBlocksInMemory_ - 1;
}

void ExternalMemorySort::Sort() const {
    std::filesystem::create_directory(TmpFolder_);

    vector<TmpFile> filesToMerge;

    std::ifstream inputFile(Filename_, std::ios_base::binary | std::ios_base::in);

    size_t ind = 0;
    for (size_t i = 0; i < InputSizeBytes_; i += MemorySizeBytes_) {
        size_t l = i;
        size_t r = std::min(i + MemorySizeBytes_, InputSizeBytes_);
        TmpFile file(r - l, std::to_string(ind++), TmpFolder_);

        filesToMerge.push_back(file);

        vector<int> values = SortInMemory(inputFile, (r - l) / 4);
        WriteValues(file.Path, values);
    }

    while (filesToMerge.size() != 1) {
        filesToMerge = MergeLayer(std::move(filesToMerge));
    }

    std::filesystem::rename(filesToMerge[0].Path, OutputFilename);

    std::filesystem::remove(TmpFolder_);
}

vector<TmpFile> ExternalMemorySort::MergeLayer(vector<TmpFile>&& tmpFiles) const {

    vector<TmpFile> result;

    std::sort(tmpFiles.begin(), tmpFiles.end(), [](const TmpFile& a, const TmpFile& b) {
        return a.Size < b.Size;
    });

    size_t start = 0;

    if (tmpFiles.size() % Branching_ != 0 && tmpFiles.size() > Branching_) {
        vector<TmpFile> filesToMerge;
        string newId;

        for (size_t i = 0; i < tmpFiles.size() % Branching_ + 1; ++i) {
            filesToMerge.push_back(tmpFiles[i]);
            newId += tmpFiles[i].Id;
        }

        if (!filesToMerge.empty()) {
            TmpFile newFile = MergeFiles(filesToMerge);
            tmpFiles.push_back(newFile);
            start = filesToMerge.size();
        }
    }

    for (size_t i = start; i < tmpFiles.size(); i += Branching_) {
        size_t l = i;
        size_t r = std::min(tmpFiles.size(), i + Branching_);

        vector<TmpFile> filesToMerge;
        size_t sumSize = 0;
        for (size_t j = 0; j < r - l; ++j) {
            filesToMerge.push_back(tmpFiles[i + j]);
            sumSize += filesToMerge.back().Size;
        }

        TmpFile newFile = MergeFiles(filesToMerge);

        result.push_back(newFile);
    }

    return result;
}

TmpFile ExternalMemorySort::MergeFiles(const vector<TmpFile>& files) const {
    vector outputBuffer(ReadBlockSizeBytes_ / 4, 0);
    vector<vector<int>> inputBuffers;
    vector<std::ifstream> fileStreams;
    vector inputBufferOffsets(files.size(), 0ul);
    vector inputFileSizes(files.size(), 0ul);
    vector inputFileOffsets(files.size(), 0ul);

    std::priority_queue<std::pair<int, size_t>, vector<std::pair<int, size_t>>, std::greater<>> heap;
    string newFileId;
    size_t newFileSize = 0;

    for (size_t i = 0; i < files.size(); ++i) {
        inputFileSizes[i] = std::filesystem::file_size(files[i].Path) / 4;
        fileStreams.emplace_back(files[i].Path, std::ios_base::binary | std::ios_base::in);

        inputBuffers.emplace_back(ReadNextValues(
            fileStreams[i],
            std::min(inputFileSizes[i], ReadBlockSizeBytes_ / 4)
        ));

        heap.emplace(inputBuffers.back()[0], i);
        newFileId += files[i].Id;
        newFileSize += files[i].Size;
    }

    TmpFile outputFile(newFileSize, newFileId, TmpFolder_);
    size_t outputBufferOffset = 0;
    std::ofstream out(outputFile.Path, std::ios_base::binary | std::ios_base::app);

    while (!heap.empty()) {
        auto [value, file_idx] = heap.top();
        heap.pop();

        outputBuffer[outputBufferOffset++] = value;

        if (outputBufferOffset == outputBuffer.size()) {
            AppendValues(out, outputBuffer, outputBufferOffset);
            outputBufferOffset = 0;
        }

        inputBufferOffsets[file_idx]++;
        inputFileOffsets[file_idx]++;
        if (inputFileOffsets[file_idx] == inputFileSizes[file_idx]) {
            continue;
        }

        if (inputBufferOffsets[file_idx] == inputBuffers[file_idx].size()) {
            inputBuffers[file_idx] = ReadNextValues(
                fileStreams[file_idx],
                std::min(inputFileSizes[file_idx] - inputFileOffsets[file_idx], ReadBlockSizeBytes_ / 4)
            );
            inputBufferOffsets[file_idx] = 0;
        }

        heap.emplace(inputBuffers[file_idx][inputBufferOffsets[file_idx]], file_idx);
    }

    if (outputBufferOffset != 0) {
        AppendValues(out, outputBuffer, outputBufferOffset);
    }

    for (auto& file: files) {
        std::filesystem::remove(file.Path);
    }

    return outputFile;
}

vector<int> ExternalMemorySort::SortInMemory(std::ifstream& input, const size_t readInts) {
    vector values = ReadNextValues(
        input,
        readInts
    );
    std::ranges::sort(values);
    return values;
}

void ExternalMemorySort::AppendValues(std::ofstream& out, const vector<int>& values, size_t writeSize) {
    out.write((char *) &values[0], writeSize * 4);
    out.flush();
}

vector<int> ExternalMemorySort::ReadNextValues(std::ifstream& file, const size_t& readInts) {
    vector values(readInts, 0);
    file.read((char *) &values[0], readInts * 4);
    return values;
}

void ExternalMemorySort::WriteValues(const std::string& outputPath, const vector<int>& values) {
    std::ofstream out(outputPath, std::ios_base::binary | std::ios_base::app);
    out.write((char *) &values[0], values.size() * 4);
}
