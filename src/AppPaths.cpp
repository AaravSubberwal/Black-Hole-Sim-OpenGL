#include "AppPaths.h"

#include <array>
#include <limits.h>
#include <unistd.h>

namespace
{
std::filesystem::path executableDirectory()
{
    std::array<char, PATH_MAX> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0)
    {
        return {};
    }

    buffer[static_cast<std::size_t>(length)] = '\0';
    return std::filesystem::path(buffer.data()).parent_path();
}

std::array<std::filesystem::path, 4> resourceRoots()
{
    const std::filesystem::path exeDir = executableDirectory();
    return {
        exeDir.parent_path() / "res",
        exeDir.parent_path() / "share" / "blackholesim",
        std::filesystem::path(BLACK_HOLE_SIM_INSTALL_DATA_DIR),
        std::filesystem::path(BLACK_HOLE_SIM_SOURCE_RES_DIR)
    };
}
}

std::filesystem::path AppPaths::findResource(const std::string &relativePath)
{
    for (const auto &root : resourceRoots())
    {
        if (root.empty())
        {
            continue;
        }

        const std::filesystem::path candidate = root / relativePath;
        if (std::filesystem::is_regular_file(candidate))
        {
            return candidate;
        }
    }

    return {};
}
