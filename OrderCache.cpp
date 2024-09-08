// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
using namespace std;

// Initialize
void OrderCache::init()
{
  int i, j;
  for (i = 0; i < NUM_SECURITIES; i++)
  {
    for (j = 0; j < NUM_COMPANIES; j++)
    {
      securityByCompany[i][j][0] = 0;
      securityByCompany[i][j][1] = 0;
    }
  }
}

// getSecurityId from SecurityId - String

int OrderCache::getSecurityId(string SecurityId)
{
  if (securityIdInteger.find(SecurityId) == securityIdInteger.end())
  {
    securityIdInteger.emplace(SecurityId, securityIdInteger.size() + 1);
  }
  return securityIdInteger.at(SecurityId);
}

// Integer companyId for each Company
int OrderCache::getCompanyId(string CompanyId)
{
  if (companyIdInteger.find(CompanyId) == companyIdInteger.end())
  {
    companyIdInteger.emplace(CompanyId, companyIdInteger.size() + 1);
  }
  return companyIdInteger.at(CompanyId);
}

// remove order from SecurityId
void OrderCache::removeOrderFromSecurityId(string orderId)
{
  int securityIdforOrder = getSecurityId(orderlist.at(orderId).securityId());
  int companyIdforOrder = getCompanyId(orderlist.at(orderId).company());
  int side = orderlist.at(orderId).side() == sides[0] ? 0 : 1;
  unsigned int qty = orderlist.at(orderId).qty();

  // Reduce Qty from the security id for company of this order
  securityByCompany[securityIdforOrder][companyIdforOrder][side] -= qty;
}
// Function definition for the files
void OrderCache::addOrder(Order order)
{
  Order curOrder(order.orderId(), order.securityId(), order.side(), order.qty(), order.user(), order.company());
  orderlist.emplace(order.orderId(), curOrder);
  userOrders[order.user()].emplace(order.orderId());

  int securityId = getSecurityId(order.securityId());
  int companyId = getCompanyId(order.company());
  int side = (order.side() == sides[0]) ? 0 : 1;

  secQtyOrders[securityId].emplace(order.orderId());
  // securityQuantityOrderlist.emplace(make_pair(securityId, order.qty()), order.orderId());

  // update orderQuantiy
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
