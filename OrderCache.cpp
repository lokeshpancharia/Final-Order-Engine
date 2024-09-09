#include "OrderCache.h"
#include <algorithm>

// Constructor
OrderCache::OrderCache() {
    initializeCache();
}

// Initialize security by company array
void OrderCache::initializeCache() {
    for (int i = 0; i < NUM_SECURITIES; ++i) {
        for (int j = 0; j < NUM_COMPANIES; ++j) {
            securityByCompany[i][j][0] = 0;
            securityByCompany[i][j][1] = 0;
        }
    }
}

// Get Security ID from Security Identifier (string)
int OrderCache::getSecurityId(const std::string &securityId) {
    auto it = securityIdMap.find(securityId);
    if (it == securityIdMap.end()) {
        it = securityIdMap.emplace(securityId, static_cast<int>(securityIdMap.size() + 1)).first;
    }
    return it->second;
}

// Get Company ID from Company Identifier (string)
int OrderCache::getCompanyId(const std::string &companyId) {
    auto it = companyIdMap.find(companyId);
    if (it == companyIdMap.end()) {
        it = companyIdMap.emplace(companyId, static_cast<int>(companyIdMap.size() + 1)).first;
    }
    return it->second;
}

// Remove order from security mapping
void OrderCache::removeOrderFromSecurity(const std::string &orderId) {
    auto it = orders.find(orderId);
    if (it != orders.end()) {
        const Order& order = it->second;
        int securityId = getSecurityId(order.securityId());
        int companyId = getCompanyId(order.company());
        int side = (order.side() == sides[0]) ? 0 : 1;
         // Bounds checking
        if (securityId < NUM_SECURITIES && companyId < NUM_COMPANIES && side < 2) {
            securityByCompany[securityId][companyId][side] -= order.qty();
        }
    }
}

// Add order to the cache
void OrderCache::addOrder(Order order) {
    orders.emplace(order.orderId(), order);
    userOrders[order.user()].emplace(order.orderId());

    int securityId = getSecurityId(order.securityId());
    int companyId = getCompanyId(order.company());
    int side = (order.side() == sides[0]) ? 0 : 1;

     // Bounds checking
    if (securityId < NUM_SECURITIES && companyId < NUM_COMPANIES && side < 2) {
        secQtyOrders[securityId].emplace(order.orderId());
        securityByCompany[securityId][companyId][side] += order.qty();
    }
}

// Cancel an order by order ID
void OrderCache::cancelOrder(const std::string &orderId) {
    auto it = orders.find(orderId);
    if (it != orders.end()) {
        const std::string &userId = it->second.user();
        userOrders[userId].erase(orderId);
        removeOrderFromSecurity(orderId);
        orders.erase(orderId);
    }
}

// Cancel all orders for a specific user
void OrderCache::cancelOrdersForUser(const std::string &userId) {
  auto userOrdersIt = userOrders.find(userId);
  if (userOrdersIt == userOrders.end())
    return;

  std::vector<std::string> ordersToCancel(userOrdersIt->second.begin(), userOrdersIt->second.end());
  for (const auto &orderId : ordersToCancel) {
     cancelOrder(orderId);
  }
  userOrders[userId].clear();
}


// Cancel orders for a security with quantity >= minQty
void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) {
    int secId = getSecurityId(securityId);
    auto it = secQtyOrders.find(secId);
    if (it == secQtyOrders.end()) return;

    std::vector<std::string> ordersToRemove;
    for (const auto &orderId : it->second) {
        if (orders.at(orderId).qty() >= minQty) {
            ordersToRemove.push_back(orderId);
        }
    }

    for (const auto &orderId : ordersToRemove) {
        cancelOrder(orderId);
        it->second.erase(orderId);
    }
}

// Get the matching size for a given security ID
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId) {
    int securityIdInt = getSecurityId(securityId);
    unsigned int totalMatchingQty = 0;

    // Start with two pointers
    int companyIndex1, companyIndex2; 
    for (companyIndex1 = 1; companyIndex1 < companyIdMap.size(); ++companyIndex1) {
        for (companyIndex2 = companyIndex1 + 1; companyIndex2 <= companyIdMap.size(); ++companyIndex2) {
            // Bounds checking
            if (securityIdInt >= NUM_SECURITIES || companyIndex1 >= NUM_COMPANIES || companyIndex2 >= NUM_COMPANIES)
                continue;
            if (!securityByCompany[securityIdInt][companyIndex1][0]) 
                break;
            unsigned int matchingQty = std::min(securityByCompany[securityIdInt][companyIndex1][0], 
                                                securityByCompany[securityIdInt][companyIndex2][1]);
            totalMatchingQty += matchingQty;
            securityByCompany[securityIdInt][companyIndex1][0] -= matchingQty;
            securityByCompany[securityIdInt][companyIndex2][1] -= matchingQty;
        }
    }

    // Iterate from the first company to the last for additional matching
    for (companyIndex2 = 1; companyIndex2 < companyIdMap.size(); ++companyIndex2) {
        if (securityIdInt >= NUM_SECURITIES || companyIndex1 >= NUM_COMPANIES || companyIndex2 >= NUM_COMPANIES)
            continue;
        if (!securityByCompany[securityIdInt][companyIndex1][0])
            break;
        unsigned int matchingQty = std::min(securityByCompany[securityIdInt][companyIndex1][0], 
                                            securityByCompany[securityIdInt][companyIndex2][1]);
        totalMatchingQty += matchingQty;
        securityByCompany[securityIdInt][companyIndex1][0] -= matchingQty;
        securityByCompany[securityIdInt][companyIndex2][1] -= matchingQty;
    }

    return totalMatchingQty;
}



// Get all orders in the cache
std::vector<Order> OrderCache::getAllOrders() const {
    std::vector<Order> orderList;
    for (const auto &[orderId, order] : orders) {
        orderList.push_back(order);
    }
    return orderList;
}
