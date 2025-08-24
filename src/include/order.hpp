#pragma once
#include <stdexcept>
#include <cstdint>
#include "side.hpp"
#include "type.hpp"

using OrderID  = std::uint64_t;
using Price    = std::double_t;
using Quantity = std::int64_t;

class Order {
public:
    static Order create(Side side, OrderType type, TimeInForce timeInForce, Price price, Quantity qty) {
        Order o(side, type, timeInForce, price, qty);  // ctor runs
        o.validate();                          // throws on bad input
        return o;                              // o already has a unique _orderId
    }

    OrderID     getOrderId()         const { return _orderId; }
    Side        getSide()            const { return _side; }
    OrderType   getOrderType()       const { return _orderType; }
    TimeInForce getTimeInForce()     const { return _timeInForce; }
    Price       getPrice()           const { return _price; }
    Quantity    getOriginalQuantity()const { return originalQuantity; }
    Quantity    getRemainingQuantity()const { return remainingQuantity; }

    void reduceRemainingQuantity(const Quantity& take) {
        remainingQuantity -= take;
    }

private:
    Order(Side side, OrderType type, TimeInForce tif, Price price, Quantity qty)
            : _side(side), _orderType(type), _timeInForce(tif),
              _price(price), originalQuantity(qty), remainingQuantity(qty)
    {
        _orderId = ++_nextOrderId;  // <-- assign unique id here
    }

    void validate() {
        if (_price <= 0)
            throw std::invalid_argument("Limit order price must be > 0");
        if (originalQuantity == 0)
            throw std::invalid_argument("Order quantity must be > 0");

        if (_orderType == OrderType::MARKET) {
            if (_timeInForce != TimeInForce::FILL_OR_KILL &&
                _timeInForce != TimeInForce::IMMEDIATE_OR_CANCEL)
                throw std::invalid_argument("Market TIF must be FOK or IOC");
        } else { // LIMIT
            if (_timeInForce != TimeInForce::FILL_OR_KILL &&
                _timeInForce != TimeInForce::GOOD_TILL_CANCEL)
                throw std::invalid_argument("Limit TIF must be FOK or GTC");
        }
    }

    OrderID     _orderId{0};
    Side        _side;
    OrderType   _orderType;
    TimeInForce _timeInForce;
    Price       _price{};
    Quantity    originalQuantity;
    Quantity    remainingQuantity;

    inline static OrderID _nextOrderId = 0;
};