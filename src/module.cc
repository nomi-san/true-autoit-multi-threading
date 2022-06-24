#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <windows.h>

static std::vector<char> m_data{};

// Get current module (EXE) in binary data.
LPVOID N_GetBinaryModule(SIZE_T *psize)
{
    if (!m_data.size())
    {
        // Get module path.
        WCHAR path[2048];
        GetModuleFileNameW(NULL, path, 2048);
        
        FILE *file = _wfopen(path, L"rb");

        fseek(file, 0L, SEEK_END);
        size_t fileSize = ftell(file);
        rewind(file);

        // Read and cache it.
        m_data.resize(fileSize);
        fread(m_data.data(), sizeof(char), fileSize, file);

        fclose(file);
    }

    *psize = m_data.size();
    return (LPVOID)m_data.data();
}