/* \file
* \brief The [Knuth-Morris-Pratt Algorithm](https://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm)
 * for finding a pattern within a piece of text with complexity O(n + m)
*
* 1. Preprocess pattern to identify any suffixes that are identical to
* prefixes. This tells us where to continue from if we get a mismatch between a
* character in our pattern and the text.
* 2. Step through the text one character at a time and compare it to a
* character in the pattern updating our location within the pattern if
* necessary
*/

#pragma once

#ifndef CHELPER_KMPMATCHER_H
#define CHELPER_KMPMATCHER_H

#include <iostream>
#include <string_view>
#include <vector>

namespace CHelper {

    /**
     * \see https://github.com/TheAlgorithms/C-Plus-Plus/blob/master/strings/knuth_morris_pratt.cpp
     */
    class KMPMatcher {
    private:
        std::u16string_view pattern;
        size_t *failure;

    public:
        /**
        * Generate the partial match table aka failure function for a pattern to search.
        * \param[in] pattern text for which to create the partial match table
        * \returns the partial match table as a vector array
        */
        explicit KMPMatcher(const std::u16string_view &pattern);

        KMPMatcher(const KMPMatcher &pattern) = delete;

        ~KMPMatcher();

        KMPMatcher &operator=(const KMPMatcher &kmpMatcher) = delete;

        /**
        * KMP algorithm to find a pattern in a text
        * \param[in] text text in which to search
        * \returns position if pattern was found
        * \returns `-1` if pattern was not found
        */
        size_t match(const std::u16string &text);
    };


}// namespace CHelper

#endif// CHELPER_KMPMATCHER_H
