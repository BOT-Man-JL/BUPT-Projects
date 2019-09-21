
// Unittest of Data structures of Booking Item

#include "booking_manager.h"

#include "catch2/catch.hpp"

TEST_CASE("BookingManager::ParseCommand", "[BookingManager::ParseCommand]") {
  using booking::BookingManager;

  // Good
  {
    REQUIRE_FALSE(BookingManager::ParseCommand("U001 2019-05-11 9:00~22:00 A")
                      ->second.is_cancel);
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:00~22:00 A C")
                ->second.is_cancel);
  }
  // Missing Field
  {
    REQUIRE(BookingManager::ParseCommand(" 2019-05-11 9:00~22:00 A") ==
            nullptr);
    REQUIRE(BookingManager::ParseCommand("U001  9:00~22:00 A") == nullptr);
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11  A") == nullptr);
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:00~22:00 ") ==
            nullptr);
  }
  // Invalid Cancel Flag
  {
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:00~22:00 A X") ==
            nullptr);
  }
  // Invalid Hour Format
  {
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:30~22:30 A") ==
            nullptr);
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:00-22:00 A") ==
            nullptr);
  }
  // Invalid Range
  {
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-32 9:00~22:00 A") ==
            nullptr);
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 12:00~12:00 A") ==
            nullptr);
  }
  // Invalid Court
  {
    REQUIRE(BookingManager::ParseCommand("U001 2019-05-11 9:00~22:00 E") ==
            nullptr);
  }
}

TEST_CASE("BookingManager::Book/Cancel", "[BookingManager::Book/Cancel]") {
  using booking::BookingManager;
  using booking::BookingRange;
  BookingManager manager;

  // Invalid Court
  {
    REQUIRE_FALSE(
        manager.Book("E", {"", *BookingRange::Create("2019-05-11", {9, 12})}));
    REQUIRE_FALSE(manager.Cancel(
        "E", {"", *BookingRange::Create("2019-05-11", {9, 12})}));
  }
  // Book
  {
    REQUIRE(manager.Book(
        "A", {"U001", *BookingRange::Create("2019-05-11", {9, 12})}));
    REQUIRE(manager.Book(
        "B", {"U001", *BookingRange::Create("2019-05-11", {9, 12})}));
    REQUIRE(manager.Book(
        "A", {"U001", *BookingRange::Create("2019-05-11", {12, 14})}));
    REQUIRE(manager.Book(
        "A", {"U002", *BookingRange::Create("2019-05-12", {9, 12})}));
  }
  // Booked Already
  {
    REQUIRE_FALSE(manager.Book(
        "A", {"U002", *BookingRange::Create("2019-05-11", {9, 12})}));
    REQUIRE_FALSE(manager.Book(
        "A", {"U001", *BookingRange::Create("2019-05-11", {10, 11})}));
  }
  // Cancel Not Match
  {
    REQUIRE_FALSE(manager.Cancel(
        "A", {"U001", *BookingRange::Create("2019-05-11", {10, 11})}));
    REQUIRE_FALSE(manager.Cancel(
        "A", {"U002", *BookingRange::Create("2019-05-11", {9, 12})}));
  }
  // Cancel
  {
    REQUIRE(manager.Cancel(
        "A", {"U001", *BookingRange::Create("2019-05-11", {9, 12})}));
  }
  // Cancelled Already
  {
    REQUIRE_FALSE(manager.Cancel(
        "A", {"U001", *BookingRange::Create("2019-05-11", {9, 12})}));
  }
  // Book Again
  {
    REQUIRE(manager.Book(
        "A", {"U001", *BookingRange::Create("2019-05-11", {9, 12})}));
  }
}

TEST_CASE("BookingManager::GetSummary", "[BookingManager::GetSummary]") {
  using booking::BookingManager;
  using booking::BookingRange;
  BookingManager manager;

  // Empty
  {
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
---
总计：0 元
)");
  }
  // Book
  {
    manager.Book("A", {"U1", *BookingRange::Create("2017-08-01", {19, 22})});
    manager.Book("B", {"U2", *BookingRange::Create("2017-08-02", {13, 17})});
    manager.Book("C", {"U3", *BookingRange::Create("2017-08-03", {15, 16})});
    manager.Book("D", {"U4", *BookingRange::Create("2017-08-05", {9, 11})});
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
场地:A
2017-08-01 19:00~22:00 200 元
小计：200 元

场地:B
2017-08-02 13:00~17:00 200 元
小计：200 元

场地:C
2017-08-03 15:00~16:00 50 元
小计：50 元

场地:D
2017-08-05 09:00~11:00 80 元
小计：80 元
---
总计：530 元
)");
  }
  // Cancel
  {
    manager.Cancel("A", {"U1", *BookingRange::Create("2017-08-01", {19, 22})});
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
场地:A
2017-08-01 19:00~22:00 违约金 100 元
小计：100 元

场地:B
2017-08-02 13:00~17:00 200 元
小计：200 元

场地:C
2017-08-03 15:00~16:00 50 元
小计：50 元

场地:D
2017-08-05 09:00~11:00 80 元
小计：80 元
---
总计：430 元
)");
  }
  // Book Again (Undefined)
  {
    manager.Book("A", {"U1", *BookingRange::Create("2017-08-01", {19, 22})});
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
场地:A
2017-08-01 19:00~22:00 200 元
小计：200 元

场地:B
2017-08-02 13:00~17:00 200 元
小计：200 元

场地:C
2017-08-03 15:00~16:00 50 元
小计：50 元

场地:D
2017-08-05 09:00~11:00 80 元
小计：80 元
---
总计：530 元
)");
  }
  // Cancel and Book With Different Time
  {
    manager.Cancel("A", {"U1", *BookingRange::Create("2017-08-01", {19, 22})});
    manager.Book("A", {"U1", *BookingRange::Create("2017-08-01", {21, 22})});
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
场地:A
2017-08-01 19:00~22:00 违约金 100 元
2017-08-01 21:00~22:00 60 元
小计：160 元

场地:B
2017-08-02 13:00~17:00 200 元
小计：200 元

场地:C
2017-08-03 15:00~16:00 50 元
小计：50 元

场地:D
2017-08-05 09:00~11:00 80 元
小计：80 元
---
总计：490 元
)");
  }
  // Cancel and Book-Cancel With Different Time Again
  {
    manager.Cancel("A", {"U1", *BookingRange::Create("2017-08-01", {21, 22})});
    manager.Book("A", {"U1", *BookingRange::Create("2017-08-01", {20, 21})});
    manager.Cancel("A", {"U1", *BookingRange::Create("2017-08-01", {20, 21})});
    REQUIRE(manager.GetSummary() == R"(收入汇总
---
场地:A
2017-08-01 19:00~22:00 违约金 100 元
2017-08-01 20:00~21:00 违约金 30 元
2017-08-01 21:00~22:00 违约金 30 元
小计：160 元

场地:B
2017-08-02 13:00~17:00 200 元
小计：200 元

场地:C
2017-08-03 15:00~16:00 50 元
小计：50 元

场地:D
2017-08-05 09:00~11:00 80 元
小计：80 元
---
总计：490 元
)");
  }
}
