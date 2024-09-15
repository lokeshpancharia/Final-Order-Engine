// OrderCache.cpp

#include "OrderCache.h"

void OrderCache::addOrder(Order order) {
    const std::string& orderId = order.orderId();

    // Check if order ID already exists
    if (orders_.find(orderId) != orders_.end()) {
        // Do not add duplicate order IDs
        return;
    }

    // Insert order into order map
    orders_.emplace(orderId, order);

    // Insert order ID into user's set
    user_orders_[order.user()].insert(orderId);

    // Insert order ID into security's set
    security_orders_[order.securityId()].insert(orderId);

    // Update per-company quantities
    updateQuantities(order, order.qty());
}

void OrderCache::cancelOrder(const std::string& orderId) {
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        // Order ID does not exist
        return;
    }

    Order& order = it->second;

    // Update per-company quantities
    updateQuantities(order, -static_cast<int>(order.qty()));

    // Remove order from user's set
    user_orders_[order.user()].erase(orderId);
    if (user_orders_[order.user()].empty()) {
        user_orders_.erase(order.user());
    }

    // Remove order from security's set
    security_orders_[order.securityId()].erase(orderId);
    if (security_orders_[order.securityId()].empty()) {
        security_orders_.erase(order.securityId());
    }

    // Remove order from order map
    orders_.erase(it);
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
    auto it = user_orders_.find(user);
    if (it == user_orders_.end()) {
        // No orders for this user
        return;
    }

    // Copy order IDs to avoid iterator invalidation
    std::vector<std::string> orderIds(it->second.begin(), it->second.end());

    for (const auto& orderId : orderIds) {
        cancelOrder(orderId);
    }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {
    auto it = security_orders_.find(securityId);
    if (it == security_orders_.end()) {
        // No orders for this security ID
        return;
    }

    // Copy order IDs to avoid iterator invalidation
    std::vector<std::string> orderIds(it->second.begin(), it->second.end());

    for (const auto& orderId : orderIds) {
        auto order_it = orders_.find(orderId);
        if (order_it != orders_.end()) {
            if (order_it->second.qty() >= minQty) {
                cancelOrder(orderId);
            }
        }
    }
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) {
    // Get Buy quantities per company
    auto buy_it = buy_qty_per_company_.find(securityId);
    if (buy_it == buy_qty_per_company_.end()) {
        // No Buy orders for this security ID
        return 0;
    }
    auto& buyQtyPerCompany = buy_it->second;

    // Get Sell quantities per company
    auto sell_it = sell_qty_per_company_.find(securityId);
    if (sell_it == sell_qty_per_company_.end()) {
        // No Sell orders for this security ID
        return 0;
    }
    auto& sellQtyPerCompany = sell_it->second;

    unsigned int totalBuyQty = 0;
    for (const auto& pair : buyQtyPerCompany) {
        totalBuyQty += pair.second;
    }

    unsigned int totalSellQty = 0;
    for (const auto& pair : sellQtyPerCompany) {
        totalSellQty += pair.second;
    }

    // Potential matching quantities from Buy side
    unsigned int potentialMatchingFromBuy = 0;
    for (const auto& buy_pair : buyQtyPerCompany) {
        const std::string& buyCompany = buy_pair.first;
        unsigned int buyQty = buy_pair.second;

        unsigned int sellQtyFromOtherCompanies = totalSellQty;
        auto sell_company_it = sellQtyPerCompany.find(buyCompany);
        if (sell_company_it != sellQtyPerCompany.end()) {
            sellQtyFromOtherCompanies -= sell_company_it->second;
        }

        unsigned int potentialMatchQty = std::min(buyQty, sellQtyFromOtherCompanies);
        potentialMatchingFromBuy += potentialMatchQty;
    }

    // Potential matching quantities from Sell side
    unsigned int potentialMatchingFromSell = 0;
    for (const auto& sell_pair : sellQtyPerCompany) {
        const std::string& sellCompany = sell_pair.first;
        unsigned int sellQty = sell_pair.second;

        unsigned int buyQtyFromOtherCompanies = totalBuyQty;
        auto buy_company_it = buyQtyPerCompany.find(sellCompany);
        if (buy_company_it != buyQtyPerCompany.end()) {
            buyQtyFromOtherCompanies -= buy_company_it->second;
        }

        unsigned int potentialMatchQty = std::min(sellQty, buyQtyFromOtherCompanies);
        potentialMatchingFromSell += potentialMatchQty;
    }

    // The total matched quantity is the minimum of the potential matches from Buy and Sell sides
    unsigned int totalMatchedQty = std::min(potentialMatchingFromBuy, potentialMatchingFromSell);

    return totalMatchedQty;
}

std::vector<Order> OrderCache::getAllOrders() const {
    std::vector<Order> allOrders;
    allOrders.reserve(orders_.size());

    for (const auto& pair : orders_) {
        allOrders.push_back(pair.second);
    }

    return allOrders;
}

void OrderCache::updateQuantities(const Order& order, int qty_change) {
    const std::string& securityId = order.securityId();
    const std::string& company = order.company();

    if (order.side() == "Buy") {
        buy_qty_per_company_[securityId][company] += qty_change;
        if (buy_qty_per_company_[securityId][company] == 0) {
            buy_qty_per_company_[securityId].erase(company);
        }
        if (buy_qty_per_company_[securityId].empty()) {
            buy_qty_per_company_.erase(securityId);
        }
    } else if (order.side() == "Sell") {
        sell_qty_per_company_[securityId][company] += qty_change;
        if (sell_qty_per_company_[securityId][company] == 0) {
            sell_qty_per_company_[securityId].erase(company);
        }
        if (sell_qty_per_company_[securityId].empty()) {
            sell_qty_per_company_.erase(securityId);
        }
    }
}
