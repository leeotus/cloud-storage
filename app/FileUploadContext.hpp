/**
 * @file FileUploadContext.hpp
 * @author leeotus@163.com
 * @brief 文件上传的上下文
 */

#ifndef __CLOUD_STORAGE_FILEUPLOAD_CONTEXT_HPP__
#define __CLOUD_STORAGE_FILEUPLOAD_CONTEXT_HPP__

#include "utils/log/Logging.hpp"
#include <nlohmann/json.hpp>

#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <unistd.h>

namespace flkeeper {

using namespace flkeeper;
using namespace flkeeper::log;
using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

class FileUploadContext {
public:
  enum class State {
    kExpectHeaders,  // 等待头部
    kExpectContent,  // 等待文件内容
    kExpectBoundary, // 等待边界
    kComplete        // 上传完成
  };

  FileUploadContext(const std::string &filename,
                    const std::string &originalFilename)
      : filename_(filename), originalFilename_(originalFilename),
        totalBytes_(0), state_(State::kExpectHeaders), boundary_("") {
    // 确保目录存在
    fs::path filePath(filename_);
    fs::path dir = filePath.parent_path();
    if (!dir.empty() && !fs::exists(dir)) {
      fs::create_directories(dir);
    }

    // 打开文件
    file_.open(filename_, std::ios::binary | std::ios::out);
    if (!file_.is_open()) {
      LOG_ERROR << "Failed to open file: " << filename;
      throw std::runtime_error("Failed to open file: " + filename);
    }
    LOG_INFO << "Creating file: " << filename
             << ", original name: " << originalFilename;
  }

  ~FileUploadContext() {
    if (file_.is_open()) {
      file_.close();
    }
  }

  void writeData(const char *data, size_t len) {
    if (!file_.is_open()) {
      throw std::runtime_error("File is not open: " + filename_);
    }

    if (!file_.write(data, len)) {
      throw std::runtime_error("Failed to write to file: " + filename_);
    }

    file_.flush(); // 确保数据写入磁盘
    totalBytes_ += len;
    // LOG_INFO << "Wrote " << len << " bytes, total: " << totalBytes_;
  }

  uintmax_t getTotalBytes() const { return totalBytes_; }
  const std::string &getFilename() const { return filename_; }
  const std::string &getOriginalFilename() const { return originalFilename_; }

  void setBoundary(const std::string &boundary) { boundary_ = boundary; }
  State getState() const { return state_; }
  void setState(State state) { state_ = state; }
  const std::string &getBoundary() const { return boundary_; }

private:
  std::string filename_;         // 保存在服务器上的文件名
  std::string originalFilename_; // 原始文件名
  std::ofstream file_;
  uintmax_t totalBytes_;
  State state_;          // 当前状态
  std::string boundary_; // multipart边界
};

}   // namespace flkeeper

#endif
