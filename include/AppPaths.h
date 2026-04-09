#pragma once

#include <filesystem>
#include <string>

namespace AppPaths
{
std::filesystem::path findResource(const std::string &relativePath);
}
