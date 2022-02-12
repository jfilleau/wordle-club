#ifndef WORDLE_LETTER_H
#define WORDLE_LETTER_H

#include <cstddef>
#include <unordered_set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <numeric>

namespace wordle {

class Letter {
public:
    Letter() noexcept
        : Letter(all_letters.begin(), all_letters.end())
    { }

    template <typename InputIt>
    Letter(InputIt first, InputIt last) noexcept
        : _possible(first, last)
    { }

    Letter(char c) noexcept 
        : Letter(&c, (&c) + 1)
    { }

    Letter(const Letter& other) = default;

    Letter& operator=(const Letter& other) = default;

    Letter(Letter&& other) noexcept
        : _possible(std::move(other._possible))
    { }

    Letter& operator=(Letter&& other) noexcept {
        if (this != &other) {
            _possible = std::move(other._possible);
        }

        return *this;
    }

    bool may_satisfy(char c) const {
        return _possible.find(c) != _possible.end();
    }

    friend Letter operator-(const Letter& lhs, const Letter& rhs);

    Letter& operator-=(const Letter& other) {
        for (const auto& c : other._possible) {
            if (auto it = _possible.find(c); it != _possible.end()) {
                _possible.erase(it);
            }
        }

        return *this;
    }

    bool operator==(const Letter& other) const {
        return _possible == other._possible;
    }

private:
    std::unordered_set<char> _possible;
};

Letter operator-(const Letter& lhs, const Letter& rhs) {
    Letter letter = lhs;
    return (letter -= rhs);
}

} // namespace wordle

#endif // WORDLE_LETTER_H