#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t QWORD;

typedef struct BitMap {
	QWORD *data;
	QWORD qsize; // QWORD array size
	QWORD size; // bit num;
}BitMap;

BitMap* create_bitmap (QWORD n);
// 创建一个有n个bit的bitmap, 返回指向该bitmap的指针(需要自行free), 失败返回NULL
BitMap* bitmap_init (BitMap *bmp, QWORD n);
// 初始化结构体bmp, 使其拥有n个bit, 成功返回bmp指针, 失败返回NULL
// TODO: 以上2个函数不对初始bitmap各个bit的值作任何保证

void bitmap_destroy (BitMap *bmp);
// 释放bmp的内存

void bitmap_clear (BitMap*);
// 所有bit置0
void bitmap_set_all (BitMap*);
// 所有bit置1

// TODO: 所有pos从0开始
bool bitmap_is_set (BitMap*, QWORD pos);
// 判断第pos个bit是否为1
void bitmap_set (BitMap*, QWORD pos);
// 将第pos个bit置为1
void bitmap_reset (BitMap*, QWORD pos);
// 将第pos个bit置为0
QWORD bitmap_find_first (BitMap*, bool *is_ok);
// 寻找第一个为0的bit位置
	// 成功: 设置is_ok为true, 返回bit位置
	// 失败(所有bit均为1): 设置is_ok为false, 返回值无效
	// 如果确信存在为0的bit, 则is_ok可以为NULL

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _BITMAP_H_

