/**
 * \file   test_VariableName.cc
 * \author Lars Froehlich
 * \date   Created on January 6, 2022
 * \brief  Test suite for the VariableName class.
 *
 * \copyright Copyright 2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gul14/catch.h>
#include "taskomat/Error.h"
#include "taskomat/VariableName.h"

using namespace std::literals;
using namespace task;

TEST_CASE("VariableName: VariableName(const char*)", "[VariableName]")
{
    REQUIRE_THROWS_AS(VariableName(nullptr), Error);
    REQUIRE_THROWS_AS(VariableName(""), Error);
    REQUIRE_THROWS_AS(VariableName("_a"), Error);
    REQUIRE_THROWS_AS(VariableName("1a"), Error);
    REQUIRE_THROWS_AS(VariableName("42"), Error);
    REQUIRE_THROWS_AS(VariableName("a12345678901234567890123456789012345678901234567890"
                                   "12345678901234567890"), Error);
    REQUIRE_THROWS_AS(VariableName("a c"), Error);
    REQUIRE_THROWS_AS(VariableName("a\tc"), Error);
    REQUIRE_THROWS_AS(VariableName("a-c"), Error);
    REQUIRE_THROWS_AS(VariableName("a+c"), Error);

    // All of the following possible because of implicit constructor from const char*
    VariableName names[] = { "a", "b52", "fortytwo", "snake_case", "CamelCase",
        "dromedaryCase", "a_very_long_but_perfectly_acceptable_variable_name" };

    REQUIRE(names[0] == "a");
    REQUIRE(names[1] == "b52");
    REQUIRE(names[2] == "fortytwo");
}

TEST_CASE("VariableName: VariableName(const std::string&)", "[VariableName]")
{
    const std::string bad_names[] = { "", "_a", "1a", "42",
        "a1234567890123456789012345678901234567890123456789012345678901234567890",
        "a c", "a\tc", "a-c", "a+c", "a\0b"s
    };

    for (const std::string& str : bad_names)
        REQUIRE_THROWS_AS(VariableName(str), Error);

    const std::string good_names[] = {"a", "b52", "fortytwo", "snake_case", "CamelCase",
        "dromedaryCase", "a_very_long_but_perfectly_acceptable_variable_name" };

    for (const std::string& str : good_names)
        REQUIRE_NOTHROW(VariableName(str));
}

TEST_CASE("VariableName: VariableName(std::string&&)", "[VariableName]")
{
    std::string bad_names[] = { "", "_a", "1a", "42",
        "a1234567890123456789012345678901234567890123456789012345678901234567890",
        "a c", "a\tc", "a-c", "a+c", "a\0b"s
    };

    for (const std::string& str : bad_names)
        REQUIRE_THROWS_AS(VariableName(std::move(str)), Error);

    std::string good_names[] = {"a", "b52", "fortytwo", "snake_case", "CamelCase",
        "dromedaryCase", "a_very_long_but_perfectly_acceptable_variable_name" };

    for (const std::string& str : good_names)
        REQUIRE_NOTHROW(VariableName(std::move(str)));
}

TEST_CASE("VariableName: operator+=(std::string, VariableName)")
{
    std::string str{ "Hello" };
    const VariableName var{ "World" };

    str += var;
    REQUIRE(str == "HelloWorld");
}

TEST_CASE("VariableName: operator+(VariableName, string_view)")
{
    std::string str{ "String" };
    const VariableName var{ "Var" };

    REQUIRE(var + "Cstring" == "VarCstring");
    REQUIRE(var + str == "VarString");
}

TEST_CASE("VariableName: operator+(string_view, VariableName)")
{
    std::string str{ "String" };
    const VariableName var{ "Var" };

    REQUIRE("Cstring" + var == "CstringVar");
    REQUIRE(str + var == "StringVar");
}
