#pragma once

// What kind of order it is
enum class OrderType {
    MARKET,
    LIMIT
};

// How long the order should remain active
enum class TimeInForce {
    FILL_OR_KILL,       // All-or-nothing, immediate
    IMMEDIATE_OR_CANCEL, // Partial fills allowed, rest canceled
    GOOD_TILL_CANCEL    // Stays in book until filled or canceled
};