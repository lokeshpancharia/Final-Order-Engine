#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>


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
    OrderCache();

    void addOrder(Order order) override;
    void cancelOrder(const std::string &orderId) override;
    void cancelOrdersForUser(const std::string &user) override;
    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override;
    unsigned int getMatchingSizeForSecurity(const std::string &securityId) override;
    std::vector<Order> getAllOrders() const override;

private:
    void initializeCache();
    int getSecurityId(const std::string &securityId);
    int getCompanyId(const std::string &companyId);
    void removeOrderFromSecurity(const std::string &orderId);

    std::unordered_map<std::string, Order> orders;                      // Order storage
    std::unordered_map<std::string, std::unordered_set<std::string>> userOrders; // Orders by user
    std::unordered_map<int, std::unordered_set<std::string>> secQtyOrders; // Orders by security ID and quantity

    static constexpr unsigned int NUM_SECURITIES = 1005;
    static constexpr unsigned int NUM_COMPANIES = 105;

    int securityByCompany[NUM_SECURITIES][NUM_COMPANIES][2]{}; // Security by company, sides Buy/Sell

    std::unordered_map<std::string, int> securityIdMap;
    std::unordered_map<std::string, int> companyIdMap;
    std::vector<std::string> sides{"Buy", "Sell"};
};
