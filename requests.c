// Costin Didoaca 323CA
#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "buffer.h"
#include "parson.h"

#define SET_COOKIE_START "Set"
#define COOKIE_START_CHAR 'c'
#define ERROR_MESSAGE "Error in getting the cookie\n"
#define TOKEN_DELIMITER " ;"

void append_line_to_message(char **message, char *line)
{
    strcat(*message, line);
    strcat(*message, "\r\n");
}

char *compute_get_request(char *host, char *url, char *token, char **cookies, int cookies_count)
{
    
    char *line = (char*)calloc(LINELEN, sizeof(char));
    char *message = (char*)calloc(BUFLEN, sizeof(char));

    if(message == NULL || line == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    snprintf(line, LINELEN, "GET %s HTTP/1.1", url);
    append_line_to_message(&message, line);

    snprintf(line, LINELEN, "Host: %s", host);
    append_line_to_message(&message, line);

    if(cookies != NULL && strlen(cookies[0]) != 0) 
    {
        snprintf(line, LINELEN, "Cookie:");
        for(int i = 0; i < cookies_count; i++) 
        {
            strncat(line, " ", LINELEN - strlen(line) - 1);
            strncat(line, cookies[i], LINELEN - strlen(line) - 1);
        }
        append_line_to_message(&message, line);
    }

    if(token != NULL && strlen(token) != 0)
    {
        snprintf(line, LINELEN, "Authorization: Bearer %s", token);
        append_line_to_message(&message, line);
    }

    append_line_to_message(&message, "\n");

    free(line);
    return message;
}


char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                          int body_data_fields_count, char **cookies, int cookies_count, char *token)
{
    
    char *line = (char*)calloc(LINELEN, sizeof(char));
    char *message = (char*)calloc(BUFLEN, sizeof(char));
    char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

    if(message == NULL || line == NULL || body_data_buffer == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    snprintf(line, LINELEN, "POST %s HTTP/1.1", url);
    append_line_to_message(&message, line);

    snprintf(line, LINELEN, "Host: %s", host);
    append_line_to_message(&message, line);

    if(token != NULL && strlen(token) != 0) 
    {
        snprintf(line, LINELEN, "Authorization: Bearer %s", token);
        append_line_to_message(&message, line);
    }

    strncat(body_data_buffer, "{\n   ", LINELEN - strlen(body_data_buffer) - 1);
    for(int i = 0; i < body_data_fields_count; i++) 
    {
        strncat(body_data_buffer, body_data[i], LINELEN - strlen(body_data_buffer) - 1);
        if(i < body_data_fields_count - 1) 
        {
            strncat(body_data_buffer, ",\n   ", LINELEN - strlen(body_data_buffer) - 1);
        } 
        else 
        {
            strncat(body_data_buffer, "\n}", LINELEN - strlen(body_data_buffer) - 1);
        }
    }

    snprintf(line, LINELEN, "Content-Type: %s", content_type);
    append_line_to_message(&message, line);
    snprintf(line, LINELEN, "Content-Length: %ld", strlen(body_data_buffer));
    append_line_to_message(&message, line);

    if(cookies != NULL && strlen(cookies[0]) != 0) 
    {
        snprintf(line, LINELEN, "Cookie:");
        for(int i = 0; i < cookies_count; i++) 
        {
            strncat(line, " ", LINELEN - strlen(line) - 1);
            strncat(line, cookies[i], LINELEN - strlen(line) - 1);
            strncat(line, ";", LINELEN - strlen(line) - 1);
        }
        append_line_to_message(&message, line);
    }

    append_line_to_message(&message, "");

    memset(line, 0, LINELEN);
    strncat(line, body_data_buffer, LINELEN - strlen(line) - 1);
    append_line_to_message(&message, line);

    free(line);
    free(body_data_buffer);

    return message;
}


char *compute_delete_request(char *host, char *url, char *token, char **cookies)
{
    
    char *line = (char*)calloc(LINELEN, sizeof(char));
    char *message = (char*)calloc(BUFLEN, sizeof(char));

    if(message == NULL || line == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    snprintf(line, LINELEN, "DELETE %s HTTP/1.1", url);
    append_line_to_message(&message, line);

    snprintf(line, LINELEN, "Host: %s", host);
    append_line_to_message(&message, line);

    if(cookies != NULL && strlen(cookies[0]) > 0) 
    {
        snprintf(line, LINELEN, "Cookie: %s", cookies[0]);
        append_line_to_message(&message, line);
    }

    if(token != NULL && strlen(token) != 0) 
    {
        snprintf(line, LINELEN, "Authorization: Bearer %s", token);
        append_line_to_message(&message, line);
    }

    append_line_to_message(&message, "\n");

    free(line);

    return message;
}


char* get_cookie(char *response)
{
   
    char* line = strtok(response, "\r\n");
    
    while(line) 
    {   
        // iheck if the line starts with "Set" indicates a Set-Cookie header
        if(memcmp(line, SET_COOKIE_START, strlen(SET_COOKIE_START)) == 0)
        {
            // tokenize the line into words
            char* word = strtok(line, TOKEN_DELIMITER);
            
            while(word) 
            {
                // if the word starts with 'c' it's a found cookie
                if(word[0] == COOKIE_START_CHAR)
                {
                    printf("Cookie: %s\n", word);
                    return word;
                } 
                // next word
                word = strtok(NULL, TOKEN_DELIMITER);
            }
        }
        // next line
        line = strtok(NULL, "\r\n");
    }
    
    return ERROR_MESSAGE;
}

const char* get_token(char *response) 
{
    char* token = strtok(response, "\r\n");
    while(token) 
    {
        if(token[0] == '{') 
        {
            JSON_Value *tip = json_parse_string(token);
            JSON_Object *element = json_value_get_object(tip);
            
            if(json_object_get_string(element, "token") != NULL) 
            {
                printf("Token: %s\n\n", json_object_get_string(element, "token"));
                return json_object_get_string(element, "token");
            }

        }
        token = strtok(NULL,"\r\n");
    }
    return "Error in extracting the token\n";
}

