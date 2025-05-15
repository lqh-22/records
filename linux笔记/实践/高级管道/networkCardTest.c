#include <stdio.h>

char buffer[2048];
int main(int argc, char const *argv[]){
    FILE *fp = popen("ifconfig", "r");
    fread(buffer, sizeof(char), sizeof(buffer), fp);
    fprintf(stdout, "Network card information:\n%s", buffer);
    pclose(fp);
    return 0;
}