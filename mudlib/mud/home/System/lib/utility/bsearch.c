#include <kotaka/paths.h>

inherit SECOND_AUTO;

/* find the largest value lesser than or equal to the target */
static int binary_search_floor(mixed *arr, mixed value)
{
	int begin;
	int end;
	
	begin = 0;
	end = sizeof(arr);
	
	if (begin == end) {
		/* zero size array */
		return -1;
	}
	
	if (value < arr[begin]) {
		/* completely out of range */
		return -1;
	}
	
	if (value >= arr[end - 1]) {
		/* it floats to the top floor */
		return end - 1;
	}
	
	while (end - begin > 1) {
		int midpoint;
		mixed probe;
		
		midpoint = (begin + end) >> 1;
		
		probe = arr[midpoint];
		
		if (value < probe) {
			/* too low */
			end = midpoint;
		} else {
			begin = midpoint;
		}
	}
	
	return begin;
}

/* find the smallest value greater than or equal to the target */
static int binary_search_ceiling(mixed *arr, mixed value)
{
	int begin;
	int end;
	
	begin = 0;
	end = sizeof(arr);
	
	if (value <= arr[begin]) {
		/* floats to the first floor */
		return 0;
	}
	
	if (value > arr[end - 1]) {
		/* goes into orbit */
		return end;
	}
	
	while (end - begin > 1) {
		int midpoint;
		mixed probe;
		
		midpoint = (begin + end) >> 1;
		
		probe = arr[midpoint];
		
		if (value <= probe) {
			end = midpoint;
		} else {
			/* too high */
			begin = midpoint;
		}
	}
	
	return begin;
}
