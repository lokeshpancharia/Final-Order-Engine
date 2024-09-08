// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <algorithm>  // for std::min
using namespace std;
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
  auto it = orderlist.find(orderId);
  if (it != orderlist.end())
  {
    int securityIdforOrder = getSecurityId(it->second.securityId());
    int companyIdforOrder = getCompanyId(it->second.company());
    int side = it->second.side() == sides[0] ? 0 : 1;
    unsigned int qty = it->second.qty();

    // Reduce Qty from the security id for company of this order
    securityByCompany[securityIdforOrder][companyIdforOrder][side] -= qty;
  }
}


// Add an order to the cache
void OrderCache::addOrder(Order order)
{
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
// Cancel all orders for a specific user
void OrderCache::cancelOrdersForUser(const std::string &user)
{
  auto it = userOrders.find(user);
  if (it == userOrders.end())
    return;

  std::vector<std::string> ordersToCancel(it->second.begin(), it->second.end());
  for (const auto &orderId : ordersToCancel)
  {
    cancelOrder(orderId);
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

  for (auto orderIt = it->second.begin(); orderIt != it->second.end(); )
  {
    const std::string &orderId = *orderIt;
    unsigned qty = orderlist.at(orderId).qty();
    if (qty >= minQty)
    {
      cancelOrder(orderId);
      orderIt = it->second.erase(orderIt);  // Erase while iterating
    }
    else
    {
      ++orderIt;
    }
  }
}

// Get the matching size for a given security ID
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId)
{
  int secId = getSecurityId(securityId);
  unsigned int totalMatching  = 0;
  int i,j; // loop variables
  for (i = 1; i < companyIdInteger.size(); ++i)
  {
    for (j=i+1; j <= companyIdInteger.size(); ++j)
    {
      if (!securityByCompany[secId][i][0])
        break;
      unsigned int matching = min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
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
    unsigned matching = min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
    totalMatching  += matching;
    securityByCompany[secId][i][0] -= matching;
    securityByCompany[secId][j][1] -= matching;
  }
  return totalMatching ;
}

// Get all orders in the cache
std::vector<Order> OrderCache::getAllOrders() const
{
  vector<Order> orders;
  for (const auto &[orderId, order] : orderlist)
  {
    orders.push_back(order);
  }
  return orders;
}
