#include "time_as_words.h"


static const char* const ONES[] = {
	"CLOCK",
	"ONE",
	"TWO",
	"THREE",
	"FOUR",
	"FIVE",
	"SIX",
	"SEVEN",
	"EIGHT",
	"NINE",
	"TEN",
	"ELEVEN",
	"TWELVE"
};

static const char* const TEENS[] = {
	"",
	"ELEVEN",
	"TWELVE",
	"THIRTEEN",
	"FOURTEEN",
	"FIFTEEN",
	"SIXTEEN",
	"SEVENTEEN",
	"EIGHTEEN",
	"NINETEEN"
};

static const char* const TENS[] = {
	"O'",
	"TEN",
	"TWENTY",
	"THIRTY",
	"FORTY",
	"FIFTY"
};


void time_as_words(int int_hour, int int_min, char* str_words) {
	
	strcpy(str_words, "");
	
	//hour
	if (int_hour % 12 == 0) {
		strcat(str_words, ONES[12]);
	}
	else {
		strcat(str_words, ONES[int_hour % 12]);
	}
	
	//newline
	strcat(str_words, "\n");
	
	//minute
	if (int_min > 10 && int_min < 20) {
		strcat(str_words, TEENS[int_min - 10]);
	}
	else {
		strcat(str_words, TENS[int_min / 10]);
		if (int_min % 10 != 0 || int_min == 0) {
			
			strcat(str_words, " ");
			strcat(str_words, ONES[int_min % 10]);
		}
	}
	
}