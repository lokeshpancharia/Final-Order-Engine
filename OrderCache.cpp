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

void OrderCache::cancelOrder(const std::string &orderId)
{
  // Todo...
  if (orderlist.find(orderId) != orderlist.end())
  {
    string userId = orderlist.at(orderId).user();
    userOrders[userId].erase(orderId);
    // Need to remove security Id and Quantity from this order
    removeOrderFromSecurityId(orderId);
    orderlist.erase(orderId);
  }
}

void OrderCache::cancelOrdersForUser(const std::string &user)
{
  // Todo...
  if (userOrders.find(user) == userOrders.end())
    return;
  std::vector<std::string> ordersToCancel(userOrders[user].begin(), userOrders[user].end());

  for (const auto &orderId : ordersToCancel)
  {
    cancelOrder(orderId);
  }
  userOrders[user].clear();
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty)
{
  // Todo...
  int secId = getSecurityId(securityId);
  if (secQtyOrders.find(secId) == secQtyOrders.end())
    return;
  for (auto &order : secQtyOrders.at(secId))
  {
    unsigned qty = orderlist.at(order).qty();
    if (qty >= minQty)
    {
      cancelOrder(order);
      secQtyOrders[secId].erase(order);
    }
  }
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId)
{
  // Todo...
  int secId = getSecurityId(securityId);
  int i, j;
  unsigned int ans = 0;
  for (i = 1; i < companyIdInteger.size(); i++)
  {
    int j = i + 1;

    for (; j <= companyIdInteger.size(); j++)
    {
      if (!securityByCompany[secId][i][0])
        break;
      int matching = min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
      ans += matching;
      securityByCompany[secId][i][0] -= matching;
      securityByCompany[secId][j][1] -= matching;
    }
  }

  // do for last company also by iterating from 1st to last
  for (j = 1; j < companyIdInteger.size(); j++)
  {
    if (!securityByCompany[secId][i][0])
      break;
    int matching = min(securityByCompany[secId][i][0], securityByCompany[secId][j][1]);
    ans += matching;
    securityByCompany[secId][i][0] -= matching;
    securityByCompany[secId][j][1] -= matching;
  }
  return ans;
}

std::vector<Order> OrderCache::getAllOrders() const
{
  // Todo...
  vector<Order> orders;
  for (auto &[k, v] : orderlist)
  {
    orders.push_back(v);
  }
  return orders;
}
