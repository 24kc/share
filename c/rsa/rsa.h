#ifndef _RSA_H_
#define _RSA_H_

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

typedef struct {
	QWORD N;
	QWORD L;
	QWORD E;
	QWORD D;
} RSA;

typedef struct {
	QWORD N;
	union {
		QWORD E;
		QWORD D;
	};
} RSA_Key, RSA_Pub, RSA_Pri;

RSA rsa_generate();
int rsa_init(RSA *rsa, DWORD p, DWORD q);
RSA_Pub rsa_pub_key(RSA*);
RSA_Pri rsa_pri_key(RSA*);
QWORD rsa_encrypt(QWORD p, RSA_Key *key);
QWORD rsa_decrypt(QWORD c, RSA_Key *key);
BYTE* rsa_encrypt_all(void*, int, int*, RSA_Key *key);
BYTE* rsa_decrypt_all(void*, int, int*, RSA_Key *key);

QWORD krand();
QWORD lcm(QWORD, QWORD);
QWORD gcd(QWORD, QWORD);
QWORD inter_prime_small(QWORD);
QWORD mul_inv_mod(QWORD a, QWORD p);
int is_prime_fool(QWORD);
int is_prime_32b(DWORD);
DWORD generate_prime_32b(DWORD max);
int binary_search(DWORD *a, int size, DWORD val);
int64_t ext_euclid(QWORD d, QWORD f);
QWORD exponent_mod(QWORD a, QWORD n, QWORD p);

RSA
rsa_generate()
{
	DWORD max = 0xffff;
	DWORD N_min = 0xffffff;
	DWORD p, q;
	do {
		p = generate_prime_32b(max);
		q = generate_prime_32b(max);
	} while ( p * q <= N_min );
	RSA rsa;
	rsa_init(&rsa, p, q);
	return rsa;
}

int
rsa_init(RSA *rsa, DWORD p, DWORD q)
{
	rsa->N = p * q;
	rsa->L = lcm(p-1, q-1);
	rsa->E = inter_prime_small(rsa->L);
	rsa->D = mul_inv_mod(rsa->E, rsa->L);
	return 1;
}

RSA_Pub
rsa_pub_key(RSA *rsa)
{
	RSA_Pub pub;
	pub.N = rsa->N;
	pub.E = rsa->E;
	return pub;
}

RSA_Pri
rsa_pri_key(RSA *rsa)
{
	RSA_Pri pri;
	pri.N = rsa->N;
	pri.D = rsa->D;
	return pri;
}

int64_t ext_euclid(QWORD d, QWORD f)
{
	int64_t x1, x2, x3,
	y1, y2, y3,
	t1, t2, t3,
	q;

	x1 = y2 = 1;
	x2 = y1 = 0;
	x3 = d>=f ? d : f;
	y3 = d>=f ? f : d;

	for (;;) {
//		if ( ! y3 )
//			return 0;
		if ( y3 == 1 )
			return y2;
		q = x3 / y3;
		t1 = x1 - q * y1;
		t2 = x2 - q * y2;
		t3 = x3 - q * y3;
		x1 = y1;
		x2 = y2;
		x3 = y3;
		y1 = t1;
		y2 = t2;
		y3 = t3;
	}
	return 0;
}

QWORD
inter_prime_small(QWORD n)
{
	QWORD m = n - 2;
	QWORD r = ((krand() % m) & 0xffffffff) + 2;
	for (QWORD i=r; i>1; --i)
		if ( gcd(i, n) == 1 )
			return i;
	return n-1;
}

QWORD
mul_inv_mod(QWORD a, QWORD p)
{
	int64_t x = ext_euclid(a, p);
	if ( x < 0 )
		x += p;
	return x % p;
}

QWORD
rsa_encrypt(QWORD p, RSA_Key *key)
{
	return exponent_mod(p, key->E, key->N);
}

QWORD
rsa_decrypt(QWORD c, RSA_Key *key)
{
	return exponent_mod(c, key->D, key->N);
}

