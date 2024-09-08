// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <algorithm>

// Add an order to the cache
void OrderCache::addOrder(Order order) {
    auto it = ordersBySecId.emplace(order.securityId(), order);
    ordersById[order.orderId()] = it;
    ordersByUser[order.user()].push_back(it);
}

// Cancel a specific order by its orderId
void OrderCache::cancelOrder(const std::string& orderId) {
    auto it = ordersById.find(orderId);
    if (it != ordersById.end()) {
        // Remove from the security map
        ordersBySecId.erase(it->second);
        // Remove from the user list
        removeOrderFromUserMap(it->second->second.user(), it->second);
        // Remove from the orderId map
        ordersById.erase(it);
    }
}

// Cancel all orders for a specific user
void OrderCache::cancelOrdersForUser(const std::string& user) {
    auto it = ordersByUser.find(user);
    if (it != ordersByUser.end()) {
        for (auto orderIt : it->second) {
            ordersBySecId.erase(orderIt);
            ordersById.erase(orderIt->second.orderId());
        }
        ordersByUser.erase(it);
    }
}

// Cancel orders for a specific security with a minimum quantity
void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {
    auto range = ordersBySecId.equal_range(securityId);
    for (auto it = range.first; it != range.second; ) {
        if (it->second.qty() >= minQty) {
            removeOrderFromUserMap(it->second.user(), it);
            ordersById.erase(it->second.orderId());
            it = ordersBySecId.erase(it);  // Move to the next iterator
        } else {
            ++it;
        }
    }
}

// Get the total matching size for a security
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) {
    unsigned int totalMatchingSize = 0;
    auto range = ordersBySecId.equal_range(securityId);

    std::vector<std::multimap<std::string, Order>::iterator> buyOrders;
    std::vector<std::multimap<std::string, Order>::iterator> sellOrders;

    // Separate Buy and Sell orders
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.side() == "Buy") {
            buyOrders.push_back(it);
        } else if (it->second.side() == "Sell") {
            sellOrders.push_back(it);
        }
    }

    // Match Buy and Sell orders
    for (auto buyIt = buyOrders.begin(); buyIt != buyOrders.end(); ++buyIt) {
        for (auto sellIt = sellOrders.begin(); sellIt != sellOrders.end(); ) {
            Order& buyOrder = (*buyIt)->second;
            Order& sellOrder = (*sellIt)->second;

            if (buyOrder.company() != sellOrder.company()) {
                unsigned int matchQty = std::min(buyOrder.qty(), sellOrder.qty());
                totalMatchingSize += matchQty;

                // Reduce quantities in both buy and sell orders
                buyOrder.reduceQty(matchQty);
                sellOrder.reduceQty(matchQty);

                // If sell order is fully matched, move to the next sell order
                if (sellOrder.qty() == 0) {
                    sellIt = sellOrders.erase(sellIt);
                } else {
                    ++sellIt;
                }

                // If buy order is fully matched, break and move to the next buy order
                if (buyOrder.qty() == 0) {
                    break;
                }
            } else {
                ++sellIt;  // Skip if same company
            }
        }
    }

    return totalMatchingSize;
}

// Get all orders
std::vector<Order> OrderCache::getAllOrders() const {
    std::vector<Order> allOrders;
    for (const auto& orderPair : ordersBySecId) {
        allOrders.push_back(orderPair.second);
    }
    return allOrders;
}

// Helper method to remove an order from the user map
void OrderCache::removeOrderFromUserMap(const std::string& user, std::multimap<std::string, Order>::iterator orderIt) {
    auto& userOrders = ordersByUser[user];
    userOrders.erase(std::remove(userOrders.begin(), userOrders.end(), orderIt), userOrders.end());
    if (userOrders.empty()) {
        ordersByUser.erase(user);
    }
}
