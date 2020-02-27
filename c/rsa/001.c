#include <stdio.h>
#include <string.h>
#include "rsa.h"

typedef struct {
	char str[16];
	int val;
} Arg;

#define _keygen  (0x2410)
#define _key  (0x2420)
#define _in  (0x2430)
#define _out  (0x2440)
#define _e  (0x2450)
#define _d  (0x2460)
#define _help  (0x100)

Arg args[] = {
{"-keygen", _keygen},
{"-k", _key},
{"-key", _key},
{"-in", _in},
{"-out", _out},
{"-e", _e},
{"-d", _d},
{"-h", _help},
{"-help", _help},
{"", 0}
};

enum {ENCRYPT=_e, DECRYPT=_d};

void help(const char*);
int arg_get_val(Arg*, const char*);
int generate_key(const char*);
RSA_Key load_key(const char*);
BYTE* fread_all(const char*, int*);
int fwrite_all(const char*, void*, int);

int main(int argc, char **argv)
{
	int mode = ENCRYPT;
	const char *keygen=NULL, *fkey=NULL, *in=NULL, *out=NULL;
	int optind = 0;

	while ( ++optind < argc ) {
		const char *optstr = argv[optind];
		int opt = arg_get_val(args, optstr);

		if ( ! opt ) {
			printf("unrecognized command line option \'%s\'\n", optstr);
			return 1;
		}

		switch ( opt ) {
		case _e: case _d:
			mode = opt;
			break;
		case _in:
			if ( ++optind < argc )
				in = argv[optind];
			else
				opt = -1;
			break;
		case _out:
			if ( ++optind < argc )
				out = argv[optind];
			else
				opt = -1;
			break;
		case _help:
			help(argv[0]);
			return 0;
		case _key:
			if ( ++optind < argc )
				fkey = argv[optind];
			else
				opt = -1;
			break;
		case _keygen:
			if ( ++optind < argc )
				keygen = argv[optind];
			else
				opt = -1;
			break;
		
		default:
			printf("getopt error\n");
			return 1;
		}

		if ( opt < 0 ) {
			printf("missing argument after \'%s\'\n", optstr);
			return 1;
		}
	}

	if ( keygen ) {
		generate_key(keygen);
		return 0;
	}

	if ( ! fkey ) {
		printf("missing key file, need -k/-key option\n");
		return 1;
	}

	if ( ! in || ! out ) {
		printf("missing input/output file, need -in/-out option\n");
		return 1;
	}

	RSA_Key key = load_key(fkey);
//	printf("%lu %lu\n", key.N, key.E);

	int size;
	BYTE *b = fread_all(in, &size);
	if ( mode == ENCRYPT )
		b = rsa_encrypt_all(b, size, &size, &key);
	else
		b = rsa_decrypt_all(b, size, &size, &key);
	fwrite_all(out, b, size);
}

RSA_Key
load_key(const char *fname)
{
	RSA_Key key = {0, {0}};
	FILE *fp = fopen(fname, "r");
	if ( ! fp )
		return key;

	fscanf(fp, "%*[^0-9]%lu%*[^0-9]%lu", &key.N, &key.E);
	fclose(fp);

	return key;
}

int
generate_key(const char *fname)
{
	const char *suffix = ".pub";
	char *fname_pub = (char*)malloc(strlen(fname)+strlen(suffix)+1);
	strcpy(fname_pub, fname);
	strcat(fname_pub, suffix);

	FILE *fpub, *fpri;
	fpub = fopen(fname_pub, "w");
	fpri = fopen(fname, "w");
	if ( ! fpub || ! fpri )
		return 0;

	RSA rsa = rsa_generate();
	RSA_Key pubk, prik;
	pubk = rsa_pub_key(&rsa);
	prik = rsa_pri_key(&rsa);

	fprintf(fpub, "---BEGIN RSA PUBLIC KEY---\n[N=%lu, E=%lu]\n---END RSA PUBLIC KEY---\n", pubk.N, pubk.E);
	fprintf(fpri, "---BEGIN RSA PRIVATE KEY---\n[N=%lu, D=%lu]\n---END RSA PRIVATE KEY---\n", prik.N, prik.D);
	fclose(fpub);
	fclose(fpri);

	free(fname_pub);
	return 1;
}

BYTE*
fread_all(const char *fname, int *p_size)
{
	BYTE *buf = NULL;
	FILE *fp = fopen(fname, "rb");
	if ( ! fp )
		return buf;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = (BYTE*)malloc(size);
	fread(buf, size, 1, fp);
	*p_size = size;

	fclose(fp);
	return buf;
}

int
fwrite_all(const char *fname, void *buf, int size)
{
	FILE *fp = fopen(fname, "wb");
	if ( ! fp )
		return 0;

	int r = fwrite(buf, size, 1, fp);

	fclose(fp);
	return r;
}

int
arg_get_val(Arg *args, const char *s)
{
	Arg *arg = args;
	while ( *arg->str ) {
		if ( strcmp(arg->str, s) == 0 )
			return arg->val;
		++arg;
	}
	return 0;
}

void
help(const char *cmd)
{
	printf("Usage:    %s [-in IN_FILE -out OUT_FILE]\n", cmd);
	printf(
"Valid options are:\n"
" -e                Encrypt\n"
" -d                Decrypt\n"
" -in in_file       Input file\n"
" -out out_file     Output file\n"
" -k/-key key       Specifying the key file\n"
" -keygen file      Generate key file\n"
" -h/-help          Display this message\n"
	);
}

