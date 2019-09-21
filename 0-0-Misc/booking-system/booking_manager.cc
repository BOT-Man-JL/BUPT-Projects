
// Booking Manager

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif  // _MSC_VER

#include "booking_manager.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace {

using booking::CourtId;

bool IsValidCourt(CourtId court) {
  static const std::vector<CourtId> v{"A", "B", "C", "D"};
  return std::any_of(std::begin(v), std::end(v),
                     [court](CourtId c) { return c == court; });
}

}  // namespace

namespace booking {

BookingManager::BookingManager() {}

bool BookingManager::Book(CourtId court, const BookingItem& item) {
  if (!IsValidCourt(court))
    return false;

  for (const auto& booked_item : items_[court]) {
    if (!booked_item.is_cancel &&
        !Intersects(booked_item.booking_range, item.booking_range).empty())
      return false;
  }

  items_[court].erase(item);
  items_[court].insert([&item] {
    auto item_copy = item;
    item_copy.is_cancel = false;
    return item_copy;
  }());
  return true;
}

bool BookingManager::Cancel(CourtId court, const BookingItem& item) {
  if (!IsValidCourt(court))
    return false;

  auto court_iter = items_.find(court);
  if (court_iter == items_.end())
    return false;

  for (const auto& booked_item : court_iter->second) {
    if (booked_item.is_cancel || booked_item.user != item.user ||
        !(booked_item.booking_range == item.booking_range))
      continue;

    const_cast<BookingItem&>(booked_item).is_cancel = true;
    return true;
  }
  return false;
}

std::string BookingManager::GetSummary() const {
  std::ostringstream oss;
  oss << "收入汇总\n"
      << "---\n";

  size_t index = 0;
  Price total = 0;
  for (const auto& court_items : items_) {
    Price sub_total = 0;

    oss << "场地:" << court_items.first << "\n";

    for (const auto& item : court_items.second) {
      const auto price = item.price();

      oss << item.booking_range.date() << " ";
      oss.width(2);
      oss.fill('0');
      oss << item.booking_range.interval().first << ":00"
          << "~";
      oss.width(2);
      oss.fill('0');
      oss << item.booking_range.interval().second << ":00"
          << " ";
      oss << (item.is_cancel ? "违约金 " : "") << price << " 元\n";

      sub_total += price;
    }

    oss << "小计：" << sub_total << " 元\n";

    if (index < items_.size() - 1)
      oss << "\n";
    ++index;

    total += sub_total;
  }

  oss << "---\n"
      << "总计：" << total << " 元\n";
  return oss.str();
}

std::unique_ptr<std::pair<CourtId, BookingItem>> BookingManager::ParseCommand(
    const std::string& line) {
  booking::UserId user;
  std::string date;
  std::string range;
  booking::CourtId court;

  std::istringstream iss(line);
  iss >> user >> date >> range >> court;
  if (iss.fail())
    return nullptr;

  std::string cancel_flag;
  iss >> cancel_flag;

  if (!cancel_flag.empty() && cancel_flag != "C")
    return nullptr;

  booking::Hour beg = 0;
  booking::Hour end = 0;
  if (sscanf(range.c_str(), "%lu:00~%lu:00", &beg, &end) != 2)
    return nullptr;

  auto range_obj = booking::BookingRange::Create(date, {beg, end});
  if (!range_obj)
    return nullptr;

  if (!IsValidCourt(court))
    return nullptr;

  return std::make_unique<std::pair<CourtId, BookingItem>>(
      court, booking::BookingItem{user, *range_obj, cancel_flag == "C"});
}

bool BookingManager::BookingItemCmp::operator()(const BookingItem& a,
                                                const BookingItem& b) const {
  return a.booking_range < b.booking_range;
}

}  // namespace booking
