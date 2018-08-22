#pragma once

class NonCopyable {
public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = default;
  NonCopyable& operator=(const NonCopyable&) = default;
};
