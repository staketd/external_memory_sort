#pragma once

#include <string>
#include <filesystem>

using std::string;

struct TmpFile {
    size_t Size;
    string Id;
    string Path;

    TmpFile(size_t size, const string& id, const string& tmpFolderPath): Size(size), Id(id),
                                                                         Path(std::filesystem::path(tmpFolderPath) /
                                                                              std::filesystem::path(id)) {
    }

    bool operator<(const TmpFile& other) const {
        return Size < other.Size;
    }
};
