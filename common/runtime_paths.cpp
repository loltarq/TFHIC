#include "runtime_paths.h"
#include "tfhic_config.h"

#include <cstdlib>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

static bool dir_exists(const fs::path& p) {
  std::error_code ec;
  return !p.empty() && fs::exists(p, ec) && fs::is_directory(p, ec);
}

static bool ensure_dir(const fs::path& p) {
  if (p.empty()) return false;
  std::error_code ec;
  if (fs::exists(p, ec)) return fs::is_directory(p, ec);
  return fs::create_directories(p, ec);
}

static fs::path exe_dir_from_argv0(const char* argv0) {
  if (!argv0 || std::string(argv0).empty()) return fs::current_path();
  fs::path p(argv0);
  if (p.has_parent_path()) {
    std::error_code ec;
    fs::path abs = fs::absolute(p, ec);
    return (ec ? p : abs).parent_path();
  }
  return fs::current_path();
}

static bool has_repo_layout(const fs::path& p) {
  return dir_exists(p / "blastwave") &&
         dir_exists(p / "thermal_yields") &&
         dir_exists(p / "common");
}

static bool looks_like_build_dir(const fs::path& p) {
  if (!fs::exists(p / "CMakeCache.txt") && !dir_exists(p / "CMakeFiles")) return false;
  if (!p.has_parent_path()) return false;
  // If parent also matches the repo layout, prefer parent as the true source root.
  return has_repo_layout(p.parent_path());
}

static fs::path find_repo_root(const fs::path& start) {
  fs::path cur = start;
  fs::path last_match;
  for (int i = 0; i < 6; ++i) {
    if (has_repo_layout(cur)) {
      if (!looks_like_build_dir(cur)) return cur;
      last_match = cur;
    }
    if (!cur.has_parent_path()) break;
    cur = cur.parent_path();
  }
  return last_match;
}

static std::string get_env(const char* key) {
  const char* v = std::getenv(key);
  return v ? std::string(v) : std::string();
}

static fs::path pick_dir(const std::string& cli,
                         const std::string& env,
                         const fs::path& install_default,
                         const std::vector<fs::path>& fallbacks,
                         const fs::path& final_fallback)
{
  if (!cli.empty()) return fs::path(cli);
  if (!env.empty()) return fs::path(env);
  if (dir_exists(install_default)) return install_default;
  for (const auto& p : fallbacks) {
    if (dir_exists(p)) return p;
  }
  return final_fallback;
}

static fs::path pick_out_dir(const std::string& cli,
                             const std::string& env,
                             const fs::path& install_default,
                             const std::vector<fs::path>& fallbacks,
                             const fs::path& final_fallback)
{
  if (!cli.empty()) return fs::path(cli);
  if (!env.empty()) return fs::path(env);
  if (!install_default.empty() && ensure_dir(install_default)) return install_default;
  for (const auto& p : fallbacks) {
    if (ensure_dir(p)) return p;
  }
  if (ensure_dir(final_fallback)) return final_fallback;
  return {};
}

RuntimePaths resolve_runtime_paths(const char* argv0,
                                  const std::string& cli_data_dir,
                                  const std::string& cli_conf_dir,
                                  const std::string& cli_out_dir,
                                  const std::string& out_subdir_hint)
{
  RuntimePaths paths;
  paths.exe_dir = exe_dir_from_argv0(argv0);
  paths.repo_root = find_repo_root(paths.exe_dir);

  const std::string env_data = get_env("TFHIC_DATA");
  const std::string env_conf = get_env("TFHIC_CONF");
  const std::string env_out  = get_env("TFHIC_OUT");

  fs::path install_data = TFHIC_INSTALL_DATA_DIR;
  fs::path install_conf = TFHIC_INSTALL_CONF_DIR;
  fs::path install_out  = TFHIC_INSTALL_OUT_DIR;

  std::vector<fs::path> data_fallbacks;
  std::vector<fs::path> conf_fallbacks;
  std::vector<fs::path> out_fallbacks;

  if (!paths.repo_root.empty()) {
    data_fallbacks.push_back(paths.repo_root / "blastwave" / "data");
    conf_fallbacks.push_back(paths.repo_root / "thermal_yields" / "conf");
    if (!out_subdir_hint.empty()) {
      out_fallbacks.push_back(paths.repo_root / out_subdir_hint);
    }
    fs::path common_data = paths.repo_root / "common" / "data";
    if (dir_exists(common_data)) paths.data_fallbacks.push_back(common_data);
  }

  data_fallbacks.push_back(paths.exe_dir / ".." / "data");
  conf_fallbacks.push_back(paths.exe_dir / ".." / "conf");

  paths.data_dir = pick_dir(cli_data_dir, env_data, install_data, data_fallbacks, fs::path("data"));
  paths.conf_dir = pick_dir(cli_conf_dir, env_conf, install_conf, conf_fallbacks, fs::path("conf"));
  paths.out_dir  = pick_out_dir(cli_out_dir, env_out, install_out, out_fallbacks,
                                fs::current_path() / "out");

  paths.data_fallbacks.insert(paths.data_fallbacks.end(), data_fallbacks.begin(), data_fallbacks.end());
  paths.conf_fallbacks.insert(paths.conf_fallbacks.end(), conf_fallbacks.begin(), conf_fallbacks.end());

  return paths;
}

