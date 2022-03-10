#ifndef _REQUESTS_
#define _REQUESTS_
// 324CA Ciorceanu Andrei Razvan
// functiile sunt similare cu cele din laboratorul 10 HTTP
// am schimbat antetul functiilor, eliminand parametrii si am adaugat
// inca o functie delete necesare unei comenzi data de user
// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *url_params,
							char *cookies, char *token);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
							 char* cookies, char *token);
// computes and returns a DELETE request string (cookies can be NULL if not needed)
char *compute_delete_request(char *host, char *url, char *url_params,
							char *cookies, char *token);
#endif