#include <stdio.h>
#include <string.h>
#include <time.h>  

#ifdef __linux__
#include "getch.h"
const char *bin_file = "0";
const char *txt_file = "1.txt";
#else
#include <conio.h>
const char *bin_file = "D:\\管理员开户文件_二进制";
const char *txt_file = "D:\\管理员开户文件.txt";
#endif

#define ResetIn()	while (getchar() != '\n') {}

void admin(); //管理员开户 
void customer(); //顾客开户 
void research_customer(); //顾客查询
void reasearch_admin();  //超级管理员权限 
void admin_customer();  //管理员查询 
void admin_delete();    //管理员批量删除功能 

//管理员部分---定义管理员结构体 
typedef struct Admin
{
 	char Name[10];//名字 
 	char Password[16];//密码 
	int Account;//账号 
}Admin;

#define type Admin
#include "list.h"

int main()
{
	int a = 1,j = 0,i=0;
	char ch = 0;
	char sh = 0; 
	FILE *fp = NULL;//定义文件指针
	char xitongmima[10] = {'1','2','3','4','5','6'};//银行系统验证密码 
	char yanzhengpassword[10]; //验证密码； 
	
	Admin adm = {"", "", 100000}; // Admin结构体
	list L; // Admin链表
	list_init(&L);
	list_load(&L, bin_file);
	
	while (a!=0)
	{
		printf ("请输入验证密码\n");
		scanf ("%s",yanzhengpassword);
		a = strcmp ( xitongmima,yanzhengpassword);
	}
	a = 1; //初始化让 a=0再次使用 
	memset (yanzhengpassword,0,10); //清空文件回收理由 
	printf ("\n请输入您的姓名\n");
	scanf  ("%s",adm.Name);
	ResetIn();
	while(a!=0)
	{
		printf ("\n请输入您的密码\n");
		while  ((ch = getch()) != '\n')
		{
			adm.Password[i] = ch;
			putchar('*');
			i++;
		}
		i = 0;
		printf ("\n请重复您的密码\n");
		while  ((sh = getch()) != '\n')
		{
			yanzhengpassword[i] = sh;
			putchar('*');
			i++;
		}
		a = strcmp (adm.Password,yanzhengpassword);
	}

	printf ("\n审核通过,请牢记密码和账号\n");
	adm.Account = adm.Account + list_size(&L);
	printf("您申请的账号为：%d",adm.Account);

	list_push_back(&L, &adm);
	list_save(&L, bin_file);

	fp = fopen(txt_file, "w");
	if ( ! fp ) // 创建txt文件失败
		return 1;
	Admin *pa = list_first(&L);
	while ( pa != list_tail(&L) ) {
		//管理员文件写入
		fprintf(fp," 账号： %d\n 姓名： %s\n 密码： %s\n \n",pa->Account,pa->Name,pa->Password); 
		pa = list_next(&L, pa);
	}
	fclose(fp);
	return 0;
}
 
