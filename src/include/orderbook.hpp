#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include "portfolio.hpp"

#pragma once
#include "order.hpp"
#include "matchingpolicy.hpp"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> qty_dist(10, 500);
std::uniform_real_distribution<double> micro_pct(0.0005, 0.005);

using Bids = std::vector<Order>;
using Asks = std::vector<Order>;

class Orderbook {

public:

    void populateOrderbook(const Price& previousDayPrice){
        // today's price will be 1-3% in either direction of the previous day's price
        int percentage = 1 + rand() % 3;
        int upOrDown = rand() % 2; // 0 = down, 1 = up
        double changeAmount = previousDayPrice * (static_cast<double>(percentage) / 100.0);

        // Compute today's price accordingly
        Price todaysPrice = previousDayPrice;
        if (upOrDown == 0) {
            todaysPrice -= changeAmount;
        } else {
            todaysPrice += changeAmount;
        }
        _todaysPrice = todaysPrice;
        std::cout << "Today's Price: " << todaysPrice << std::endl;

        clearOrderbook();

       // To keep things simple, generate 5 bids & asks on each new day
       for (int i = 0; i < 5; ++i) {
           double level = static_cast<double>(i + 1);

           double bidOffset = level * micro_pct(gen);
           Price bidPrice = todaysPrice * (1.0 - bidOffset);
           if (bidPrice <= 0) bidPrice = std::max<Price>(0.01, todaysPrice * 0.5);
           Quantity bidQty = qty_dist(gen);
           auto bid = Order::create(Side::BUY, OrderType::LIMIT, TimeInForce::GOOD_TILL_CANCEL, bidPrice, bidQty);
           _bids.push_back(bid);

           double askOffset = level * micro_pct(gen);
           Price askPrice = todaysPrice * (1.0 + askOffset);
           if (askPrice <= 0) askPrice = std::max<Price>(0.01, todaysPrice * 1.5);
           Quantity askQty = qty_dist(gen);
           auto ask = Order::create(Side::SELL, OrderType::LIMIT, TimeInForce::GOOD_TILL_CANCEL, askPrice, askQty);
           _asks.push_back(ask);
       }

    }

    void displayOrderbook(){
        std::sort(_bids.begin(), _bids.end(), [](const Order& a, const Order& b) {
            return a.getPrice() > b.getPrice();
        });

        std::sort(_asks.begin(), _asks.end(), [](const Order& a, const Order& b) {
            return a.getPrice() > b.getPrice();
        });

        std::cout << "Orderbook Ladder:" << std::endl;

        std::cout << " Asks (SELL):" << std::endl;
        for (const auto& ask : _asks) {
            std::cout << "  Price: " << ask.getPrice() << "  Qty: " << ask.getRemainingQuantity() << std::endl;
        }

        std::cout << " -- TODAY'S PRICE: " << _todaysPrice << " --" << std::endl;


        std::cout << " Bids (BUY):" << std::endl;
        for (const auto& bid : _bids) {
            std::cout << "  Price: " << bid.getPrice() << "  Qty: " << bid.getRemainingQuantity() << std::endl;
        }

        std::cout << "\n\n\n";
    }

    void simulateNextDay(const Price& previousDayPrice){
        populateOrderbook(previousDayPrice);
    }

    [[nodiscard]] const Bids &getBids() const { return _bids; }

    [[nodiscard]] const Asks &getAsks() const { return _asks; }

    [[nodiscard]] Price getTodaysPrice() const { return _todaysPrice; }

    void executeMarketOrder(const Portfolio& portfolio){

        if (_bids.empty() && _asks.empty()) {
            std::cout << "Orderbook is empty. Nothing to execute.\n";
            return;
        }

        // determine if user wants buy or sell
        char buyOrSell{};
        std::cout << "Do you want to buy or sell? (B=BUY, S=SELL): ";
        std::cin >> buyOrSell;
        buyOrSell = static_cast<char>(std::toupper(static_cast<unsigned char>(buyOrSell)));
        if (buyOrSell != 'B' && buyOrSell != 'S') {
            std::cout << "Invalid side.\n";
            return;
        }
        Side side = (buyOrSell == 'B') ? Side::BUY : Side::SELL;

        // inquiry about time restrictions (for MARKET orders, use FOK or IOC)
        int timeInForce{};
        std::cout << "Do you want\n"
                     "1. Fill or Kill\n"
                     "2. Immediate or Cancel\n";
        std::cin >> timeInForce;
        if (timeInForce != 1 && timeInForce != 2) {
            std::cout << "Invalid time-in-force choice.\n";
            return;
        }
        TimeInForce tif = (timeInForce == 1) ? TimeInForce::FILL_OR_KILL : TimeInForce::IMMEDIATE_OR_CANCEL;

        // pick a reference price for MARKET order from the best opposite side
        Price price{};
        if (side == Side::BUY) {
            if (_asks.empty()) { std::cout << "No asks available.\n"; return; }
            auto it = std::min_element(_asks.begin(), _asks.end(), [](const Order& a, const Order& b){
                return a.getPrice() < b.getPrice();
            });
            price = it->getPrice();
        } else {
            if (_bids.empty()) { std::cout << "No bids available.\n"; return; }
            auto it = std::max_element(_bids.begin(), _bids.end(), [](const Order& a, const Order& b){
                return a.getPrice() < b.getPrice();
            });
            price = it->getPrice();
        }

        // enter quantity
        Quantity quantity{};
        std::cout << "What quantity do you want: ";
        std::cin >> quantity;
        if (quantity <= 0) {
            std::cout << "Quantity must be > 0.\n";
            return;
        }

        // Based on data collected, generate the MARKET order
        Order order = Order::create(side, OrderType::MARKET, tif, price, quantity);
        try {
            std::cout << "Created order id " << order.getOrderId()
                      << " | " << ((side == Side::BUY) ? "BUY" : "SELL")
                      << " " << quantity
                      << " @ MKT $" << price
                      << " | Time In Force=" << ((tif == TimeInForce::FILL_OR_KILL) ? "Fill Or Kill" : "Immediate or Cancel")
                      << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Failed to create order: " << ex.what() << "\n";
        }

        matchingEngine(order);



    }

