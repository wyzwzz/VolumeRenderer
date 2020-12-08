//
// Created by wyz on 20-12-8.
//

#ifndef VOLUMERENDERER_UTIL_H
#define VOLUMERENDERER_UTIL_H

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
namespace sv {

    inline auto convert_int16_to_uint8(const int16_t *source, size_t size,
                                       float max, float min)
    -> std::vector<uint8_t> {
        std::vector<uint8_t> data;
        data.reserve(size);

        double max_range = static_cast<double>(max) - static_cast<double>(min);
        uint8_t max_value = 255;
        for (size_t i = 0; i < size; i++) {
            double range = static_cast<double>(source[i]) - static_cast<double>(min);
            auto value = round(range / max_range * max_value);
            data.push_back(value);
        }

        return data;
    }

    inline auto convert_int16_to_uint8(const int16_t *source, size_t size,
                                       std::array<float, 2> &range)
    -> std::vector<uint8_t> {
        auto min = source[0];
        auto max = source[0];
        for (size_t i = 0; i < size; i++) {
            auto value = source[i];
            if (value > max) {
                max = value;
            } else if (value < min) {
                min = value;
            }
        }
        range[0] = min;
        range[1] = max;
        return convert_int16_to_uint8(source, size, max, min);
    }

    inline auto convert_float_to_uint8(const float *source, size_t size, float max,
                                       float min) -> std::vector<uint8_t> {
        std::vector<uint8_t> data;
        data.reserve(size);

        double max_range = max - min;
        uint8_t max_value = 255;
        for (size_t i = 0; i < size; i++) {
            double range = source[i] - min;
            auto value = round(range / max_range * max_value);
            data.push_back(value);
        }

        return data;
    }

    inline auto convert_float_to_uint8(const float *source, size_t size,
                                       std::array<float, 2> &range)
    -> std::vector<uint8_t> {
        auto min = source[0];
        auto max = source[0];
        for (size_t i = 0; i < size; i++) {
            auto value = source[i];
            if (value > max) {
                max = value;
            } else if (value < min) {
                min = value;
            }
        }
        range[0] = min;
        range[1] = max;
        return convert_float_to_uint8(source, size, max, min);
    }

    inline auto get_file_extension(const std::string &filepath) -> std::string {
        auto ext_idx = filepath.find_last_of('.');
        if (ext_idx == std::string::npos) {
            return "";
        }

        return filepath.substr(ext_idx);
    }

    inline auto get_file_name(const std::string &filepath) -> std::string {
        size_t begin = 0;
        size_t end = filepath.size();

        auto ext_idx = filepath.find_last_of('.');
        if (ext_idx != std::string::npos) {
            end = ext_idx;
        }
        auto slash_idx = filepath.find_last_of('/');
        if (slash_idx != std::string::npos) {
            begin = slash_idx + 1;
        }

        return filepath.substr(begin, end - begin);
    }

    inline auto split(const std::string &s, const std::string &delimiter = " ")
    -> std::vector<std::string> {
        std::vector<std::string> tokens;
        auto last_pos = s.find_first_not_of(delimiter, 0);
        auto pos = s.find_first_of(delimiter, last_pos);
        while (pos != std::string::npos || last_pos != std::string::npos) {
            tokens.push_back(s.substr(last_pos, pos - last_pos));
            last_pos = s.find_first_not_of(delimiter, pos);
            pos = s.find_first_of(delimiter, last_pos);
        }
        return tokens;
    }

}

#endif //VOLUMERENDERER_UTIL_H
