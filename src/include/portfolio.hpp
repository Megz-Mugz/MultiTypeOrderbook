
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include "order.hpp"

using Balance = int64_t;


class Portfolio{

public:

    [[nodiscard]] Balance getBalance() const { return _balance; }

    void setBalance(Balance balance) { _balance = balance; }



private:

    Balance _balance;

};