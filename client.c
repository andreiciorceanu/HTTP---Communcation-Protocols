//324CA Ciorceanu Andrei Razvan
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"


int main(int argc, char *argv[])
{
    char *message; // mesajul trimis la server
    char *response; // raspunsul de la server
    int sockfd;
    char *buff = (char *) calloc(BUFLEN, sizeof(char));
    char *cookie = (char *) calloc(BUFLEN, sizeof(char));
    char *token = (char *) calloc(BUFLEN, sizeof(char));
    char *aux = (char *) calloc(BUFLEN, sizeof(char));
    DIE(!aux, "alocare buffer auxiliar");
    DIE(!buff, "alocare buffer");
    DIE(!cookie, "alocare cookie");
    DIE(!token, "alocare token");
    while(1) {
        fgets(buff, BUFLEN - 1, stdin); // citim comanda de la stdin

        if(!strcmp(buff, "register\n")) { // daca am introdus register
            // alocam memorie pentru user si parola
            char **login_data = (char **) calloc(2, sizeof(char *));
            login_data[0] = (char *) calloc(50, sizeof(char));
            login_data[1] = (char *) calloc(50, sizeof(char));
            char *error_json = (char *) calloc(BUFLEN, sizeof(char));
            char *req = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!req, "alocare req");
            DIE(!error_json,"alocare vector de erori");
            DIE(!login_data, "alocare user/passwd");
            DIE(!login_data[0], "alocare username");
            DIE(!login_data[1], "alocare password");
            // deschidem conexiunea cu serverul
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // citim de la tastatura username-ul si password
            printf("username=");
            fgets(login_data[0], BUFLEN - 1, stdin);
            printf("password=");
            fgets(login_data[1], BUFLEN - 1, stdin);
            login_data[0][strlen(login_data[0]) - 1] = '\0'; // punem terminatorul de sir pentru cele 2 stringuri
            login_data[1][strlen(login_data[1]) - 1] = '\0';
            // cream obiectul de tip JSON
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            // cream string-ul care va fi parsat cu json
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", login_data[0]);
            json_object_set_string(root_object, "password", login_data[1]);
            serialized_string = json_serialize_to_string_pretty(root_value);
            printf("\n");
            printf("REQUEST\n\n");
            // facem request-ul catre server
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/register", "application/json", serialized_string, NULL, NULL);
            puts(message);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("ANSWER\n\n");
            error_json = strstr(response, "error");
            req = strstr(response, "Too many requests");
            if(error_json != NULL) { //if JSON received is type of {"error:""..."}
                printf("[ERROR] User already used by someone else\n");
            } else if (req != NULL) { //if too many requests on the server
                printf("[ERROR] Too many requests, please try again later.\n");
            } else { //if command executed correctly
                printf("[SUCCES] Registered in with username \"%s\" and password \"%s\"\n", login_data[0], login_data[1]);
            }
            //eliberam memoria
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            free(login_data[0]);
            free(error_json);
            free(req);
            free(login_data[1]);
            free(login_data);
            // inchidem conexiunea cu serverul
            close(sockfd);
        }

        if(!strcmp(buff, "login\n")) {
            // alocam memorie pentru stringurile in care pastram
            // user si parola pentru login si stringurile folosite
            // la parsarea erorilor
            char **login_data = (char **) calloc(2, sizeof(char *));
            login_data[0] = (char *) calloc(50, sizeof(char));
            login_data[1] = (char *) calloc(50, sizeof(char));
            char *error_json = (char *) calloc(BUFLEN, sizeof(char));
            char *req = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!req, "alocare req");
            DIE(!error_json,"alocare vector de erori");
            DIE(!login_data, "alocare user/passwd");
            DIE(!login_data[0], "alocare username");
            DIE(!login_data[1], "alocare password");
            // deschidem conexiunea cu serverul
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // citim de la tastatura username-ul si password
            printf("username=");
            fgets(login_data[0], BUFLEN - 1, stdin);
            printf("password=");
            fgets(login_data[1], BUFLEN - 1, stdin);
            login_data[0][strlen(login_data[0]) - 1] = '\0';
            login_data[1][strlen(login_data[1]) - 1] = '\0';
            // cream obiectul de tip JSON
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            // cream stringul care va fi parsat cu json
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", login_data[0]);
            json_object_set_string(root_object, "password", login_data[1]);
            serialized_string = json_serialize_to_string_pretty(root_value);
            printf("\n");
            printf("REQUEST\n\n");
            // facem cererea catre server cu parametrii corespunzatori
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/login", "application/json", serialized_string, NULL, NULL);
            puts(message);
            // raspunsul trimis de server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            memset(aux, 0 ,BUFLEN);
            memset(cookie, 0, BUFLEN);
            // cautam cookie-ul in raspunsul trimis de server pentru a il retine
            // intr o variabila auxiliara,fiind folosit maii tarziu la autentificarea
            // pe server
            aux = strstr(response, "Set-Cookie: ");
            if(aux != NULL) {
            sscanf(aux, "Set-Cookie: %s;", cookie);
            cookie[strlen(cookie) - 1] = 0;
            }
            printf("%s\n\n", cookie);
            printf("ANSWER\n\n");
            error_json = strstr(response, "error");
            req = strstr(response, "Too many requests");
            if(error_json != NULL) { // if JSON received is type of {"error:""..."}
                printf("[ERROR] Credentials dont match registered credentials, try again later\n");
            } else if (req != NULL) { // if too many requests on the server
                printf("[ERROR] Too many requests, please try again later.\n");
            } else { // if command executed correctly
                printf("[SUCCES] Login in with username \"%s\" and password \"%s\"\n", login_data[0], login_data[1]);
            }

            // eliberam memoria
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            free(login_data[0]);
            free(error_json);
            free(req);
            free(login_data[1]);
            free(login_data);
            // inchidem conexiunea
            close(sockfd);
        }

        if(!strcmp(buff, "enter_library\n")) {
            // deschidem conexiunea cu serverul
            // alocam memorie pentru stringurile folosite la parsarea erorilor
            char *error_json = (char *) calloc(BUFLEN, sizeof(char));
            char *req = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!req, "alocare req");
            DIE(!error_json,"alocare vector de erori");
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            printf("\n");
            printf("REQUEST\n\n");
            // mesajul de requestul trimis catre server cu parametrii corespunzatori
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/access", NULL, cookie, NULL);
            puts(message);
            // primim raspunsul de la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // extragem din mesajul primit de la server token-ul
            // il retinem intr o variabila auxiliara pentru a putea
            // demonstra accesul in biblioteca
            memset(aux, 0 ,BUFLEN);
            memset(token, 0 ,BUFLEN);
            aux = strstr(response, "token");
            if(aux != NULL) {
            strcpy(token, aux + 8);
            // token-ul are forma {"token:""..."},astfel incat
            // retinem in variabila noastra doar ce se afla in locul ...
            token = strtok(token, "}");
            token[strlen(token) - 1] = 0;
            }
            printf("%s\n\n", token);
            printf("ANSWER\n\n");
            // parsarea mesajului primit de la server
            error_json = strstr(response, "error");
            req = strstr(response, "Too many requests");
            if(error_json != NULL) { //if JSON received is type of {"error:""..."}
                printf("[ERROR] Couldnt access the library, you are not logged in\n");
            } else if (req != NULL) { //if too many requests on the server
                printf("[ERROR] Too many requests, please try again later.\n");
            } else { //if command executed correctly
                printf("[SUCCES] Welcome to the library\n");
            }
            free(error_json);
            free(req);
            // inchidem conexiunea cu serverul
            close(sockfd);
        }

        if(!strcmp(buff, "get_books\n")) {
            // deschidem conexiunea cu serverul
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            printf("\n");
            printf("REQUEST\n\n");
            // cream mesajul de reqeust trimit catre server
            // cu ip-ul, url-ul corespunzator si token-ul pentru a demonstra
            // ca suntem autentificat si avem acces in librarie
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/books", NULL, NULL, token);
            puts(message);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printam mesajul de la server
            printf("ANSWER\n\n");
            puts(response);
            // inchidem conexiunea
            close(sockfd);
        }

        if(!strcmp(buff, "get_book\n")) {
            // deschidem conexiunea cu serverul
            int id;
            // cream url-ul pe care il vom transmite ca parametru functiei de request catre serve
            // acesta se va obtine din url dat si concatenarea cu id-ul cartii cerute de utilizator
            char *id_s = (char *) calloc(20, sizeof(char));
            DIE(!id_s, "alocare id ca string");
            char s[] = "/api/v1/tema/library/books/";
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            printf("id=");
            scanf("%d", &id);
            // citim id-ul de la stdin si transformam int-ul intr o variabila de tip string
            sprintf(id_s, "%d", id);
            strcat(s, id_s);
            printf("%s\n\n", s);
            printf("\n");
            printf("REQUEST\n\n");
            // facem request-ul catre server
            message = compute_get_request(IP_SERVER, s, NULL, NULL, token);
            puts(message);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // afisam mesajul primit de la server
            printf("ANSWER\n\n");
            puts(response);
            free(id_s);
            // inchidem conexiunea
            close(sockfd);
        }

        if(!strcmp(buff, "add_book\n")) {
            // deschidem conexiunea cu serverul
            // alocam memorie pentru toate string-urile citite de la stdin
            // cele corespunazatoare adaugarii unei carti in librarie
            char *title = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!title, "alocare titlu");
            char *author = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!author, "alocare autor");
            char *genre = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!genre, "alocare gen carte");
            char *publisher = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!publisher, "alocare publisher");
            int page_count;
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // citim de la stdin toate aceste date
            printf("title=");
            fgets(title, BUFLEN - 1, stdin);
            printf("author=");
            fgets(author, BUFLEN - 1, stdin);
            printf("genre=");
            fgets(genre, BUFLEN - 1, stdin);
            printf("publisher=");
            fgets(publisher, BUFLEN - 1, stdin);
            printf("page_count=");
            scanf("%d", &page_count);
            // punem terminatorul de sir pentru fiecare string
            title[strlen(title) - 1] = 0;
            author[strlen(author) - 1] = 0;
            genre[strlen(genre) - 1] = 0;
            publisher[strlen(publisher) - 1] = 0;
            // cream obiectul de tip JSON
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            // cream string-ul care va fi parsat cu json
            char *serialized_string = NULL;
            // punem toate datele citite de la stdin in acest string
            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_number(root_object, "page_count", page_count);
            json_object_set_string(root_object, "publisher", publisher);
            serialized_string = json_serialize_to_string_pretty(root_value);
            printf("\n");
            printf("REQUEST\n\n");
            // facem request-ul catre server cu obiectul de tip JSON pe care l am creat
            message = compute_post_request(IP_SERVER, "/api/v1/tema/library/books", "application/json", serialized_string, NULL, token);
            puts(message);

            send_to_server(sockfd, message);
            // afisam mesajul de la server
            response = receive_from_server(sockfd);
            printf("ANSWER\n\n");
            puts(response);
            // eliberam memoria alocata pentru citirea datelor
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            free(title);
            free(author);
            free(genre);
            free(publisher);
            // inchidem conexiunea
            close(sockfd);
        }

        if(!strcmp(buff, "delete_book\n")) {
            // deschidem conexiunea cu serverul si citim de la stdin
            // id--ul cartii ce urmeaza sa fie stearsa
            int id;
            char *id_s = (char *) calloc(20, sizeof(char));
            DIE(!id_s, "alocare id ca string");
            char s[] = "/api/v1/tema/library/books/";
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            printf("id=");
            // citim id-ul cartii si o transformam dintr-o variabila de tip int
            // intr o variabila de tip string
            scanf("%d", &id);
            sprintf(id_s, "%d", id);
            strcat(s, id_s);
            printf("%s\n\n", s);
            printf("\n");
            printf("REQUEST\n\n");
            // facem request-ul catre server
            message = compute_delete_request(IP_SERVER, s, NULL, NULL, token);
            puts(message);

            send_to_server(sockfd, message);
            // afisam mesajul primit de la server
            response = receive_from_server(sockfd);
            printf("ANSWER\n\n");
            puts(response);
            free(id_s);
            // inchidem conexiunea
            close(sockfd);
       }

       if(!strcmp(buff, "logout\n")) {
            // deschidem conexiunea cu serverul
            char *error_json = (char *) calloc(BUFLEN, sizeof(char));
            char *req = (char *) calloc(BUFLEN, sizeof(char));
            DIE(!req, "alocare req");
            DIE(!error_json,"alocare vector de erori");
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            printf("\n");
            printf("REQUEST\n\n");
            // trimitem mesajul de request catre server
            message = compute_get_request(IP_SERVER, "/api/v1/tema/auth/logout", NULL, cookie, NULL);
            puts(message);
            // afisam mesajul primit de la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // resetam cookie-ul si token-ul pentru ca userul
            // nu trebuie sa mai aiba acces la server dupa comanda logout
            memset(cookie, 0, BUFLEN);
            memset(token, 0 , BUFLEN);
            printf("ANSWER\n\n");
            error_json = strstr(response, "error");
            req = strstr(response, "Too many requests");
            if(error_json != NULL) { //if JSON received is type of {"error:""..."}
                printf("[ERROR] Couldnt access the library, you are not logged in\n");
            } else if (req != NULL) { //if too many requests on the server
                printf("[ERROR] Too many requests, please try again later.\n");
            } else { //if command executed correctly
                printf("[SUCCES] Deconnected succesfully from the library\n");
            }
            free(error_json);
            free(req);
            // inchidem conexiunea
            close(sockfd);
       }

       if(!strcmp(buff, "exit\n")) {
           // oprim conexiunea cu serverul
           // nu mai pot fi citite comenzi de la tastatura
           printf("Exit server\n");
           break;
       }
    }
    // eliberam memoria alocata pentru cookie,token
    // message si responde pe care il trimitem si primim de la server
    free(buff);
    free(cookie);
    free(token);
    free(message);
    free(response);
    return 0;
}
