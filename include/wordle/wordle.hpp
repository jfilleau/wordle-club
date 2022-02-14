#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <string_view>

namespace wordle {

class Wordle {
public:
    friend class Filter;

    Wordle(const std::string_view& word) noexcept;

    int count(char c) const {
        return counts_[c - 'a'];
    }

    const char& operator[](std::size_t index) const {
        return word_[index] + 'a';
    }

    friend std::ostream& operator<<(std::ostream& os, const Wordle& wordle);

    friend bool operator<(const Wordle& lhs, const Wordle& rhs);

private:
    std::string word_{};
    std::array<char, 26> counts_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

inline bool operator<(const Wordle& lhs, const Wordle& rhs) {
    return lhs.word_ < rhs.word_;
}

} // namespace wordle