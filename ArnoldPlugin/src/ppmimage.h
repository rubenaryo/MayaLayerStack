#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <windows.h>
#include <filesystem>


std::filesystem::path getDllDirectory()
{
    HMODULE hModule = nullptr;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&getDllDirectory),
        &hModule);

    char path[MAX_PATH];
    GetModuleFileNameA(hModule, path, MAX_PATH);

    return std::filesystem::path(path).parent_path();
}

struct Pixel {
    uint8_t r, g, b;
};

class PPMImage {
public:
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels; // �� RGBRGBRGB... �洢

    PPMImage(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open PPM file: " + path);
        }

        std::string magic;
        file >> magic;
        if (magic != "P6") {
            throw std::runtime_error("Unsupported PPM format (only P6 supported)");
        }

        // ����ע����
        char ch;
        file.get(ch);
        while (ch == '#') {
            std::string comment;
            std::getline(file, comment);
            file.get(ch);
        }
        file.unget(); // �Ż�ȥ

        int maxVal = 0;
        file >> width >> height >> maxVal;

        if (maxVal != 255) {
            throw std::runtime_error("Only maxVal=255 is supported");
        }

        file.get(); // �Ե�һ���հ׷���ͨ���ǻ��У�

        pixels.resize(width * height * 3);
        file.read(reinterpret_cast<char*>(pixels.data()), pixels.size());

        if (!file) {
            throw std::runtime_error("Failed to read pixel data");
        }
    }

    // ��uv���ʣ�u,v �� [0,1]
    // ע�⣺u�����ң�v���ϵ���
    Pixel getPixel(float u, float v) const {
        Pixel result;
        if (pixels.empty()) return result;

        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        int x = static_cast<int>(u * (width - 1));
        int y = static_cast<int>(v * (height - 1));

        // ע�⣬PPM������ (0,0) �����Ͻ�
        int index = (y * width + x) * 3;
        result.r = pixels[index + 0];
        result.g = pixels[index + 1];
        result.b = pixels[index + 2];
        return result;
    }
};
