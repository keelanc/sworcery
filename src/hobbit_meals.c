#include "hobbit_meals.h"

static const char* const MEALS[] = {
	"almost\nbreakfast",
	"breakfast",/*6am*/
	"seven-ish",
	"almost second\nbreakfast",
	"second\nbreakfast",/*9am*/
	"almost\nelevenses",
	"elevenses",/*11am*/
	"luncheon",/*12pm*/
	"sleep",
	"afternoon\ntea",/*2pm*/
	"three-ish",
	"almost\ndinner",
	"dinner",/*5pm*/
	"almost\nsupper",
	"supper",/*7pm*/
	"eight-ish",
	"nine-ish"
};


void hobbit_time(int int_hour, char* str_hour) {
	strcpy(str_hour, "");
	if (int_hour > 4 && int_hour < 22) {
		strcat(str_hour, MEALS[int_hour - 5]);
	}
	else {
		strcat(str_hour, "sleep");
	}

}