#pragma once

#include <vector>
#include <filesystem>

bool should_decompile_file(const char* chunkname, std::filesystem::path possible_dest_path);
void decompile_buffer(const std::vector<char>& buffer, std::filesystem::path destination);