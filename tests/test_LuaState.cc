/**
 * \file   test_LuaState.cc
 * \author Lars Froehlich
 * \date   Created on December 3, 2021
 * \brief  Test suite for the LuaState class.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <type_traits>
#include <gul14/catch.h>
#include <hlc/util/exceptions.h>
#include <lua.h>
#include "../include/avtomat/LuaState.h"

using namespace avto;

TEST_CASE("LuaState: Default constructor", "[LuaState]")
{
    static_assert(std::is_default_constructible<LuaState>::value,
        "LuaState is_default_constructible");

    LuaState state;
}

TEST_CASE("LuaState: Move constructor", "[LuaState]")
{
    LuaState state;

    LuaState state2{ std::move(state) };
    REQUIRE(state.get() == nullptr);
    REQUIRE(state2.get() != nullptr);
}

TEST_CASE("LuaState: close()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get() != nullptr);

    state.close();
    REQUIRE(state.get() == nullptr);

    state.close();
    REQUIRE(state.get() == nullptr);
}

TEST_CASE("LuaState: create_table()", "[LuaState]")
{
    LuaState state;

    state.create_table();
    REQUIRE(lua_gettop(state.get()) == 1); // 1 object on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(0, 10);
    REQUIRE(lua_gettop(state.get()) == 2); // 2 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(10, 0);
    REQUIRE(lua_gettop(state.get()) == 3); // 3 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(10, 10);
    REQUIRE(lua_gettop(state.get()) == 4); // 4 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    REQUIRE_THROWS_AS(state.create_table(-1, 0), hlc::Error);
    REQUIRE(lua_gettop(state.get()) == 4); // 4 objects on stack

    REQUIRE_THROWS_AS(state.create_table(0, -2), hlc::Error);
    REQUIRE(lua_gettop(state.get()) == 4); // 4 objects on stack

    REQUIRE_THROWS_AS(state.create_table(-10, -1), hlc::Error);
    REQUIRE(lua_gettop(state.get()) == 4); // 4 objects on stack

    state.close();
    REQUIRE_THROWS_AS(state.create_table(), hlc::Error);
}

TEST_CASE("LuaState: get()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get() != nullptr);

    state.close();
    REQUIRE(state.get() == nullptr);
}

TEST_CASE("LuaState: get_global()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get_global("pippo") == LUA_TNIL);

    state.push_number(42.0);
    state.set_global("pippo");

    REQUIRE(state.get_global("pippo") == LUA_TNUMBER);
}

TEST_CASE("LuaState: load_string()", "[LuaState]")
{
    LuaState state;

    SECTION("Valid LUA strings")
    {
        state.load_string("");
        state.load_string("local a = 2");
    }

    SECTION("Syntax error")
    {
        REQUIRE_THROWS_AS(state.load_string("locally a = 2"), hlc::Error);
    }

    SECTION("Exception thrown when called on closed state")
    {
        state.close();
        REQUIRE_THROWS_AS(state.load_string(""), hlc::Error);
    }
}

TEST_CASE("LuaState: operator=(LuaState&&) (move assignment)", "[LuaState]")
{
    LuaState state1;
    auto* state1_ptr = state1.get();
    REQUIRE(state1_ptr != nullptr);

    LuaState state2;
    auto* state2_ptr = state2.get();
    REQUIRE(state2_ptr != nullptr);

    state2 = std::move(state1);
    REQUIRE(state1.get() == nullptr);
    REQUIRE(state2.get() == state1_ptr);
}

TEST_CASE("LuaState: pop_number()", "[LuaState]")
{
    LuaState state;

    auto initial_stack_pos = lua_gettop(state.get());
    lua_pushnumber(state.get(), 42.0);

    SECTION("Number can be retrieved and stack position is adjusted")
    {
        REQUIRE(state.pop_number() == 42.0);
        REQUIRE(lua_gettop(state.get()) == initial_stack_pos);
    }

    SECTION("Exception thrown when called on closed state")
    {
        state.close();
        REQUIRE_THROWS_AS(state.pop_number(), hlc::Error);
    }

    SECTION("Exception thrown when there is nothing to pop")
    {
        state.pop_number();
        REQUIRE_THROWS_AS(state.pop_number(), hlc::Error);
    }
}

TEST_CASE("LuaState: pop_string()", "[LuaState]")
{
    LuaState state;

    auto initial_stack_pos = lua_gettop(state.get());
    lua_pushstring(state.get(), "Test");

    SECTION("String can be retrieved and stack position is adjusted")
    {
        REQUIRE(state.pop_string() == "Test");
        REQUIRE(lua_gettop(state.get()) == initial_stack_pos);
    }

    SECTION("Exception thrown when called on closed state")
    {
        state.close();
        REQUIRE_THROWS_AS(state.pop_string(), hlc::Error);
    }

    SECTION("Exception thrown when there is nothing to pop")
    {
        state.pop_string();
        REQUIRE_THROWS_AS(state.pop_string(), hlc::Error);
    }
}

TEST_CASE("LuaState: push_number()", "[LuaState]")
{
    LuaState state;

    auto initial_stack_pos = lua_gettop(state.get());

    state.push_number(42.0);
    REQUIRE(lua_gettop(state.get()) == initial_stack_pos + 1);
    REQUIRE(state.pop_number() == 42.0);
}

TEST_CASE("LuaState: set_global()", "[LuaState]")
{
    LuaState state;

    state.push_number(42.0);
    state.set_global("pippo");

    REQUIRE(state.get_global("pippo") == LUA_TNUMBER);
}

TEST_CASE("LuaState: set_table()", "[LuaState]")
{
    LuaState state;

    SECTION("Store value 42 at table index 1")
    {
        state.create_table();
        state.push_number(1);
        state.push_number(42.0);
        state.set_table(-3);
        REQUIRE(lua_gettop(state.get()) == 1); // 1 object on stack (just the table)
        REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

        state.push_number(1); // index to retrieve
        lua_gettable(state.get(), -2);
        REQUIRE(lua_gettop(state.get()) == 2); // 2 objects on stack (table + result)
        REQUIRE(state.pop_number() == 42.0);
    }
}
