/**
 * @file testUtils.h
 * @author Konrad Zemek
 * @copyright (C) 2014 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#ifndef VEILHELPERS_TEST_UTILS_H
#define VEILHELPERS_TEST_UTILS_H


#include <algorithm>
#include <functional>
#include <random>
#include <string>

namespace
{

thread_local std::random_device rd;
thread_local std::mt19937 gen{rd()};

int randomInt(const int lower = 1, const int upper = 100)
{
    std::uniform_int_distribution<int> dis{lower, upper};
    return dis(gen);
}

std::string randomString(const unsigned int length)
{
    static thread_local std::uniform_int_distribution<char> dis{'a', 'z'};
    std::string result;
    std::generate_n(std::back_inserter(result), length, std::bind(dis, gen));
    return std::move(result);
}

std::string randomString()
{
    return randomString(randomInt());
}

}


#endif // VEILHELPERS_TEST_UTILS_H
