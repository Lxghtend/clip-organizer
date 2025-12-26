#pragma comment(lib, "User32.lib")
#include <print>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>

std::string read_config_string(
  const std::string& section,
  const std::string& key,
  const std::string& default_value,
  const std::string& filename = "config.ini"
) {
  char buffer[256];

  GetPrivateProfileStringA(
    section.c_str(),
    key.c_str(),
    default_value.c_str(),
    buffer,
    sizeof(buffer),
    filename.c_str()
  );

  return buffer;
}

std::string get_formatted_time() {
  auto now = std::chrono::system_clock::now();
  return std::format("{:%b-%d-%Y_%H-%M}", now);
}

std::string get_day() {
  auto now = std::chrono::system_clock::now();
  return std::format("{:%b-%d}", now);
}

std::string get_executable_from_path(const std::string& path) {
  return std::filesystem::path(path).stem().string();
}

std::string get_foreground_window_path() {
  std::string result;

  HWND hwnd = GetForegroundWindow();

  if (!hwnd) {
    return std::string(result);
  }

  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);

  HANDLE hProcess = OpenProcess(
    PROCESS_QUERY_LIMITED_INFORMATION,
    FALSE,
    pid
  );

  if (!hProcess) {
    return std::string(result);
  }

  char path[MAX_PATH];
  DWORD size = MAX_PATH;

  if (QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
    result = std::string(path, size);
  }

  CloseHandle(hProcess);
  return std::string(result);
}

bool file_is_stable(const std::filesystem::path& file_path, std::chrono::seconds wait_duration) {
  auto initial_size = std::filesystem::file_size(file_path);
  std::this_thread::sleep_for(wait_duration);
  auto new_size = std::filesystem::file_size(file_path);

  return initial_size == new_size;
}

int handle_new_clip(const std::string& file_name, const std::string&executable_name) {
  std::println("Handling new clip...");

  std::string time = get_formatted_time();
  std::string day = get_day();

  std::filesystem::path file_path(file_name);

  // Wait until the file is not being written to
  while (!file_is_stable(file_path, std::chrono::seconds(1))) {
    std::println("Waiting for file to stabilize...");
  }

  std::filesystem::path base_folder = file_path.parent_path();
  
  std::filesystem::path game_folder = base_folder / executable_name;

  std::filesystem::create_directories(game_folder);

  std::filesystem::path save_path = game_folder / (executable_name + "-" + time + ".mp4");

  std::filesystem::path day_folder = game_folder / day;
  std::filesystem::create_directories(day_folder);

  std::filesystem::rename(file_path, save_path);

  std::println("Moved clip to: {}", save_path.string());
  std::println();
  return 0; 
}

int main() {
  const char* dir = "D:\\temp-clips\\replays";

  // Check if the directory exists
  if (!std::filesystem::exists(dir)) {
    std::println("Directory does not exist.");
    return -1;
  }

  // Iterate through the directory contents and print the file names
  std::vector<std::string> original_file_names;
  for (const auto & entry : std::filesystem::directory_iterator(dir)) {
    //std::cout << entry.path() << std::endl;
    original_file_names.push_back(entry.path().string());
  }
  
  //for (const auto & file : file_names) {
  //    std::print("{}, ", file)
  //}

  while (true) {
    std::vector<std::string> new_file_names;
    for (const auto & entry : std::filesystem::directory_iterator(dir)) {
      //std::println("{}", entry.path());
      new_file_names.push_back(entry.path().string());
    }

    if (new_file_names.size() > original_file_names.size()) {
      std::println("Change detected in directory contents.");

      std::string window_path = get_foreground_window_path();
      std::string executable_name = get_executable_from_path(window_path);

      handle_new_clip(new_file_names.back(), executable_name);

      //original_file_names = new_file_names;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}