static bool is_bare_name(const fs::path& p) {
  return !p.empty() && !p.is_absolute() && !p.has_parent_path();
}

static fs::path resolve_in_dirs(const std::string& file,
                                const fs::path& primary,
                                const std::vector<fs::path>& fallbacks)
{
  fs::path p(file);
  if (!is_bare_name(p)) return p;

  fs::path candidate = primary / p;
  {
    std::error_code ec;
    if (dir_exists(candidate.parent_path()) && fs::exists(candidate, ec) && !ec) return candidate;
  }

  for (const auto& fb : fallbacks) {
    fs::path c = fb / p;
    std::error_code ec;
    if (dir_exists(c.parent_path()) && fs::exists(c, ec) && !ec) return c;
  }

  return candidate;
}

std::filesystem::path resolve_data_path(const RuntimePaths& paths, const std::string& file) {
  if (file.empty()) return {};
  return resolve_in_dirs(file, paths.data_dir, paths.data_fallbacks);
}

std::filesystem::path resolve_conf_path(const RuntimePaths& paths, const std::string& file) {
  if (file.empty()) return {};
  return resolve_in_dirs(file, paths.conf_dir, paths.conf_fallbacks);
}

std::filesystem::path resolve_out_path(const RuntimePaths& paths, const std::string& file) {
  if (file.empty()) return {};
  fs::path p(file);
  if (!is_bare_name(p)) return p;
  return paths.out_dir / p;
}

std::filesystem::path resolve_common_data_path(const RuntimePaths& paths, const std::string& file) {
  if (file.empty()) return {};
  fs::path p(file);
  if (!is_bare_name(p)) return p;

  if (!paths.repo_root.empty()) {
    fs::path candidate = paths.repo_root / "common" / "data" / p;
    std::error_code ec;
    if (dir_exists(candidate.parent_path()) && fs::exists(candidate, ec) && !ec) return candidate;
  }

  fs::path cur = paths.exe_dir;
  for (int i = 0; i < 6; ++i) {
    fs::path candidate = cur / "common" / "data" / p;
    std::error_code ec;
    if (dir_exists(candidate.parent_path()) && fs::exists(candidate, ec) && !ec) return candidate;
    if (!cur.has_parent_path()) break;
    cur = cur.parent_path();
  }

  return resolve_data_path(paths, file);
}

static std::vector<fs::path> common_data_search_dirs(const RuntimePaths& paths) {
  std::vector<fs::path> dirs;
  if (!paths.repo_root.empty()) {
    dirs.push_back(paths.repo_root / "common" / "data");
  }
  fs::path cur = paths.exe_dir;
  for (int i = 0; i < 6; ++i) {
    dirs.push_back(cur / "common" / "data");
    if (!cur.has_parent_path()) break;
    cur = cur.parent_path();
  }
  return dirs;
}

static std::string describe_dirs(const std::vector<fs::path>& dirs) {
  std::ostringstream out;
  for (const auto& p : dirs) {
    out << "  - " << p.string() << "\n";
  }
  return out.str();
}

std::string describe_data_search(const RuntimePaths& paths) {
  std::vector<fs::path> dirs;
  dirs.push_back(paths.data_dir);
  dirs.insert(dirs.end(), paths.data_fallbacks.begin(), paths.data_fallbacks.end());
  return describe_dirs(dirs);
}

std::string describe_conf_search(const RuntimePaths& paths) {
  std::vector<fs::path> dirs;
  dirs.push_back(paths.conf_dir);
  dirs.insert(dirs.end(), paths.conf_fallbacks.begin(), paths.conf_fallbacks.end());
  return describe_dirs(dirs);
}

std::string describe_common_data_search(const RuntimePaths& paths) {
  auto dirs = common_data_search_dirs(paths);
  // Fall back to data dirs as well since resolve_common_data_path will.
  dirs.push_back(paths.data_dir);
  dirs.insert(dirs.end(), paths.data_fallbacks.begin(), paths.data_fallbacks.end());
  return describe_dirs(dirs);
}
