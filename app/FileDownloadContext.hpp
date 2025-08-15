#ifndef __CLOUD_STORAGE_FILEDOWNLOAD_CONTEXT_HPP__
#define __CLOUD_STORAGE_FILEDOWNLOAD_CONTEXT_HPP__

namespace flkeeper {

using namespace flkeeper;
using namespace flkeeper::log;
using json = nlohmann::json;
namespace fs = std::experimental::filesystem;


// 文件下载上下文
class FileDownContext {
public:
  FileDownContext(const std::string &filepath,
                  const std::string &originalFilename)
      : filepath_(filepath), originalFilename_(originalFilename), fileSize_(0),
        currentPosition_(0), isComplete_(false) {
    // 获取文件大小
    fileSize_ = fs::file_size(filepath_);

    // 打开文件
    file_.open(filepath_, std::ios::binary | std::ios::in);
    if (!file_.is_open()) {
      LOG_ERROR << "Failed to open file: " << filepath_;
      throw std::runtime_error("Failed to open file: " + filepath_);
    }
    LOG_INFO << "Opening file for download: " << filepath_
             << ", size: " << fileSize_;
  }

  ~FileDownContext() {
    if (file_.is_open()) {
      file_.close();
    }
  }

  void seekTo(uintmax_t position) {
    if (!file_.is_open()) {
      throw std::runtime_error("File is not open: " + filepath_);
    }
    file_.seekg(position);
    currentPosition_ = position;
    isComplete_ = false;
  }

  // @brief 从文件中读取下一个数据块,用于将文件分块传输给客户端
  bool readNextChunk(std::string &chunk) {
    if (!file_.is_open() || isComplete_) {
      return false;
    }

    const uintmax_t chunkSize = 1024 * 1024; // 1MB
    uintmax_t remainingBytes = fileSize_ - currentPosition_;
    uintmax_t bytesToRead = std::min(chunkSize, remainingBytes);

    if (bytesToRead == 0) {
      isComplete_ = true;
      return false;
    }

    std::vector<char> buffer(bytesToRead);
    file_.read(buffer.data(), bytesToRead);
    chunk.assign(buffer.data(), bytesToRead);
    currentPosition_ += bytesToRead;

    LOG_INFO << "Read chunk of " << bytesToRead
             << " bytes, current position: " << currentPosition_ << "/"
             << fileSize_;
    return true;
  }

  bool isComplete() const { return isComplete_; }
  uintmax_t getCurrentPosition() const { return currentPosition_; }
  uintmax_t getFileSize() const { return fileSize_; }
  const std::string &getOriginalFilename() const { return originalFilename_; }

private:
  std::string filepath_;         // 文件路径
  std::string originalFilename_; // 原始文件名
  std::ifstream file_;           // 文件流
  uintmax_t fileSize_;           // 文件总大小，使用 uintmax_t 替代 size_t
  uintmax_t currentPosition_;    // 当前读取位置，使用 uintmax_t 替代 size_t
  bool isComplete_;              // 是否完成
};

}   // namespace flkeeper

#endif
