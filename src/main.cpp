#include <iostream>
#include <format>
#include "include/order.hpp"
#include "include/choice.hpp"
#include "include/orderbook.hpp"

using Day = int32_t;

int main() {

    Orderbook orderbook;
    orderbook.populateOrderbook(100);

    Portfolio portfolio;
    portfolio.setBalance(10000);

    bool keep_trading = true;
    int choice{};
    Day day = 1;

    while (keep_trading) {

        std::cout << std::format("\nDay # {}\n"
                                 "What would you like to do?\n"
                                 "1. Market Buy/Sell\n"
                                 "2. Limit Buy/Sell\n"
                                 "3. Simulate Day\n"
                                 "4. Display Orderbook\n"
                                 "5. Exit\n"
                                 "Enter choice: ",
                                 day);

        std::cin >> choice;

        switch (choice) {
            case Choice::MARKET:
                orderbook.executeMarketOrder(portfolio);
                break;
            case Choice::LIMIT:
                orderbook.executeLimitOrder();
                break;
            case Choice::SIMULATE_DAY:
                orderbook.simulateNextDay(orderbook.getTodaysPrice());
                day++;
                break;
            case Choice::DISPLAY_ORDERBOOK:
                orderbook.displayOrderbook();
                break;
            case Choice::EXIT:
                keep_trading = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    return 0;
}
