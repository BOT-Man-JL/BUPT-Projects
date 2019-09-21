
// Data structures of Booking Item

#ifndef BOOKING_ITEM_H_
#define BOOKING_ITEM_H_

#include <stddef.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace booking {

using Price = double;
using UserId = std::string;
using CourtId = std::string;

using Hour = size_t;
using HourInterval = std::pair<Hour, Hour>;

class BookingRange {
 public:
  static std::unique_ptr<BookingRange> Create(const std::string& date,
                                              HourInterval interval);

  const std::string& date() const { return date_; }
  bool is_weekend() const { return is_weekend_; }

  HourInterval interval() const { return interval_; }

 private:
  BookingRange() = default;

  std::string date_;
  bool is_weekend_ = false;

  HourInterval interval_;
};

bool operator==(const BookingRange& a, const BookingRange& b);
bool operator<(const BookingRange& a, const BookingRange& b);

std::vector<Hour> Intersects(const HourInterval& a, const HourInterval& b);
std::vector<Hour> Intersects(const BookingRange& a, const BookingRange& b);

struct BookingItem {
  UserId user;
  BookingRange booking_range;
  bool is_cancel = false;

  Price price() const;
};

}  // namespace booking

#endif  // BOOKING_ITEM_H_
