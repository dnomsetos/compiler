#pragma once

#include <cstdint>

enum class Status : std::uint8_t {
  Undefined = 0,
  Live = 1,
  SharedBorrowed = 2,
  MutBorrowed = 3,
  UnknownBorowed = 4,
  Dead = 5,
};

namespace {

constexpr Status undef = Status::Undefined;
constexpr Status live = Status::Live;
constexpr Status sb = Status::SharedBorrowed;
constexpr Status mb = Status::MutBorrowed;
constexpr Status ub = Status::UnknownBorowed;
constexpr Status dead = Status::Dead;

constexpr bool leq_table[6][6] = {
    // Undef  Live   SB     MB     UB     Dead
    {  true,  true,  true,  true,  true,  true  }, // Undef
    {  false, true,  true,  true,  true,  true  }, // Live
    {  false, false, true,  false, true,  true  }, // SB
    {  false, false, false, true,  true,  true  }, // MB
    {  false, false, false, false, true,  true  }, // UB
    {  false, false, false, false, false, true  }, // Dead
};

/*
partially ordered set:

         undef
        /    \
       /      \
      /        live
     /        /   \
    /        V     V
   /        SB     MB
  /          \     /
 /            \   /
 |             V V
 |             UB
 \            /
  \          /
   \        /
     V    V
      dead
*/

constexpr Status meet_table[6][6] = {
    // Undef  Live   SB     MB     UB     Dead
    {  undef, undef, undef, undef, undef, undef  }, // Undef
    {  undef, live,  live,  live,  live,  live   }, // Live
    {  undef, live,  sb,    live,  sb,    sb     }, // SB
    {  undef, live,  live,  mb,    mb,    mb     }, // MB
    {  undef, live,  sb,    mb,    ub,    ub     }, // UB
    {  undef, live,  sb,    mb,    ub,    dead   }, // Dead
};

constexpr Status join_table[6][6] = {
    // Undef  Live   SB     MB     UB     Dead
    {  undef, live,  sb,    mb,    ub,    dead   }, // Undef
    {  live,  live,  sb,    mb,    ub,    dead   }, // Live
    {  sb,    sb,    sb,    ub,    ub,    dead   }, // SB
    {  mb,    mb,    ub,    mb,    ub,    dead   }, // MB
    {  ub,    ub,    ub,    ub,    ub,    dead   }, // UB
    {  dead,  dead,  dead,  dead,  dead,  dead   }, // Dead
};

} // namespace

constexpr bool operator<=(Status a, Status b) {
  return leq_table[static_cast<std::uint8_t>(a)][static_cast<std::uint8_t>(b)];
}

constexpr Status meet(Status a, Status b) {
  return meet_table[static_cast<std::uint8_t>(a)][static_cast<std::uint8_t>(b)];
}

constexpr Status join(Status a, Status b) {
  return join_table[static_cast<std::uint8_t>(a)][static_cast<std::uint8_t>(b)];
}

