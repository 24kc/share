#include <stdio.h>
#include <stdbool.h>
#include "bitmap.h"

int main()
{
	bool is_ok;
	BitMap *bmp = create_bitmap(10000);
	// 1w个bit
	bitmap_set_all(bmp);
	// 全部置1

	bitmap_reset(bmp, 3000);
	// 清除第3000位
	bitmap_reset(bmp, 6666);
	// 清除第6666位

	printf("first: %lu\n", bitmap_find_first(bmp, NULL));

	printf("bit3000: %d\n", bitmap_is_set(bmp, 3000));
	bitmap_set(bmp, 3000);
	printf("bit3000: %d\n", bitmap_is_set(bmp, 3000));

	printf("first: %lu ", bitmap_find_first(bmp, &is_ok));
	printf("is_ok = %s\n", is_ok ? "true" : "false");

	bitmap_set(bmp, 6666);
	printf("first: %lu ", bitmap_find_first(bmp, &is_ok));
	printf("is_ok = %s\n", is_ok ? "true" : "false");

	bitmap_destroy(bmp);
	free(bmp);
}