    void executeLimitOrder() {
        // Side
        char buyOrSell{};
        std::cout << "Limit order: Do you want to buy or sell? (B=BUY, S=SELL): ";
        std::cin >> buyOrSell;
        buyOrSell = static_cast<char>(std::toupper(static_cast<unsigned char>(buyOrSell)));
        if (buyOrSell != 'B' && buyOrSell != 'S') {
            std::cout << "Invalid side.\n";
            return;
        }
        Side side = (buyOrSell == 'B') ? Side::BUY : Side::SELL;

        // Time-in-force (for LIMIT: GTC or FOK)
        int tifChoice{};
        std::cout << "Time In Force\n"
                     "1. Good Till Cancel (GTC)\n"
                     "2. Fill or Kill (FOK)\n"
                     "Choose: ";
        std::cin >> tifChoice;
        if (tifChoice != 1 && tifChoice != 2) {
            std::cout << "Invalid TIF choice.\n";
            return;
        }
        TimeInForce tif = (tifChoice == 1) ? TimeInForce::GOOD_TILL_CANCEL
                                           : TimeInForce::FILL_OR_KILL;

        // Price
        Price price{};
        std::cout << "Limit Price: ";
        std::cin >> price;
        if (price <= 0.0) {
            std::cout << "Price must be > 0.\n";
            return;
        }

        // Quantity
        Quantity quantity{};
        std::cout << "Quantity: ";
        std::cin >> quantity;
        if (quantity <= 0) {
            std::cout << "Quantity must be > 0.\n";
            return;
        }

        // Create & route
        Order order = Order::create(side, OrderType::LIMIT, tif, price, quantity);
        std::cout << "Created LIMIT order id " << order.getOrderId()
                  << " | " << ((side == Side::BUY) ? "BUY" : "SELL")
                  << " " << quantity << " @ $" << price
                  << " | TIF=" << ((tif == TimeInForce::GOOD_TILL_CANCEL) ? "GTC" : "FOK") << "\n";

        matchingEngine(order);
    }


private:
    Bids _bids;
    Asks _asks;
    Price _todaysPrice;

    void clearOrderbook(){
        _bids.clear();
        _asks.clear();
    }

    // Helper: best-first sort & price acceptance
    static inline void sort_best_first(std::vector<Order>& book, Side takerSide) {
        if (takerSide == Side::BUY) {
            // BUY hits asks: best (lowest) ask first
            std::sort(book.begin(), book.end(),
                      [](const Order& a, const Order& b){ return a.getPrice() < b.getPrice(); });
        } else {
            // SELL hits bids: best (highest) bid first
            std::sort(book.begin(), book.end(),
                      [](const Order& a, const Order& b){ return a.getPrice() > b.getPrice(); });
        }
    }

    static inline bool price_is_acceptable(const Order& taker, const Order& resting) {
        if (taker.getOrderType() == OrderType::MARKET) return true;
        if (taker.getSide() == Side::BUY)  return resting.getPrice() <= taker.getPrice();
        else                                return resting.getPrice() >= taker.getPrice();
    }

