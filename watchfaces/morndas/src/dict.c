#include "dict.h"

char* dict_get_weekday(int day) {
  switch(day) {
    case 0: return "Sundas";
    case 1: return "Morndas";
    case 2: return "Tirdas";
    case 3: return "Middas";
    case 4: return "Turdas";
    case 5: return "Fredas";
    case 6: return "Loredas";
    default: return "UNKNOWN";
  }
}

char* dict_get_month(int month) {
  switch(month) {
    case 0: return "Morning Star";
    case 1: return "Sun's Dawn";
    case 2: return "First Seed";
    case 3: return "Rain's Hand";
    case 4: return "Second Seed";
    case 5: return "Mid Year";
    case 6: return "Sun's Height";
    case 7: return "Last Seed";
    case 8: return "Heartfire";
    case 9: return "Frostfall";
    case 10: return "Sun's Dusk";
    case 11: return "Evening Star";
    default: return "UNKNOWN";
  }
}
