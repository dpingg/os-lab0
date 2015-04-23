#include "common.h"
#include "string.h"


void getHex(char* arr, int n){
	uint32_t num=(uint32_t)n;
	
	char start[10];
	char *tmp=start;


	if(num==0) {
		*arr='0';
		*(arr+1)=0;
		return;
	}

	while(num) {
		switch (num%16) {
			case 10: *tmp++ = 'a'; break;
			case 11: *tmp++ = 'b'; break;
			case 12: *tmp++ = 'c'; break;
			case 13: *tmp++ = 'd'; break;
			case 14: *tmp++ = 'e'; break;
			case 15: *tmp++ = 'f'; break;
			default: *tmp++ = ('0'+num%16); break;
		}
		num/=16;
	}

	tmp--;
	while(tmp != start){
		*arr = *tmp;
		arr++;
		tmp--;
	}
	*arr++=*tmp;
	*arr = 0;
}

void getDec(char* arr, int n){
	uint32_t num=(uint32_t)n;
	
	char start[40];
	char *tmp=start;

	if(n>=0) num=(uint32_t)n;
	else num=(uint32_t)((~n)+1);

	if(num==0) {
		*arr='0';
		*(arr+1)=0;
		return;
	}

	while(num) {
		*tmp++ = ('0'+num%10);
		num/=10;
	}
	if(n<0) *tmp++ = '-';

	tmp--;
	while(tmp != start){
		*arr = *tmp;
		arr++;
		tmp--;
	}
	*arr++=*tmp;
	*arr = 0;
}

/* implement this function to support printk */
void vfprintf(void (*printer)(char), const char *ctl, void **args) {
	const char *str = __FUNCTION__;
	int flag=0;
//	for(;*str != '\0'; str ++) printer(*str);

	//str = ": vfprintf() is not implemented!\n";
	while(*ctl) {
		switch(*ctl) {
			case '%':
				if(flag==1) {
					printer('%');
					flag=0;
				}
				else {
					flag=1;
				}
				break;
			case 'd':
				if(flag==1) {
					char d[40];
					char *ch;
					getDec(d, *(int *)args);
					for(ch=d; *ch != '\0'; ch ++) printer(*ch);
					args++;
					flag=0;
				}
				else {
					printer('d');
				}
				break;
			case 's':
				if(flag==1) {
					str=*(char **)args;
					for(;*str != '\0'; str ++) printer(*str);
					args++;
					flag=0;
				}
				else {
					printer('s');
				}
				break;
			case 'c':
				if(flag==1) {
					printer(*(char *)args);
					args++;
					flag=0;
				}
				else {
					printer('c');
				}
				break;
			case 'x':
				if(flag==1) {
					char x[10];
					char *ch;
					getHex(x, *(int *)args);
					for(ch=x; *ch != '\0'; ch ++) printer(*ch);
					args++;
					flag=0;
				}
				else {
					printer('x');
				}
				break;
			default:
				printer(*ctl);
				break;

		}
		ctl++;
	}
	
}



extern void serial_printc(char);

/* __attribute__((__noinline__))  here is to disable inlining for this function to avoid some optimization problems for gcc 4.7 */
void __attribute__((__noinline__))  printk(const char *ctl, ...) {
	void **args = (void **)&ctl + 1;
	vfprintf(serial_printc, ctl, args);
}
