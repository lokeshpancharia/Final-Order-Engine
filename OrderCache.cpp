// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <algorithm>  // for std::min
#include <thread>


std::unordered_map<std::string, std::mutex> orderLocks; // Map of orderId to mutex
std::mutex orderLocksMapMutex; // Mutex to protect access to orderLocks map

// Helper function to get or create a mutex for a specific orderId
std::mutex& OrderCache::getOrderLock(const std::string& orderId) {
    std::unique_lock lock(orderLocksMapMutex); // Lock to ensure safe access to the map
    return orderLocks[orderId]; // Will create the mutex if it doesn't exist
}

// Initialize
void OrderCache::init()
{
  for (int i = 0; i < NUM_SECURITIES; ++i)
  {
    for (int j = 0; j < NUM_COMPANIES; ++j)
    {
      securityByCompany[i][j][0] = 0;
      securityByCompany[i][j][1] = 0;
    }
  }
}

// getSecurityId from SecurityId - String to Integer
int OrderCache::getSecurityId(const std::string &securityId)
{
  auto it = securityIdInteger.find(securityId);
    if (it == securityIdInteger.end())
    {
        it = securityIdInteger.emplace(securityId, securityIdInteger.size() + 1).first;
    }
    return it->second;
}

// Get company ID from company identifier (string)
int OrderCache::getCompanyId(const std::string &companyId)
{
  auto it = companyIdInteger.find(companyId);
  if (it == companyIdInteger.end())
  {
    it = companyIdInteger.emplace(companyId, companyIdInteger.size() + 1).first;
  }
  return it->second;
}

// Remove order from security ID
void OrderCache::removeOrderFromSecurityId(const std::string &orderId)
{
  std::mutex &orderLock = getOrderLock(orderId); // Get the lock for this orderId
    std::lock_guard lock(orderLock); // Lock the specific orderId

  auto it = orderlist.find(orderId);
  if (it != orderlist.end())
  {
  const Order& order = it->second;
  int securityIdforOrder = getSecurityId(order.securityId());
  int companyIdforOrder = getCompanyId(order.company());
  int side = order.side() == sides[0] ? 0 : 1;
  unsigned int qty = order.qty();

  // Reduce Qty from the security id for company of this order
  securityByCompany[securityIdforOrder][companyIdforOrder][side] -= qty;
  }
}


// Add an order to the cache
void OrderCache::addOrder(Order order)
{
    std::mutex &orderLock = getOrderLock(order.orderId()); // Get the lock for this orderId
    std::lock_guard lock(orderLock); // Lock the specific orderId
    orderlist.emplace(order.orderId(), order);
    userOrders[order.user()].emplace(order.orderId());

    int securityId = getSecurityId(order.securityId());
    int companyId = getCompanyId(order.company());
    int side = (order.side() == sides[0]) ? 0 : 1;

    secQtyOrders[securityId].emplace(order.orderId());

    // Update order quantity
    securityByCompany[securityId][companyId][side] += order.qty();
}
// Cancel an order by order ID
void OrderCache::cancelOrder(const std::string &orderId)
{
  auto it = orderlist.find(orderId);
  if (it != orderlist.end())
  {
      const std::string &userId = it->second.user();
      userOrders[userId].erase(orderId);
      removeOrderFromSecurityId(orderId);
      orderlist.erase(orderId);
  }
}


void OrderCache::removeOrderInParallel(const std::vector<std::string>& orderIds)
{
    std::vector<std::thread> threads;

    for (const auto& orderId : orderIds)
    {
        threads.emplace_back([this, &orderId]() {
            this->removeOrderFromSecurityId(orderId);
        });
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}


// Cancel all orders for a specific user
void OrderCache::cancelOrdersForUser(const std::string &user)
{
  auto it = userOrders.find(user);
  if (it == userOrders.end())
    return;

 std::vector<std::string> ordersToCancel(it->second.begin(), it->second.end());

    // Divide the work into smaller chunks and process in parallel
    int segmentSize = 4; // Adjust size according to performance needs
    for (size_t i = 0; i < ordersToCancel.size(); i += segmentSize)
    {
        std::vector<std::string> segment(ordersToCancel.begin() + i, 
                 ordersToCancel.begin() + std::min(i + segmentSize, ordersToCancel.size()));
        removeOrderInParallel(segment);
    }

  userOrders[user].clear();
}

// Cancel orders for a security ID that have a quantity greater than or equal to a minimum quantity
void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty)
{
  int secId = getSecurityId(securityId);
  auto it = secQtyOrders.find(secId);
  if (it == secQtyOrders.end())
      return;

  std::vector<std::string> ordersToRemove;
  for (const auto &orderId : it->second)
  {
    unsigned qty = orderlist.at(orderId).qty();
    if (qty >= minQty)
    {
      ordersToRemove.push_back(orderId);
    }
  }

  // Divide the work into smaller chunks and process in parallel
    int segmentSize = 4; // Adjust size according to performance needs
    for (size_t i = 0; i < ordersToRemove.size(); i += segmentSize)
    {
        std::vector<std::string> segment(ordersToRemove.begin() + i, 
                  ordersToRemove.begin() + std::min(i + segmentSize, ordersToRemove.size()));
        removeOrderInParallel(segment);
    }

   it->second.clear();
}

// Get the matching size for a given security ID
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId)
{
  int secId = getSecurityId(securityId);
  unsigned int totalMatching  = 0;
 int i = 1, j = 2; // Start with two pointers
  for (i = 1; i < companyIdInteger.size(); ++i)
  {
    for (j=i+1; j <= companyIdInteger.size(); ++j)
    {
      if (!securityByCompany[secId][i][0])
        break;
      unsigned int matching = std::min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
      totalMatching  += matching;
      securityByCompany[secId][i][0] -= matching;
      securityByCompany[secId][j][1] -= matching;
    }
  }


  // do for last company also by iterating from 1st to last
  for (j = 1; j < companyIdInteger.size(); ++j)
  {
    if (!securityByCompany[secId][i][0])
      break;
    unsigned matching = std::min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
    totalMatching  += matching;
    securityByCompany[secId][i][0] -= matching;
    securityByCompany[secId][j][1] -= matching;
  }
  return totalMatching ;
}

// Get all orders in the cache
std::vector<Order> OrderCache::getAllOrders() const
{
  std::vector<Order> orders;
  for (const auto &[orderId, order] : orderlist)
  {
    orders.push_back(order);
  }
  return orders;
}
