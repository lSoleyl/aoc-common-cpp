#pragma once

#include <string>
#include <fstream>
#include <optional>
#include <iostream>
#include <string_view>

// Manually declare the only used WinAPI function here to avoid including all of windows.h and polluting our namespace just for this.
extern "C" {
  __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void* hModule, char* path, unsigned long size);
}


namespace task {
  /** Retrieves the task id from the name of the executable by stripping the path and the extension
   */
  std::string id() {
    const unsigned long MAX_PATH = 260;
    char path[MAX_PATH];
    std::string_view pathView(path, GetModuleFileNameA(nullptr, path, MAX_PATH));
    pathView.remove_prefix(pathView.find_last_of('\\') + 1); // assumption: we always get a path
    pathView.remove_suffix(4); // .exe
    return std::string(pathView);
  }

  /** This simple function will search the task's input.txt in the predefined locations
   *  and return an ifstream to that file if found. Otherwise an error will be written to the console and an exception thrown.
   */
  std::ifstream input(const char* filename = "input.txt") {
    std::string path = filename;
    std::ifstream file;

    // First try to find input file in current working directory
    file.open(path, std::ios::binary);
    if (file) {
      return file;
    }

    // We must find the input in the /data directory, so we first need the task id
    auto taskId = task::id();

    // When launching from the project directory (debugging in MSVS) the data directory is one folders up
    path = "..\\data\\" + taskId + "\\" + filename;
    file.open(path, std::ios::binary);
    if (file) {
      return file;
    }

    // When launching from the build directory /x64/Release/##.exe then the data directory is two folders up
    path = "..\\" + path;
    file.open(path, std::ios::binary);

    if (!file) {
      // Don't simply return the invalid file stream as the caller would then have to check the result always...
      std::cerr << "ERR: Could not find '" << filename << "'!\n";
      throw std::exception("Could not find input file");
    }

    return file;
  }


  /** Same as input(), but will return the input file content as binary string instead of an input stream
   */
  std::string inputString(const char* filename = "input.txt") {
    auto file = task::input(filename);
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content(fileSize, '\0');
    file.read(const_cast<char*>(content.data()), content.size());
    return content;
  }
}

