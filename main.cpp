#include <iostream>
#include <utility>
#include <vector>
#include <cstdint>
#include <cmath>


enum class OrderType
{
    GoodTillCancel,
    FillAndKill
};

enum class Side
{
    Buy,
    Sell
};

using Price = std::double_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;
struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};
using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{

public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
    : bids_{bids}, asks_{asks} {}

    [[nodiscard]] const LevelInfos &getBids() const { return bids_;}
    [[nodiscard]] const LevelInfos &getAsks() const {return asks_;}

    void setBids(const LevelInfos &bids) { bids_ = bids;}
    void setAsks(const LevelInfos &asks) { asks_ = asks;}

private:
    LevelInfos bids_;
    LevelInfos asks_;

};


class Order
{

public:

    Order (OrderId orderId, Side side, OrderType orderType, Price price, Quantity quantity) :
            orderId_{orderId},
            side_{side},
            orderType_{orderType},
            price_{price},
            originalQuantity{quantity},
            remainingQuantity{quantity}{}


private:

    OrderId orderId_;
    Side side_;
    OrderType orderType_;
    Price price_;
    Quantity originalQuantity;
    Quantity remainingQuantity;




};

int main() {


    LevelInfo l1{23.40, 20};
    LevelInfo l2{23.43, 4};
    LevelInfo l3{23.44, 10};

    LevelInfo a1{23.40, 20};
    LevelInfo a2{23.43, 4};
    LevelInfo a3{23.44, 10};

    LevelInfos bids = {l1, l2, l3};
    LevelInfos asks = {a1, a2, a3};


    OrderbookLevelInfos ob{bids, asks};

    std::cout << ob.getBids().size() << std::endl;

    return 0;
}
