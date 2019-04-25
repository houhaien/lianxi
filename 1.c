/*************************************************************************
> File Name: 1.c
> Author: houhaien 
> Mail: github.com/houhaien 
> Created Time: 2019年04月14日 星期日 19时30分16秒
************************************************************************/

#define PRINT_FONT_RED printf("\033[31m");
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>

char suru[1024];
int cd(char suru[1024]){
    char path[1024];
    char *start;
    char *end;
    int res;
    int n = 0;
    memset(path,'\0',sizeof(path));
    start = strchr(suru,' ');
    end = strchr(suru,'\n');
    if (!start || !end) {
        printf("error\n");
        return 1;
    }
    /*if(start='c' && end = 'd') {
        
    }*/
    strncpy(path, suru+3, end -start-1);
    res = chdir(path);
    if (res != 0) {
        printf("wrong path\n");
        return res;
    }
}
int main(void) {
    struct passwd *pwd;
    char hname[32];
    char buffer[1024];
    //PRINT_FONT_RED
    //printf("%s@%s:%s" , pwd -> pw_name,hname,buffer);
    //printf("\n");

    while(1){
        getcwd(buffer,sizeof(buffer));
        gethostname(hname,sizeof(hname));

        pwd = getpwuid(getuid());
        PRINT_FONT_RED
        printf("%s@%s:" , pwd -> pw_name,hname);
        printf("%s$", buffer);
        printf(" ");
        //PRINtf("\n");

        fgets (suru , 1024, stdin);
        if(!strcmp(suru,"exit\n")) {
            break;
        }
        cd(suru);

    }

    return 0;

}
