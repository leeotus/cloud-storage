#include "utils/datastructures/FKString.hpp"
#include <cstring>

namespace flkeeper {

FKString::FKString() : sptr_(nullptr), length_(0) {}

FKString::FKString(const char *str)
    : sptr_(str), length_(static_cast<DFLK_INT64>(strlen(str))) {}

FKString::FKString(const unsigned char *str)
    : sptr_(reinterpret_cast<const char *>(str)),
      length_(static_cast<DFLK_INT64>(strlen(sptr_))) {}

FKString::FKString(const std::string &str)
    : sptr_(str.data()), length_(static_cast<DFLK_INT64>(str.size())) {}

FKString::FKString(const char *offset, int len) : sptr_(offset), length_(len) {}

const char* FKString::data() const {
  return sptr_;
}

int FKString::size() const {
  return length_;
}

bool FKString::empty() const {
  return length_ == 0;
}

void FKString::clear() {
  sptr_ = nullptr;
  length_ = 0;
}

void FKString::set(const char *str, DFLK_INT64 len) {
  sptr_ = str;
  length_ = len;
}

void FKString::set(const char *str) {
  sptr_ = str;
  length_ = static_cast<DFLK_INT64>(strlen(str));
}

void FKString::set(void *buffer, DFLK_INT64 len) {
  sptr_ = reinterpret_cast<const char *>(buffer);
  length_ = len;
}

char FKString::operator[](int index) const {
  return sptr_[index];
}

void FKString::remove_prefix(int n)
{
  sptr_ += n;
  length_ -= n;
}

void FKString::remove_suffix(int n)
{
  length_ -= n;
}

bool FKString::operator==(const FKString& fkstr) const {
  return ((length_ == fkstr.size()) && !strncmp(sptr_, fkstr.data(), length_));
}

bool FKString::operator!=(const FKString &fkstr) const {
  return ((length_ != fkstr.size()) || !strncmp(sptr_, fkstr.data(), length_));
}

std::string FKString::as_string() const {
  return std::string(this->data(), this->size());
}

void FKString::CopyToString(std::string *target) const {
  target->assign(sptr_, length_);
}

bool FKString::StartWith(const FKString &fkstr) const {
  return ((length_ >= fkstr.length_) && !memcmp(sptr_, fkstr.sptr_, fkstr.length_));
}

} // namespace flkeeper
