#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct RuntimePaths {
  std::filesystem::path data_dir;
  std::filesystem::path conf_dir;
  std::filesystem::path out_dir;
  std::filesystem::path exe_dir;
  std::filesystem::path repo_root;
  std::vector<std::filesystem::path> data_fallbacks;
  std::vector<std::filesystem::path> conf_fallbacks;
};

RuntimePaths resolve_runtime_paths(const char* argv0,
                                  const std::string& cli_data_dir,
                                  const std::string& cli_conf_dir,
                                  const std::string& cli_out_dir,
                                  const std::string& out_subdir_hint);

std::filesystem::path resolve_data_path(const RuntimePaths& paths, const std::string& file);
std::filesystem::path resolve_conf_path(const RuntimePaths& paths, const std::string& file);
std::filesystem::path resolve_out_path(const RuntimePaths& paths, const std::string& file);
std::filesystem::path resolve_common_data_path(const RuntimePaths& paths, const std::string& file);

std::string describe_data_search(const RuntimePaths& paths);
std::string describe_conf_search(const RuntimePaths& paths);
std::string describe_common_data_search(const RuntimePaths& paths);
