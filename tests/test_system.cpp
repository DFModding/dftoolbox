// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "system.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const std::string& what)
{
  if (!condition)
    {
      std::cerr << "FAIL: " << what << std::endl;
      ++g_failures;
    }
}

std::string read_all(const std::string& filename)
{
  std::vector<char> data = file_read(filename);
  return std::string(data.begin(), data.end());
}

} // namespace

int main()
{
  char arg0[] = "test-system";
  char option[] = "--config-dir=test-system-config";
  char value_option[] = "--cache-dir";
  char value[] = "test-system-cache";
  char* argv[] = { arg0, option, value_option, value, nullptr };
  parse_args(4, argv, "test-system");

  check(get_config_path() == "test-system-config/", "--opt=VALUE is used");
  check(get_cache_path() == "test-system-cache/", "--opt VALUE is used");

  const std::string filename = get_config_path() + "atomic.txt";
  file_write_atomic(filename, "first");
  file_write_atomic(filename, "second\n");
  check(read_all(filename) == "second\n", "file_write_atomic() replaces the file");
  check(!file_exists(filename + ".tmp"), "file_write_atomic() leaves no temporary behind");

  bool thrown = false;
  try
    {
      file_write_atomic("test-system-missing-dir/file.txt", "text");
    }
  catch(std::exception&)
    {
      thrown = true;
    }
  check(thrown, "file_write_atomic() reports failures");

  return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* EOF */
