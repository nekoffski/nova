#include "FileSystem.hh"

#include <fstream>
#include <sstream>

#include "Utils.hh"

namespace sl {

bool FileSystem::isFile(const Path& path) const {
    return std::filesystem::is_regular_file(path);
}

bool FileSystem::isDirectory(const Path& path) const {
    return std::filesystem::is_directory(path);
}

std::vector<std::string> FileSystem::listDirectory(const Path& path) const {
    std::vector<std::string> entries;
    for (auto& entry : std::filesystem::directory_iterator(path))
        entries.emplace_back(entry.path());
    return entries;
}

void FileSystem::writeFile(
  const Path& path, const std::string& buffer, WritePolicy writePolicy
) const {
    auto mode =
      writePolicy == WritePolicy::override ? std::ios::trunc : std::ios::app;

    std::ofstream fileStream;
    fileStream.open(path, mode);
    fileStream << buffer;
    fileStream.close();
}

std::string FileSystem::readFile(const Path& path) const {
    std::ostringstream fileContentStream;
    std::ifstream fs;

    fs.open(path);
    fileContentStream << fs.rdbuf();
    fs.close();

    return fileContentStream.str();
}

std::vector<std::string> FileSystem::readLines(const Path& path) const {
    static const char endOfLine = '\n';
    return split(readFile(path), endOfLine);
}

std::filesystem::file_time_type FileSystem::getLastFileModificationTime(
  const Path& path
) const {
    return std::filesystem::last_write_time(path);
}

FileSystem FileSystem::getDefault() { return FileSystem(); }

}  // namespace sl
