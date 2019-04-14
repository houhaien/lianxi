/*************************************************************************
	> File Name: 1.c
	> Author: houhaien 
	> Mail: github.com/houhaien 
	> Created Time: 2019年04月14日 星期日 19时30分16秒
 ************************************************************************/

#include <stdio.h>
#include <pwd.h>

#include <unistd.h>
int main(void) {
    struct passwd *pwd;
    char hname[32];
    char buffer[1024];
    getcwd(buffer,sizeof(buffer));
    gethostname(hname,sizeof(hname));

    pwd = getpwuid(getuid());
    printf("%s@%s:%s" , pwd -> pw_name,hname,buffer);
    printf("\n");
    return 0;

}
