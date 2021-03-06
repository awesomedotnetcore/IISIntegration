// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

class NonCopyable {
public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = default;
  NonCopyable& operator=(const NonCopyable&) = default;
};
