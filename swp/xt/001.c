#include<stdio.h>
int main()
{
	int i,j,k;
	i=j=k=0;
	int sum=0;
	int b[20];
	int g[20];
	int a[20]={23,12,77,69,32,88,18,24,76,81,13,27,25,56,78,10,36,60,58,64};
	for(;i<20;i++)
	{
		if(a[i]%10-a[i]/10==1)
		{
			sum+=a[i];
			b[j]=a[i];
			j++;
			
		}
		else
		{
			g[k]=a[i];
			printf("%d ",g[k]);
			k++;
		}
	}
	printf("\n其和为:%d \n",sum);
}
