// 324CA Ciorceanu Andrei Razvan
// Scheletul a fost preluat si modificat din Laboratorul 10 HTTP
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"


char *compute_get_request(char *host, char *url, char *url_params, char *cookies, char *token)
{   // alocare memorie pentru mesajul si raspunsul parsate catre server
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (url_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, url_params);
    }
    else
    {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    // Adaugam token JWT pentru a fi folosit mai tarziu
    // pentru a demonstra autentificarea la server prin intermediul unui cod 
    // codificat binar
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
            sprintf(line, "Cookie: %s", cookies);
            compute_message(message, line);
        }      
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char *body_data,
                                 char *cookies, char *token)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    // Step 1: write the method n"ame, URL and protocol type
    memset(line, 0, LINELEN);
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
     // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    // Adaugare token JWT
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    int msg_size = strlen(body_data);
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    // adaugare headere cu Content-Type si Content-Length
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", msg_size);
    compute_message(message, line);
    
    compute_message(message, "");
    // verificare daca continutul de dupa header este nenul
    if(body_data != NULL) {
        compute_message(message, body_data);
    }

     // Step 4 (optional): add cookies
    if (cookies != NULL) {
            sprintf(line, "Cookie: %s", cookies);
            compute_message(message, line);    
        }
    // Step 5: add new line at end of header
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *url_params, char *cookies, char *token)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (url_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, url_params);
    }
    else
    {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    // Adaugam token JWT 
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
            sprintf(line, "Cookie: %s", cookies);
            compute_message(message, line);
        }      
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}