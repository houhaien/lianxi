/*************************************************************************
	> File Name: vsprintf.cpp
	> Author: houhaien 
	> Mail: github.com/houhaien 
	> Created Time: 2019年05月03日 星期五 13时44分17秒
 ************************************************************************/

#include <iostream>
#include <cstdio>
#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
using namespace std;

// 简单的字符转换
// strtol函数(字符串起始地址， 返回字符串有效数字的结尾地址，转化基数)
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;
    //base = 0 时默认十进制，base = '0' 时代表八进制，base = 'x' 且判断cp[1] 来确定是否是16进制
	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((*cp == 'x') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	}
    //用isxdigit（）来判断cp指向的是否为16进制数 和10进制数，如果是就转化为10进制数赋予value ，
    //否则就用islower判断小写英文字母，用toupper将小写变为大写再根据相应规则比较
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {   
		result = result*base + value;
		cp++;
	}
    //若 endptr 不为NULL，则会将遇到的不符合条件而终止的字符指针由 endptr 传回；
    //否则不使用该参数
	if (endp)
		*endp = (char *)cp;
	return result;
}

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')
//判断一个字符是否在0~9之间
static int skip_atoi(const char **s)
//将数字字符串转换成相应的数字，并且跳过现在的字符串
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';									// *((*s)++) ==>s[0][i]++ == s[0][i+1]
	return i;
}
//给相应的类型复制，后面读取到相应的二进制就给对应？赋予该类型
#define ZEROPAD	1		/* pad with zero 填充0*/
#define SIGN	2		/* unsigned/signed long  无符号或有符号长整数*/
#define PLUS	4		/* show plus（加） */
#define SPACE	8		/* space if plus 加空格*/
#define LEFT	16		/* left justified 左对齐*/
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' 大小写字母代表同一个意思*/

// n 除以base然后存入n ， 余数存入_res并返回。相应的运算方法请查看汇编（^~^ 我只知道代表什么意思）
#define do_div(n,base) ({ \
int __res; \
__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
__res; })

// 传入的参数分别为 目标字符串， %后面对应的可变参数， 采用的进制， 输出的位数， 
// 精度，类型（是一个二进制数，每一位都代表特有的含义） 
static char * number(char * str, int num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz"; 	// 是否是小写
	if (type&LEFT) type &= ~ZEROPAD; 								// 左对齐是填写‘0’
	if (base<2 || base>36) 											// 判断进制
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;								// 补 ‘0’ 还是空格
	if (type&SIGN && num<0) {  										// 负数输出
		sign='-';
		num = -num;  
	} else
		sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);  		// 正数
	if (sign) size--; 												// 判断是否带有符号是的话就-1
	if (type&SPECIAL) 												// 判断是否有转换限定符
		if (base==16) size -= 2; 									// 16进制-2
		else if (base==8) size--; 									// 8进制-1
	i=0;
	if (num==0) 													// 0 的输出
		tmp[i++]='0';
	else while (num!=0) 											// 不是0的话就一直对num进行除法运算，余数写入tmp[]
		tmp[i++]=digits[do_div(num,base)];
	if (i>precision) precision=i; 									// 精度  ：输出的位数
	size -= precision;     
	if (!(type&(ZEROPAD+LEFT))) 									// 非补零和左对齐就用空格
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type&SPECIAL) 												// 8进制和 16进制 前缀补0
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type&LEFT)) // 
		while(size-->0) 											// 当size大于0时循环，size进行自减运算
			*str++ = c;
	while(i<precision--) 											// 如果输出的小于精度 末尾 + 0
		*str++ = '0';
	while(i-->0) 													// 把数组tmp中的值从 i 开始赋值给str
		*str++ = tmp[i];
	while(size-->0) 												// 如果没有达到设定的宽度 ，就补空格
		*str++ = ' ';
	return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)				// fmt格式说明符号，buf 带写入数据
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		/* flags to number() 一个指向number（）的标志*/

	int field_width;	/* width of output field 输出字段宽度*/
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
    //
	for (str=buf ; *fmt ; ++fmt) {
        //找 % 后面的格式字符
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags 过程标志*/
		flags = 0;
		repeat: // 这一部分是为了判断输出的格式如“% d”等
			++fmt;		/* this also skips first '%' */
                //控制字符的格式
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
                //    #        对c、s、d、u类无影响；
                //            对o类（八进制），在输出时加前缀o；
                //            对x类（16进制），在输出时加前缀0x；
                //            对e（按指数形式的浮点数的格式输出）、g（自动选择合适的表示法输出）、f （按浮点数的格式输出）类当结果有小数时才给出小数点。 
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		/* get field width */
		field_width = -1;                         	// 初始宽度为-1，表示没有精度制约
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {                   	// 当下一个参数是*号时 应根据传入的实参确定精度
			/* it's the next argument */
			field_width = va_arg(args, int);      	// 检索下一个参数，类型为int 赋予  field_width    
			if (field_width < 0) {                	// 例如：printf("%*d", -2,1);‘-’代表左对齐，位数不够空格补
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision 获取精度*/
		precision = -1;
		if (*fmt == '.') {                       	// 解决printf("%5.2d",n);问题
			++fmt;	
			if (is_digit(*fmt))                  	// 判断下一个参数是否是十进制
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {              	// 当下一个参数是*号时 应根据传入的实参确定精度
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)                    	// 精度为负，默认为0
				precision = 0;
		}

		/* get the conversion qualifier  获取转换限定符*/
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') { // 检索所有的fmt
			qualifier = *fmt;
			++fmt;
		}

		switch (*fmt) {
		case 'c'://字符
			if (!(flags & LEFT)) //没有left
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)//有 left
				*str++ = ' ';
			break;

		case 's': 	//字符串
			s = va_arg(args, char *);
			if (!s) //为空
				s = "<NULL>";
			len = strlen(s); 					// 根据精度precision来确定长度
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT)) 				//判断是否左对齐
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--) 		// 不等宽补空格
				*str++ = ' ';
			break;

		case 'o':   //八进制
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags); //把现在的str转换成8进制
			break;

		case 'p':   //%p 输出指针的值
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags); //转16进制
			break;

		case 'x':   // %x 与 %X 等同 
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':   // 进入端口 从%d  %i 有符号数整数   
		case 'i':
			flags |= SIGN;
		case 'u':   // %u 无符号数整数
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'n':   //  %n不向printf传递格式化信息，而是令printf把自己到该点已打出的
					// 字符总数放到相应变元指向的整形变量中
			ip = va_arg(args, int *);
			*ip = (str - buf);       //  ip = str地址 - buf首地址
			break;

		default:  
			if (*fmt != '%')         //如果*fmt 不是% 则写入
				*str++ = '%';
			if (*fmt)                //fmt 不空则写入str
				*str++ = *fmt;      
			else                     
				--fmt;               // 到了最后结尾
			break;
		}
	}
	*str = '\0';                     //终止进程
	return str-buf;                    // 输出结果的长度
}

int sprintf(char * buf, const char *fmt, ...)
{   
    //用于检索一个变参函数args的其他参数
	va_list args;
	int i;
    //va_start宏，获取可变参数列表的第一个参数的地址 va_arg 检索下一个参数
	va_start(args, fmt);////将获取到的fmt格式字符串写入到buf这个缓存里去
	i=vsprintf(buf,fmt,args);
    //va_end宏，清空va_list可变参数列表：
	va_end(args);
	return i;
}