    void matchingEngine(const Order& taker) {

        // 0) Validate allowed TIF combos
        const OrderType ot  = taker.getOrderType();
        const TimeInForce tif = taker.getTimeInForce();
        const bool tif_ok =
                (ot == OrderType::MARKET && (tif == TimeInForce::FILL_OR_KILL || tif == TimeInForce::IMMEDIATE_OR_CANCEL)) ||
                (ot == OrderType::LIMIT  && (tif == TimeInForce::FILL_OR_KILL || tif == TimeInForce::GOOD_TILL_CANCEL));
        if (!tif_ok) {
            std::cout << "Invalid TIF for this order type (per your rules). Canceled.\n";
            return;
        }

        // 1) Policy
        MatchingPolicy policy = policyFor(ot, tif);

        // 2) Choose opposite side of the book AND my side (for potential resting)
        std::vector<Order>& opposite = (taker.getSide() == Side::BUY) ? _asks : _bids;
        std::vector<Order>& myside   = (taker.getSide() == Side::BUY) ? _bids : _asks;

        // If no liquidity on the other side:
        if (opposite.empty()) {
            if (ot == OrderType::LIMIT && policy.rest_unfilled_remainder_on_book) {
                // Rest the entire taker order as-is on its own side
                myside.push_back(taker);
                // Keep ladder tidy (asks asc, bids desc)
                if (taker.getSide() == Side::BUY) {
                    std::sort(myside.begin(), myside.end(), [](const Order& a, const Order& b){ return a.getPrice() > b.getPrice(); }); // bids desc
                } else {
                    std::sort(myside.begin(), myside.end(), [](const Order& a, const Order& b){ return a.getPrice() < b.getPrice(); }); // asks asc
                }
                std::cout << "No liquidity. LIMIT+GTC order rested on book.\n";
            } else {
                std::cout << "No liquidity. Order canceled.\n";
            }
            return;
        }

        // Sort opposite best-first (asks asc, bids desc)
        sort_best_first(opposite, taker.getSide());

        Quantity want = taker.getOriginalQuantity();
        Quantity remaining = want;
        Quantity filled = 0;
        double   notional = 0.0;

        // 3) FOK pre-check: is there enough acceptable liquidity right now?
        if (policy.require_full_immediate_fill) {
            Quantity possible = 0;
            for (const auto& r : opposite) {
                if (!price_is_acceptable(taker, r)) break;
                possible += r.getRemainingQuantity();
                if (possible >= want) break;
            }
            if (possible < want) {
                std::cout << "FOK not fully fillable immediately. Canceled.\n";
                return;
            }
        }

        // 4) Execute against book (mutation: decrement resting orders)
        for (auto& r : opposite) {
            if (remaining == 0) break;
            if (!price_is_acceptable(taker, r)) break;

            Quantity avail = r.getRemainingQuantity();
            if (avail <= 0) continue;

            Quantity take = std::min(remaining, avail);

            // (Optional) Portfolio checks could go here (affordability / inventory)
            // e.g., if taker.getSide()==Side::BUY && !portfolio.canAfford(r.getPrice(), take)) { ...trim take... }

            filled    += take;
            notional  += r.getPrice() * static_cast<double>(take);
            remaining -= take;

            r.reduceRemainingQuantity(take);
        }

        const bool full_filled = (remaining == 0);

        // 5) Policy outcomes
        if (policy.require_full_immediate_fill && !full_filled) {
            std::cout << "FOK could not fill completely after check. Canceled.\n";
            // (Optional) If you had applied any portfolio changes, you'd rollback here.
            // Since we only mutated resting orders, there's nothing to 'unfill' on taker.
            // In a real engine you'd avoid needing rollback by pre-checking, as done above.
            return;
        }

        if (!full_filled && !policy.allow_partial_immediate_execution) {
            std::cout << "Partial fill not allowed by policy. Canceled remainder.\n";
            // Do not rest remainder for Market; this branch is mainly here for future policies.
            // For your rules, MARKET only has FOK/IOC and we already allowed partial for IOC.
        }

        // If LIMIT + GTC and remainder exists: rest the remainder on our side
        if (ot == OrderType::LIMIT && policy.rest_unfilled_remainder_on_book && remaining > 0) {
            Order rest = Order::create(
                    taker.getSide(),
                    OrderType::LIMIT,
                    TimeInForce::GOOD_TILL_CANCEL,
                    taker.getPrice(),
                    remaining
            );
            myside.push_back(rest);
            // Keep my side sorted too
            if (taker.getSide() == Side::BUY) {
                std::sort(myside.begin(), myside.end(), [](const Order& a, const Order& b){ return a.getPrice() > b.getPrice(); }); // bids desc
            } else {
                std::sort(myside.begin(), myside.end(), [](const Order& a, const Order& b){ return a.getPrice() < b.getPrice(); }); // asks asc
            }
        }

        // 6) Cleanup: remove fully filled resting orders
        opposite.erase(
                std::remove_if(opposite.begin(), opposite.end(),
                               [](const Order& r){ return r.getRemainingQuantity() == 0; }),
                opposite.end()
        );

        // 7) Report
        std::cout << (taker.getSide() == Side::BUY ? "BUY " : "SELL ")
                  << filled << " shares"
                  << (full_filled ? " (FULL)" : " (PARTIAL)")
                  << " @ VWAP $" << (filled ? notional / static_cast<double>(filled) : 0.0)
                  << " | Notional $" << notional
                  << ((ot == OrderType::LIMIT && policy.rest_unfilled_remainder_on_book && remaining > 0)
                      ? " | Remainder rested on book" : "")
                  << "\n";
    }


};