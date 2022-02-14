#include <wordle/filter.hpp>
#include <wordle/wordle.hpp>

#include <iostream>
#include <sstream>

int main() {
    int test_failed = 0;

    {
        std::string_view test_word = "berry";
        wordle::Wordle w(test_word);
        std::cout << "Test word is " << w << std::endl;
        std::stringstream ss;
        ss << w;
        if (ss.str() != test_word) {
            test_failed = 1;
            std::cerr << "Test failed: " << w << " == " << test_word << std::endl;
        }
    }

    {
        std::string_view test_word = "AdElE";
        wordle::Wordle w(test_word);
        std::cout << "Test word is " << w << std::endl;
        std::stringstream ss;
        ss << w;
        if (ss.str() != "adele") {
            test_failed = 2;
            std::cerr << "Test failed: " << w << " == " << "adele" << std::endl;
        }
    }

    {
        std::string_view test_word = "berry";
        wordle::Wordle w(test_word);
        using FB = wordle::GuessFeedback;
        wordle::FeedbackArray feedback = {FB::green, FB::yellow, FB::black, FB::green, FB::black};
        wordle::Filter filter(w, feedback);
        std::cout << "Result of feedback " << feedback << " applied to wordle " << w << ":\n";
        std::cout << filter << std::endl;
    }

    return 0;
}