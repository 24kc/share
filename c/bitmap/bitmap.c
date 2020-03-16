#include "bitmap.h"

BitMap*
bitmap_init(BitMap *self, QWORD n) {
	self->qsize = (n>>6) + ((n&0x3f) ? 1 : 0);
	self->data = (QWORD*)malloc(self->qsize << 3);
	if ( ! self->data )
		return NULL;
	self->size = n;
	return self;
}

void
bitmap_clear(BitMap *self)
{
	memset(self->data, 0, self->qsize << 3);
}

void
bitmap_set_all(BitMap *self)
{
	memset(self->data, 0xff, self->qsize << 3);
}

void
bitmap_destroy(BitMap *self)
{
	free(self->data);
	self->size = 0;
}

BitMap*
create_bitmap(QWORD n)
{
	BitMap *bmp = (BitMap*)malloc(sizeof(BitMap));
	if ( ! bmp || ! bitmap_init(bmp, n) )
		return NULL;
	bitmap_clear(bmp);
	return bmp;
}

bool
bitmap_is_set(BitMap *self, QWORD pos)
{
	QWORD q = ((QWORD)1) << (pos&0x3f);
	return (self->data[pos>>6] & q) ? true : false;
}

void
bitmap_set(BitMap *self, QWORD pos)
{
	QWORD q = ((QWORD)1) << (pos&0x3f);
	self->data[pos>>6] |= q;
}

void
bitmap_reset(BitMap *self, QWORD pos)
{
	QWORD q = ((QWORD)1) << (pos&0x3f);
	self->data[pos>>6] &= ~q;
}

QWORD
qword_lowest_bit(QWORD q) // 0 ~ 63
{
	QWORD n = 63;
	if (q & 0x00000000ffffffff) {
		n -= 32;
		q &= 0x00000000ffffffff;
	}
	if (q & 0x0000ffff0000ffff) {
		n -= 16;
		q &= 0x0000ffff0000ffff;
	}
	if (q & 0x00ff00ff00ff00ff) {
		n -= 8;
		q &= 0x00ff00ff00ff00ff;
	}
	if (q & 0x0f0f0f0f0f0f0f0f) {
		n -= 4;
		q &= 0x0f0f0f0f0f0f0f0f;
	}
	if (q & 0x3333333333333333) {
		n -= 2;
		q &= 0x3333333333333333;
	}
	if (q & 0x5555555555555555) {
		n -= 1;
	}
	return n;
}

QWORD
qword_highest_bit(QWORD q) // 0 ~ 63
{
	QWORD n = 0;
	if (q & 0xffffffff00000000) {
		n += 32;
		q &= 0xffffffff00000000;
	}
	if (q & 0xffff0000ffff0000) {
		n += 16;
		q &= 0xffff0000ffff0000;
	}
	if (q & 0xff00ff00ff00ff00) {
		n += 8;
		q &= 0xff00ff00ff00ff00;
	}
	if (q & 0xf0f0f0f0f0f0f0f0) {
		n += 4;
		q &= 0xf0f0f0f0f0f0f0f0;
	}
	if (q & 0xcccccccccccccccc) {
		n += 2;
		q &= 0xcccccccccccccccc;
	}
	if (q & 0xaaaaaaaaaaaaaaaa) {
		n += 1;
	}
	return n;
}

QWORD
bitmap_find_first(BitMap *self, bool *is_ok)
{
	QWORD i, res;
	for (i=0; i<self->qsize; ++i)
		if ( self->data[i] ^ 0xffffffffffffffff )
			break;
	if (i == self->qsize) {
		if (is_ok)
			*is_ok = false;
		return i;
	}
	res = (i<<6) + qword_lowest_bit( ~ self->data[i] );
	if (is_ok) {
		if ( res < self->size )
			*is_ok = true;
		else
			*is_ok = false;
	}
	return res;
}