QWORD
lcm(QWORD a, QWORD b)
{
	return a / gcd(a,b) * b;
}

QWORD
gcd(QWORD a, QWORD b)
{
	QWORD r;
	while ( (r = a % b) )
		a = b, b = r;
	return b;
}

QWORD
krand()
{
	static int flag = 0;
	if ( ! flag ) {
		srand(time(NULL));
		flag = 1;
	}
	BYTE b[8];
	for (int i=0; i<8; ++i)
		b[i] = rand() & 0xff;
	return *(QWORD*)b;
}

int
is_prime_32b(DWORD n)
{
	if ( n <= 2 ) {
		if ( n < 2 )
			return 0;
		return 1;
	}
#define SIZE ( 6542 + 1 )
	static int flag = 0;
	static DWORD a[SIZE];
	if ( ! flag ) {
		int index = 0;
		for (int i=0; i<0x10000; ++i)
			if ( is_prime_fool(i) )
				a[index++] = i;
		a[index] = 0x10000;
		flag = 1;
	}
	QWORD q = sqrt(n) + 1;
	int end = binary_search(a, SIZE, q);
	for (int i=0; i<=end; ++i)
		if ( n % a[i] == 0 )
			return 0;
	return 1;
#undef SIZE
}

int
is_prime_fool(QWORD n)
{
	if ( n <= 2 ) {
		if ( n < 2 )
			return 0;
		return 1;
	}
	QWORD q = sqrt(n) + 1;
	for (QWORD i=2; i<=q; ++i)
		if ( n % i == 0 )
			return 0;
	return 1;
}

int
binary_search(DWORD *a, int size, DWORD val)
{
// a[index] <= val < a[index+1]
	int left = 0;
	int right = size - 1;
	int index;
	for (;;) {
		index = (left + right) >> 1;
		if ( a[index] < val )
			left = index + 1;
		else
			right = index - 1;
		if ( left >= right )
			break;
	}
	return left;
}

DWORD
generate_prime_32b(DWORD max)
{
	if ( ! max )
		max = 0xffffffff;
	DWORD r = krand() & max;
	for (DWORD i=r; i<=max; ++i)
		if ( is_prime_32b(i) )
			return i;
	for (DWORD i=r-1; i>0; --i)
		if ( is_prime_32b(i) )
			return i;
	return 0;
}

QWORD
exponent_mod(QWORD a, QWORD n, QWORD p)
{
	QWORD r = 1, b = a;
	while ( n ) {
		if ( n & 1 )
			r = (r * b) % p;
		b = (b * b) % p;
		n >>= 1;
	}
	return r;
}

BYTE*
rsa_encrypt_all(void *_data, int size, int *p_size, RSA_Key *key)
{
	BYTE *data = (BYTE*)_data;
	int n = size / 3;
	int r = size % 3;
	int bufsiz = (n+1) * 4;
	DWORD *buf = (DWORD*)malloc(bufsiz);

	DWORD d = 0;
	BYTE *b = (BYTE*)&d;
	for (int i=0; i<n; ++i) {
		memcpy(b, data+i*3, 3);
		buf[i] = rsa_encrypt(d, key);
	}
	memcpy(b, data+n*3, r);
	memset(b+r, 3-r, 3-r);
	buf[n] = rsa_encrypt(d, key);
	*p_size = bufsiz;
	return (BYTE*)buf;
}

BYTE*
rsa_decrypt_all(void *_data, int size, int *p_size, RSA_Key *key)
{
	assert( ! (size%4) );
	DWORD *data = (DWORD*)_data;
	int n = size / 4;
	int bufsiz = n * 3;
	BYTE *buf = (BYTE*)malloc(bufsiz);

	DWORD d = 0;
	BYTE *b = (BYTE*)&d;
	for (int i=0; i<n; ++i) {
		d = rsa_decrypt(data[i], key);
		memcpy(buf+i*3, b, 3);
	}
	int r = buf[bufsiz - 1];
	assert(0<r && r<=3);
	*p_size = bufsiz - r;
	return buf;
}

#endif

