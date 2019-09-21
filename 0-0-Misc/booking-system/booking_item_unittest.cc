
// Unittest of Data structures of Booking Item

#include "booking_item.h"

#include "catch2/catch.hpp"

TEST_CASE("BookingRange::Create", "[BookingRange::Create]") {
  using booking::BookingRange;

  // Good
  {
    REQUIRE(BookingRange::Create("2019-05-11", {9, 22}) != nullptr);
    REQUIRE(BookingRange::Create("2019-05-11", {10, 21}) != nullptr);
  }
  // Bad Hour
  {
    REQUIRE(BookingRange::Create("2019-05-11", {9, 9}) == nullptr);
    REQUIRE(BookingRange::Create("2019-05-11", {22, 22}) == nullptr);
    REQUIRE(BookingRange::Create("2019-05-11", {8, 22}) == nullptr);
    REQUIRE(BookingRange::Create("2019-05-11", {9, 23}) == nullptr);
  }
  // Bad Date
  {
    REQUIRE(BookingRange::Create("2019-13-11", {9, 22}) == nullptr);
    REQUIRE(BookingRange::Create("2019-05-32", {9, 22}) == nullptr);
    REQUIRE(BookingRange::Create("20190511", {9, 22}) == nullptr);
    REQUIRE(BookingRange::Create("2019/05/11", {9, 22}) == nullptr);
  }
}

TEST_CASE("BookingRange::operator", "[BookingRange::operator]") {
  using booking::BookingRange;

  // operator==
  {
    REQUIRE(*BookingRange::Create("2019-05-11", {9, 22}) ==
            *BookingRange::Create("2019-05-11", {9, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-11", {9, 22}) ==
                  *BookingRange::Create("2019-05-12", {9, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-11", {9, 22}) ==
                  *BookingRange::Create("2019-05-11", {10, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-11", {9, 22}) ==
                  *BookingRange::Create("2019-05-11", {9, 21}));
  }

  // NOTE: symmetric tests are required for cmp ops
  // operator<
  {
    REQUIRE(*BookingRange::Create("2019-05-11", {9, 22}) <
            *BookingRange::Create("2019-05-12", {9, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-12", {9, 22}) <
                  *BookingRange::Create("2019-05-11", {9, 22}));

    REQUIRE(*BookingRange::Create("2019-05-11", {9, 22}) <
            *BookingRange::Create("2019-05-11", {10, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-11", {10, 22}) <
                  *BookingRange::Create("2019-05-11", {9, 22}));

    REQUIRE(*BookingRange::Create("2019-05-11", {9, 21}) <
            *BookingRange::Create("2019-05-11", {9, 22}));
    REQUIRE_FALSE(*BookingRange::Create("2019-05-11", {9, 22}) <
                  *BookingRange::Create("2019-05-11", {9, 21}));
  }
}

TEST_CASE("HourInterval::Intersects", "[HourInterval::Intersects]") {
  using booking::Hour;
  using booking::HourInterval;
  using booking::Intersects;

  // Good
  {
    REQUIRE(Intersects(HourInterval{9, 12}, HourInterval{10, 14}) ==
            std::vector<Hour>{10, 11});
    REQUIRE(Intersects(HourInterval{9, 12}, HourInterval{9, 14}) ==
            std::vector<Hour>{9, 10, 11});
    REQUIRE(Intersects(HourInterval{9, 12}, HourInterval{8, 12}) ==
            std::vector<Hour>{9, 10, 11});
  }
  // Empty
  {
    REQUIRE(Intersects(HourInterval{9, 10}, HourInterval{10, 11}).empty());
    REQUIRE(Intersects(HourInterval{9, 10}, HourInterval{12, 13}).empty());
  }
}

TEST_CASE("BookingRange::Intersects", "[BookingRange::Intersects]") {
  using booking::BookingRange;
  using booking::Hour;
  using booking::Intersects;

  // Good
  {
    REQUIRE(Intersects(*BookingRange::Create("2019-05-11", {9, 12}),
                       *BookingRange::Create("2019-05-11", {10, 14})) ==
            std::vector<Hour>{10, 11});
  }
  // Empty
  {
    REQUIRE(Intersects(*BookingRange::Create("2019-05-11", {9, 12}),
                       *BookingRange::Create("2019-05-12", {10, 14}))
                .empty());
  }
}

TEST_CASE("BookingItem::price", "[BookingItem::price]") {
  using booking::BookingItem;
  using booking::BookingRange;
  using booking::Price;

  // Weekday
  {
    REQUIRE(
        BookingItem{"", *BookingRange::Create("2019-05-10", {9, 12})}.price() ==
        Price(90));
    REQUIRE(
        BookingItem{"", *BookingRange::Create("2019-05-10", {9, 22})}.price() ==
        Price(670));
  }
  // Weekend
  {
    REQUIRE(
        BookingItem{"", *BookingRange::Create("2019-05-11", {9, 12})}.price() ==
        Price(120));
    REQUIRE(
        BookingItem{"", *BookingRange::Create("2019-05-11", {9, 22})}.price() ==
        Price(660));
  }
  // Cancel
  {
    REQUIRE(BookingItem{"", *BookingRange::Create("2019-05-10", {9, 12}), true}
                .price() == Price(90) * .5);
    REQUIRE(BookingItem{"", *BookingRange::Create("2019-05-11", {9, 12}), true}
                .price() == Price(120) * .25);
  }
}
