#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, size_t limit) {
    std::vector<std::string> result;
    std::istringstream stream(str);
    std::string word;

    // Split up to 'limit' parts
    while (stream >> word && result.size() < limit) {
        result.push_back(word);
    }

    // If there are more words and we have hit the limit, concatenate the rest
    if (stream >> word) {
        std::string remaining;
        remaining += word;
        while (stream >> word) {
            remaining += " " + word;
        }
        result.push_back(remaining);
    }

    return result;
}
