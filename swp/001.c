#include <stdio.h>
#include <stdlib.h>

int main()
{
	int i,j,n,t,z;
	printf("请输入数组个数: ");
	scanf("%d",&n);
	int *a = (int*)malloc(sizeof(int) * (n+1)); // int a[n+1]
	printf("输入数组: ");
	for(i=0;i<n;i++)
		scanf("%d",&a[i]);
	//a[n]=0;
	for(i=0;i<n-1;i++)
	{
		for(j=0;j<n-i-1;j++)
		{
			if(a[j]>a[j+1])
			{
				t=a[j],a[j]=a[j+1],a[j+1]=t;
			}
		}
	}
	printf("排序后的数组: ");
	for(i=0;i<n;i++)
		printf("%d ",a[i]);
	printf("\n");
	printf("请输入要插入的数字: ");
	scanf("%d",&z);
	for(i=0;i<n;i++)
		if(z < a[i])
			break;
	for (j=n; j>i; --j)
		a[j] = a[j-1];
	a[i] = z;
	printf("插入后的数组: ");
	for(i=0;i<n+1;i++)
		printf("%d ",a[i]);
	printf("\n");
}
