
// Booking Manager

#ifndef BOOKING_MANAGER_H_
#define BOOKING_MANAGER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "booking_item.h"

namespace booking {

class BookingManager {
 public:
  BookingManager();

  bool Book(CourtId court, const BookingItem& item);
  bool Cancel(CourtId court, const BookingItem& item);

  std::string GetSummary() const;

  static std::unique_ptr<std::pair<CourtId, BookingItem>> ParseCommand(
      const std::string& line);

 private:
  struct BookingItemCmp {
    bool operator()(const BookingItem& a, const BookingItem& b) const;
  };
  std::map<CourtId, std::set<BookingItem, BookingItemCmp>> items_;

  BookingManager(const BookingManager&) = delete;
  BookingManager& operator=(const BookingManager&) = delete;
};

}  // namespace booking

#endif  // BOOKING_MANAGER_H_
