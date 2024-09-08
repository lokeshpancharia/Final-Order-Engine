#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>
#include <mutex>
#include <shared_mutex>

class Order
{

public:
  // do not alter signature of this constructor

  Order(
      const std::string &ordId,
      const std::string &secId,
      const std::string &side,
      const unsigned int qty,
      const std::string &user,
      const std::string &company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) {}

  // do not alter these accessor methods
  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

private:
  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;    // unique order id
  std::string m_securityId; // security identifier
  std::string m_side;       // side of the order, eg Buy or Sell
  unsigned int m_qty;       // qty for this order
  std::string m_user;       // user name who owns this order
  std::string m_company;    // company for user
};

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface
{

public:
  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string &orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string &user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;
};

// Todo: Your implementation of the OrderCache...
class OrderCache : public OrderCacheInterface
{

public:
  OrderCache()
  {
    init();
  }

  void addOrder(Order order) override;

  void cancelOrder(const std::string &orderId) override;

  void cancelOrdersForUser(const std::string &user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(const std::string &securityId) override;

  std::vector<Order> getAllOrders() const override;

private:
 // Helper methods and member variables
  void init();
  int getSecurityId(const std::string &securityId);
  int getCompanyId(const std::string &companyId);
  void removeOrderFromSecurityId(const std::string &orderId);
  void removeOrderInParallel(const std::vector<std::string>& orderIds);
  std::mutex& getOrderLock(const std::string& orderId);

  std::unordered_map<std::string, Order> orderlist; // Order storage
  std::unordered_map<std::string, std::unordered_set<std::string>> userOrders; // Orders by user

  static constexpr unsigned int NUM_USERS = 1005;
  static constexpr unsigned int NUM_COMPANIES = 105;
  static constexpr unsigned int NUM_SECURITIES = 1005;

  // Security by company, 2D array for tracking
  int securityByCompany[NUM_SECURITIES][NUM_COMPANIES][2];

    // Store sides (Buy/Sell)
  std::vector<std::string> sides{"Buy", "Sell"};

    // Mapping security and company IDs to integers
  std::unordered_map<std::string, int> securityIdInteger;
  std::unordered_map<std::string, int> companyIdInteger;

    // Orders by security quantity
  std::unordered_map<int, std::unordered_set<std::string>> secQtyOrders;

  int totalOrders = 0;
};
