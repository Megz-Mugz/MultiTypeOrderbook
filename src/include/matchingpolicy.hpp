#include "order.hpp"

struct MatchingPolicy {
    bool require_full_immediate_fill;

    bool allow_partial_immediate_execution;

    bool rest_unfilled_remainder_on_book;

    void to_string() {
        std::cout << "MatchingPolicy:\n"
                  << "  require_full_immediate_fill = "
                  << (require_full_immediate_fill ? "true" : "false") << "\n"
                  << "  allow_partial_immediate_execution = "
                  << (allow_partial_immediate_execution ? "true" : "false") << "\n"
                  << "  rest_unfilled_remainder_on_book = "
                  << (rest_unfilled_remainder_on_book ? "true" : "false") << "\n";
    }


};

// 2) Map (OrderType, TimeInForce) -> policy (constexpr function)
constexpr MatchingPolicy policyFor(OrderType order_type, TimeInForce time_in_force) {
    switch (order_type) {
        case OrderType::MARKET: {
            switch (time_in_force) {
                case TimeInForce::FILL_OR_KILL:
                    // Market + Fill Or Kill:
                    // Must fill entirely now; no partials; nothing rests.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/true,
                            /*allow_partial_immediate_execution=*/false,
                            /*rest_unfilled_remainder_on_book=*/false
                    };
                case TimeInForce::IMMEDIATE_OR_CANCEL:
                    // Market + Immediate Or Cancel:
                    // Fill whatever is available now; cancel the rest.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/false,
                            /*allow_partial_immediate_execution=*/true,
                            /*rest_unfilled_remainder_on_book=*/false
                    };
                default:
                    // Sensible default for unexpected Market TIF:
                    // allow partial immediate execution; do not rest remainder.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/false,
                            /*allow_partial_immediate_execution=*/true,
                            /*rest_unfilled_remainder_on_book=*/false
                    };
            }
        }

        case OrderType::LIMIT: {
            switch (time_in_force) {
                case TimeInForce::FILL_OR_KILL:
                    // Limit + Fill Or Kill:
                    // Must fill entirely now at acceptable prices; no partials; nothing rests.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/true,
                            /*allow_partial_immediate_execution=*/false,
                            /*rest_unfilled_remainder_on_book=*/false
                    };
                case TimeInForce::GOOD_TILL_CANCEL:
                    // Limit + Good Till Cancel:
                    // Fill whatever is available now; any remainder rests on the book.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/false,
                            /*allow_partial_immediate_execution=*/true,
                            /*rest_unfilled_remainder_on_book=*/true
                    };
                default:
                    // Sensible default for unexpected Limit TIF:
                    // allow partial immediate execution; do not rest remainder.
                    return MatchingPolicy{
                            /*require_full_immediate_fill=*/false,
                            /*allow_partial_immediate_execution=*/true,
                            /*rest_unfilled_remainder_on_book=*/false
                    };
            }
        }
    }

    // Global fallback if new enum values appear:
    return MatchingPolicy{
            /*require_full_immediate_fill=*/false,
            /*allow_partial_immediate_execution=*/true,
            /*rest_unfilled_remainder_on_book=*/false
    };
}