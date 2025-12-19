#include <iostream>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>

//EscapeFromTarkovArena
//EscapeFromTarkov

std::string get_formatted_time() {
  time_t now = time(nullptr);
  struct tm* local = localtime(&now);

  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d-%b-%H-%M", local);

  return std::string(buffer);
}

std::string get_foreground_window_name() {
  HWND hwnd = GetForegroundWindow();
  
  char title[256]; // buffer for window title
  GetWindowTextA(hwnd, title, sizeof(title));

  return std::string(title);
}

bool file_is_stable(const std::filesystem::path& file_path, std::chrono::seconds wait_duration) {
  auto initial_size = std::filesystem::file_size(file_path);
  std::this_thread::sleep_for(wait_duration);
  auto new_size = std::filesystem::file_size(file_path);

  return initial_size == new_size;
}

int handle_new_clip(const std::string& file_name, const std::string&window_name) {
  std::cout << "Handling new clip..." << std::endl;

  std::filesystem::path file_path(file_name);

  // Wait until the file is not being written to
  while (!file_is_stable(file_path, std::chrono::seconds(1))) {
    std::cout << "Waiting for file to stabilize..." << std::endl;
  }

  std::filesystem::path save_path;

  std::string time = get_formatted_time();

  if (window_name == "EscapeFromTarkovArena") {
    save_path = file_path.parent_path() / ("Arena" + time + ".mp4");
  } 

  else if (window_name == "EscapeFromTarkov") {
    save_path = file_path.parent_path() / ("Tarkov" + time + ".mp4");
  } 

  else {
    save_path = file_path.parent_path() / (window_name + time + ".mp4");
  }
  
  std::filesystem::rename(file_path, save_path);
  return 0; 
}

int main() {
  const char* dir = "F:\\temp-clips\\replays";
  
  // Define a struct stat variable to store the directory information
  struct stat stat_buffer;

  // Check if the directory exists
  if (stat(dir, &stat_buffer) != 0) {
    std::cout << "Directory does not exist";
    return -1;
  }

  // Iterate through the directory contents and print the file names
  std::vector<std::string> original_file_names;
  for (const auto & entry : std::filesystem::directory_iterator(dir)) {
      //std::cout << entry.path() << std::endl;
      original_file_names.push_back(entry.path().string());
  }
  
  //for (const auto & file : file_names) {
  //    std::cout << file << ", ";
  //}

  while (true) {
    std::vector<std::string> new_file_names;
    for (const auto & entry : std::filesystem::directory_iterator(dir)) {
        //std::cout << entry.path() << std::endl;
        new_file_names.push_back(entry.path().string());
    }

    if (new_file_names.size() > original_file_names.size()) {
        std::cout << "Change detected in directory contents." << std::endl;

        std::string window_name = get_foreground_window_name();

        handle_new_clip(new_file_names.back(), window_name);

        original_file_names = new_file_names;
    }
  }
  return 0;
}