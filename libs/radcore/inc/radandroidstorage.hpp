#ifndef RADANDROIDSTORAGE_HPP
#define RADANDROIDSTORAGE_HPP

#if defined(RAD_ANDROID)
#include <SDL_system.h>
#include <string>

// Derive the shared folder from Android/data/<package>/files without relying on
// a particular storage volume or user. Normalize lexically; no filesystem I/O.
inline std::string radBuildAndroidSharedStoragePath(const char* externalFilesPath)
{
    const char* fallback = "/storage/emulated/0/c4rloxSimpsons";
    if (!externalFilesPath || !externalFilesPath[0])
        return fallback;

    std::string root(externalFilesPath);
    for (char& c : root)
        if (c == '\\') c = '/';
    while (root.size() > 1 && root.back() == '/')
        root.pop_back();
    if (root[0] != '/')
        return fallback;

    const char* expected[] = { "files", NULL, "data", "Android" };
    for (unsigned int i = 0; i < 4; ++i)
    {
        const std::string::size_type separator = root.find_last_of('/');
        if (separator == std::string::npos)
            return fallback;
        const std::string name = root.substr(separator + 1);
        if (name.empty() || name == "." || name == ".." ||
            (expected[i] && name != expected[i]))
            return fallback;
        root.erase(separator);
    }
    if (root.empty())
        return fallback;
    return root + "/c4rloxSimpsons";
}

inline const char* radGetAndroidSharedStoragePath()
{
    // The selected volume is fixed for this run, just like SDL's cached path.
    static const std::string path =
        radBuildAndroidSharedStoragePath(SDL_AndroidGetExternalStoragePath());
    return path.c_str();
}
#endif

#endif // RADANDROIDSTORAGE_HPP
