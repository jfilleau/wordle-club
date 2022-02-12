#pragma once

#include <cstddef>

namespace wordle {

template <std::size_t Positions>
struct DistributedLetter {
    std::array<bool, Positons> possible_positions
};

} // namespace wordle