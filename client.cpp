#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
using namespace std;

int s;
int ds;
char addr[]="8.8.8.8"; //надо заменить
 char host[] = "hostname.ru";

int readServ() {
    int rc;
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(s,&fdr);
    timeval timeout;
    timeout.tv_sec = 1;   ///зададим  структуру времени со значением 1 сек
    timeout.tv_usec = 0;  
    do {
        char buff[512] ={' '};
        recv(s,&buff,512,0);   ///получаем данные из потока
        cout << buff;
        rc = select(s+1,&fdr,NULL,NULL,&timeout);    
    } while(rc);     ///проверяем результат
    return 2;
}

int init_sock() {
    int len;
    sockaddr_in address;
    int result;
    int s;
    s = socket(AF_INET, SOCK_STREAM,0);
    address.sin_family = AF_INET;   ///интернет домен
    address.sin_addr.s_addr = inet_addr(addr);   ///соединяемся 
    address.sin_port = htons(2121);    /// 21 порт
    len = sizeof(address);
    result = connect(s, (sockaddr *)&address, len);   ///установка соединения
    if (result == -1) {
        perror("oops: client");
        return -1;
    }
    return s;
}

int init_data() {
    send(s,"PASV\r\n",strlen("PASV\r\n"),0);
    char buff[128];
    recv(s,buff,128,0);
    cout << buff; ////выводим на экран полученную от сервера строку
    int a,b;
    char *tmp_char;
    tmp_char = strtok(buff,"(");
    tmp_char = strtok(NULL,"(");
    tmp_char = strtok(tmp_char, ")");
    int c,d,e,f;
    sscanf(tmp_char, "%d,%d,%d,%d,%d,%d",&c,&d,&e,&f,&a,&b);
    int len;
    sockaddr_in address;
    int result;
    int port = a*256 + b;
    ds = socket(AF_INET, SOCK_STREAM,0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(addr);    ///addr - у меня глобальная переменная с адресом сервера
    address.sin_port = htons(port);
    len = sizeof(address);
    result = connect(ds, (sockaddr *)&address, len);
    if (result == -1) {
        perror("oops: client");
        return -1;
    }
    return 0;
}

int login() {
    char name[]="";
    char str[512];
    sprintf(str,"USER %s\r\n",name);
    send(s,str,strlen(str),0);
    readServ();
    char pass[]="";
    sprintf(str,"PASS %s\r\n",pass);
    send(s,str,strlen(str),0);
    readServ();
    return 0;
}

int get(char *file) {
    char str[512];
    sprintf(str,"RETR %s\r\n",file);
    send(s,str,strlen(str),0);
 
    /* получение размера файла */
    char size[512];
    recv(s,size,512,0);
    cout << size;
 
    char *tmp_size;
    tmp_size = strtok(size,"(");
    tmp_size = strtok(NULL,"(");
    tmp_size = strtok(tmp_size, ")");
    tmp_size = strtok(tmp_size, " ");
 
    int file_size;

    sscanf(tmp_size,"%d",&file_size);
    FILE *f;
    file_size = 0x527;
    cout << "file size = " <<  file_size << endl;
    f = fopen(file, "wb");   ///важно чтобы файл писался в бинарном режиме

    int read = 0;  ///изначально прочитано 0 байт
    do {
            //cout << "1";
            char buff[2048];   ////буфе для данных
            int readed = recv(ds,buff,sizeof(buff),0);   ///считываем данные с сервера. из сокета данных
            fwrite(buff,1,readed,f);   ///записываем считанные данные в файл
            read += readed;  ///увеличиваем количество скачанных данных
        } while (read < file_size);
    fclose(f);
    cout << "Готово. Ожидание ответа сервера...\n";
    return 0;
}



int main() {
   

    s = init_sock();
    readServ();
    login();
    init_data();
    char file[]="123.dat";
    get(file);
    close(s);  ///закрытие соединения
    return 0;
}