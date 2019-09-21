
// Data structures of Booking Item

#include "booking_item.h"

#include <time.h>
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>

namespace {

constexpr auto kHourMin = 9;
constexpr auto kHourMax = 22;

}  // namespace

namespace booking {

std::unique_ptr<BookingRange> BookingRange::Create(const std::string& date,
                                                   HourInterval interval) {
  if (interval.first >= interval.second || interval.first < kHourMin ||
      interval.second > kHourMax)
    return nullptr;

  std::tm tm = {};
  std::istringstream iss(date);
  iss >> std::get_time(&tm, "%Y-%m-%d");
  if (iss.fail())
    return nullptr;

  auto ret = std::unique_ptr<BookingRange>(new BookingRange);
  ret->date_ = date;
  mktime(&tm);
  ret->is_weekend_ = !(tm.tm_wday % 6);
  ret->interval_ = interval;
  return ret;
}

bool operator==(const BookingRange& a, const BookingRange& b) {
  return a.date() == b.date() && a.interval().first == b.interval().first &&
         a.interval().second == b.interval().second;
}

bool operator<(const BookingRange& a, const BookingRange& b) {
  if (a.date() < b.date())
    return true;
  if (a.date() > b.date())
    return false;

  if (a.interval().first < b.interval().first)
    return true;
  if (a.interval().first > b.interval().first)
    return false;

  return a.interval().second < b.interval().second;
}

std::vector<Hour> Intersects(const HourInterval& a, const HourInterval& b) {
  auto generate_time_vecor = [](HourInterval interval) {
    std::vector<Hour> ret;
    std::generate_n(std::back_inserter(ret), interval.second - interval.first,
                    [n = interval.first]() mutable { return n++; });
    return ret;
  };
  auto vec_a = generate_time_vecor(a);
  auto vec_b = generate_time_vecor(b);

  std::vector<Hour> ret;
  std::set_intersection(std::begin(vec_a), std::end(vec_a), std::begin(vec_b),
                        std::end(vec_b), std::back_inserter(ret));
  return ret;
}

std::vector<Hour> Intersects(const BookingRange& a, const BookingRange& b) {
  if (a.date() != b.date())
    return {};
  return Intersects(a.interval(), b.interval());
}

Price BookingItem::price() const {
  using PriceTableItem = std::pair<HourInterval, Price>;
  using PriceTable = std::vector<PriceTableItem>;
  static const PriceTable weekday_prices{
      {{9, 12}, 30},
      {{12, 18}, 50},
      {{18, 20}, 80},
      {{20, 22}, 60},
  };
  static const PriceTable weekend_prices{
      {{9, 12}, 40},
      {{12, 18}, 50},
      {{18, 22}, 60},
  };

  auto acc_price = [](HourInterval hour_interval, const PriceTable& table) {
    return std::accumulate(
        std::begin(table), std::end(table), Price(0),
        [hour_interval](Price current, PriceTableItem item) {
          return current +
                 item.second * Intersects(hour_interval, item.first).size();
        });
  };

  auto adjust_rate = [](bool is_cancel, bool is_weekend) {
    if (!is_cancel)
      return 1.;
    return is_weekend ? .25 : .5;
  };

  return acc_price(booking_range.interval(), booking_range.is_weekend()
                                                 ? weekend_prices
                                                 : weekday_prices) *
         adjust_rate(is_cancel, booking_range.is_weekend());
}

}  // namespace booking
