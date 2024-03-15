#ifndef _REQUESTS_
#define _REQUESTS_

// returns a GET request string 
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count);

// returns a POST request string and cookies can be null
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count, char* query_params);
// returns a DELETE request 
char *compute_delete_request(char *host, char *url, char *query_params, char **cookies);

// gets a cookie
char* get_cookie(char* response);

// get a JWT token
const char* get_token(char* response);

#endif
