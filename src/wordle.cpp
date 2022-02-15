#include <wordle/wordle.hpp>

#include <cctype>

namespace wordle {

Wordle::Wordle(const std::string_view& word) noexcept
{
    std::transform(word.begin(), word.end(), std::back_inserter(word_), [](const char c)->char {
        return std::tolower(c) - 'a';
    });

    for (const auto& c : word_) {
        ++counts_[c];
    }
}

std::ostream& operator<<(std::ostream& os, const Wordle& wordle) {
    for (const auto& c : wordle.word_) {
        os << static_cast<char>(c + 'a');
    }
    return os;
}

} // namespace wordle