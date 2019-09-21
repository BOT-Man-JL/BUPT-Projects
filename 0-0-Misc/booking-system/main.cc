
// Entry of Booking System

#include <iostream>

#include "booking_item.h"
#include "booking_manager.h"

int main() {
  booking::BookingManager manager;

  char buffer[1024];
  while (std::cin.getline(buffer, sizeof buffer)) {
    std::string line = buffer;
    if (!line.empty()) {
      auto parsed = booking::BookingManager::ParseCommand(line);
      if (!parsed) {
        std::cout << "Error: the booking is invalid!\n";
        continue;
      }

      if (!parsed->second.is_cancel) {
        if (manager.Book(parsed->first, parsed->second))
          std::cout << "Success: the booking is accepted!\n";
        else
          std::cout << "Error: the booking conflicts with existing bookings!\n";
      } else {
        if (manager.Cancel(parsed->first, parsed->second))
          std::cout << "Success: the booking is cancelled!\n";
        else
          std::cout << "Error: the booking being cancelled does not exist!\n";
      }
    } else {
      std::cout << manager.GetSummary();
    }
  }
  return 0;
}
