#include <wordle/filter.hpp>

#include <iomanip>

namespace wordle {

Filter::Filter(const Wordle& wordle, const FeedbackArray& feedback) noexcept
{
    for (int i = 0; i < wordle.word_.size(); ++i) {
        const char& c = wordle.word_[i];
        const auto& f = feedback[i];
        auto& row = possible_chars_[i];

        switch (f) {
            case GuessFeedback::not_present:
                row.fill(true);
                row[c] = false;
                min_is_max_[c] = true;
                break;
            case GuessFeedback::present_but_wrong_spot:
                row.fill(true);
                row[c] = false;
                ++min_chars_[c];
                break;
            case GuessFeedback::present_and_correct_spot:
                row[c] = true;
                ++min_chars_[c];
                break;
        }
    }
}

bool Filter::contains(const Wordle& wordle) const {
    auto row = possible_chars_.begin();
    for (const auto& c : wordle.word_) {
        if (!(*row)[c]) {
            return false;
        }
        ++row;
    }

    auto min_it = min_chars_.begin();
    auto max_it = min_is_max_.begin();
    for (const auto& count : wordle.counts_) {
        if (*max_it) {
            if (count != *min_it) {
                return false;
            }
        }
        else if (count < *min_it) {
            return false;
        }
        ++max_it;
        ++min_it;
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const FeedbackArray& fb) {
    constexpr std::array<char, 3> feedback_strings{'B', 'Y', 'G'};
    for (const auto& f : fb) {
        os << feedback_strings[static_cast<std::underlying_type_t<GuessFeedback>>(f)];
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Filter& filter) {
    auto format_flags = os.flags();
    os << std::right << std::setfill(' ');

    os << "   ";
    for (char c = 'a'; c <= 'z'; ++c) {
        os << std::setw(2) << c;
    }
    os << '\n';

    for (int i = 0; i < filter.possible_chars_.size(); ++i) {
        const auto& row = filter.possible_chars_[i];
        os << std::setw(3) << i;
        for (const auto& cell : row) {
            os << std::setw(2) << (cell ? 'o' : '.');
        }
        os << '\n';
    }

    os << std::setw(3) << "min";
    for (const auto& cell : filter.min_chars_) {
        os << std::setw(2) << static_cast<int>(cell);
    }
    os << '\n';

    os << std::setw(3) << "max";
    for (const auto& cell : filter.min_is_max_) {
        os << std::setw(2) << (cell ? '^' : '.');
    }
    os << '\n';

    os.flags(format_flags);
    return os;
}

} // namespace wordle