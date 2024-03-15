// Costin Didoaca 323CA
#include <stdio.h>
#include <stdlib.h>     
#include <unistd.h>     
#include <string.h>    
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <ctype.h>


#define CONNECTION_IP "34.254.242.81"
#define PORT 8080
#define MAX_COMMAND_LENGTH 15

// struct containing books details
typedef struct {
    char title[BUFLEN];
    char author[BUFLEN];
    char genre[BUFLEN];
    char publisher[BUFLEN];
    int page_count;
} Book;
// struct containing connection details
typedef struct {
    char *token;
    char *message;
    char *response;
    int sockfd;
    char **cookies;
    char **to_send;
    Book book;
} Connection;



char *get_response_code(char *response);
void clear_string(char *str);
void read_and_send_credentials(Connection *conn);



// Function to process and print book information received in a JSON format
void process_json_and_print_books(char *parse)
{
    printf("-----------\n");
    JSON_Value *type = json_parse_string(parse);
    JSON_Array *books = json_value_get_array(type);
    uint16_t book_cnt = json_array_get_count(books);
    for(int i = 0; i < book_cnt; i++) 
    {   
        JSON_Object *element = json_array_get_object(books, i);
        printf("ID: %d\nTitle: %s\n", (int)json_object_get_number(element, "id"), json_object_get_string(element, "title"));
        
    }
}
// Function to check if the response matches the expected string
int check_response(char *response, char *expected) {
    return memcmp(response, expected, 12) == 0;
}

// Function to process a command to add a book
void process_add_book_command(Connection *conn) {
    printf("title=");
    scanf("%s", conn->book.title);
    printf("author=");
    scanf("%s", conn->book.author);
    printf("genre=");
    scanf("%s", conn->book.genre);
    printf("publisher=");
    scanf("%s", conn->book.publisher);
    printf("page_count=");
    char page_count[BUFLEN];
    scanf("%s", page_count);
    if((conn->book.page_count = atoi(page_count)) == 0) {
        printf("Invalid number of pages!\n");
        return;
    }

    sprintf(conn->to_send[0], "\"title\": \"%s\"", conn->book.title);
    sprintf(conn->to_send[1], "\"author\": \"%s\"", conn->book.author);
    sprintf(conn->to_send[2], "\"genre\": \"%s\"", conn->book.genre);
    sprintf(conn->to_send[3], "\"publisher\": \"%s\"", conn->book.publisher);
    sprintf(conn->to_send[4], "\"page_count\": %d", conn->book.page_count);

    printf("\n");
    char *message = compute_post_request("2", "/api/v1/tema/library/books", "application/json", conn->to_send, 5, NULL, 0, conn->token);
    
    printf("%s\n", message);
    send_to_server(conn->sockfd, message);
    char *response = receive_from_server(conn->sockfd);
    printf("Received:\n");
    char *code = strtok(response, "\n\r");
    if(check_response(response, "HTTP/1.1 200")) {
        printf("Response: %s\n", code);
        printf("Added a book successfully\n");
    } else if(check_response(response, "HTTP/1.1 403")) {
        printf("Response: %s\n", code);
        printf("Forbidden access, you have to log in first\n");
    }   
}

// Function to process a command to get a books information
void process_get_book_command(Connection *conn) {
    char get_id[10];
    printf("id=");
    scanf("%s", get_id);
    char path[100];
    strcpy(path, "/api/v1/tema/library/books/");
    strcat(path, get_id);
    conn->message = compute_get_request("2", path, conn->token, conn->cookies, 1);
    
    printf("%s\n", conn->message);
    send_to_server(conn->sockfd, conn->message);
    conn->response = receive_from_server(conn->sockfd);
    printf("Received:\n");
    printf("%s\n", conn->response);

    char* parse;
    parse = strtok(conn->response, "\n");

    if(check_response(conn->response, "HTTP/1.1 200"))
    {
        printf("Response: %s\n", parse);
        printf("Succesfully received the book\n");
    }
    else if (check_response(conn->response, "HTTP/1.1 404"))
    {
        printf("Response: %s\n", parse);
        printf("There is no book with this id, please try another one\n");
    }
    else if (check_response(conn->response, "HTTP/1.1 403"))
    {
        printf("Response: %s\n", parse);
        printf("Forbidden access, you must log in first\n");
    }   


}

// Function to get all the books stored in users library
void process_get_books_command(Connection *connection)
{
    printf("\n");

    connection->message = compute_get_request("2", "/api/v1/tema/library/books", 
                                              connection->token, 
                                              connection->cookies, 1);

    
    printf("%s\n", connection->message);

    send_to_server(connection->sockfd, connection->message);

    connection->response = receive_from_server(connection->sockfd);

    printf("Received:\n");

    char* parse;
    parse = strtok(connection->response, "\r\n");

    if(check_response(connection->response, "HTTP/1.1 200"))
    {
        printf("Response: %s\n", parse);
        printf("Successfully received all books\n");
    }
    else if (check_response(connection->response, "HTTP/1.1 403"))
    {
        printf("Response: %s\n", parse);
        printf("Forbidden access, you must log in first\n");
    }  

    while(parse != NULL) 
    {
        if(parse[0] == '[') 
        {
            process_json_and_print_books(parse);
        }
        parse = strtok(NULL, "\n");
    }
}

// Function to delete a book from users library
void process_delete_book_command(Connection *connection) 
{
    char delete_id[100];
    char path[100];
    printf("id= ");
    scanf("%s", delete_id);
    printf("\n");
    
    strcpy(path, "/api/v1/tema/library/books/");
    strcat(path, delete_id);
    

    connection->message = compute_delete_request("2", 
                                                 path, 
                                                 connection->token, 
                                                 connection->cookies);

    
    printf("%s\n", connection->message);
    send_to_server(connection->sockfd, connection->message);
    
    connection->response = receive_from_server(connection->sockfd);
    printf("Received:\n");
    char *code = strtok(connection->response, "\n\r");

    if(check_response(connection->response, "HTTP/1.1 200"))
    {
        printf("Response: %s\n", code);
        printf("Successfully deleted the book\n");
    }
    else if (check_response(connection->response, "HTTP/1.1 404"))
    {
        printf("Response: %s\n", code);
        printf("There is no book with this id, please try another one\n");
    }
    else if (check_response(connection->response, "HTTP/1.1 403"))
    {
        printf("Response: %s\n", code);
        printf("Forbidden access, you must log in first\n");
    } 
}

// Function for logging out of current session
void process_logout_command(Connection *conn) {
    
       
        printf("\n");
        conn->message = compute_get_request("2", "/api/v1/tema/auth/logout", NULL, conn->cookies, 1);
        
        printf("%s\n", conn->message);
        send_to_server(conn->sockfd, conn->message);
        conn->response = receive_from_server(conn->sockfd);
        
        
        char *code = get_response_code(conn->response);
        if(check_response(conn->response, "HTTP/1.1 200"))
        {
            printf("Response: %s\n", code);
            printf("Successfully logged out\n");
            clear_string(conn->cookies[0]);
            clear_string(conn->token);
            
        } else
        {
            printf("Response: %s\n", code);
            printf("You are not logged in\n");
        }
        
    
}

// Function to enter the users library
void process_enter_library_command(Connection *conn) {
    printf("\n");
    conn->message = compute_get_request("2", "/api/v1/tema/library/access", NULL, conn->cookies, 1);
    
    puts(conn->message);
    send_to_server(conn->sockfd, conn->message);
    conn->response = receive_from_server(conn->sockfd);
    printf("Received:\n");
    clear_string(conn->token);
    strcpy(conn->token, get_token(conn->response));
    
    char *code = get_response_code(conn->response);
    if(check_response(conn->response, "HTTP/1.1 200"))
    {
        printf("Response: %s\n", code);
        printf("Welcome to the library\n");
    }
    else
    {
        printf("Response: %s\n", code);
        printf("Forbidden access, you must log in first\n");
    }
}

// Function to register a new user
void process_register_command(Connection *conn) {
    read_and_send_credentials(conn);

    conn->message = compute_post_request("2", "/api/v1/tema/auth/register", "application/json", conn->to_send, 2, NULL, 0, NULL);
   
    puts(conn->message);
    send_to_server(conn->sockfd, conn->message);
    conn->response = receive_from_server(conn->sockfd);
    printf("Received:\n");
    
    char *code = get_response_code(conn->response);
    if(check_response(conn->response, "HTTP/1.1 201"))
    {
        printf("Response: %s\n", code);
        printf("Succesfully registered\n");
    }
    else
    {
        printf("Response: %s\n", code);
        printf("User already exists\n");
    }
}

// Function for deleting/clearing a string
void clear_string(char *str) {
    if(str != NULL) memset(str, 0, BUFLEN);
}

// Function for logging in a registered user
int process_login_command(Connection *conn, int loggedIn) {

    
        read_and_send_credentials(conn);

        conn->message = compute_post_request("2", "/api/v1/tema/auth/login", "application/json", conn->to_send, 2, NULL, 0, NULL);
        
        puts(conn->message);
        send_to_server(conn->sockfd, conn->message);
        conn->response = receive_from_server(conn->sockfd);
        printf("Received:\n");

        char *cookie = get_cookie(conn->response);
        clear_string(conn->cookies[0]);
        strcpy(conn->cookies[0], cookie);

        char *code = get_response_code(conn->response);
        if(check_response(conn->response, "HTTP/1.1 200"))
        {    
            
            printf("Response: %s\n", code);
            printf("Welcome to the server\n");
            loggedIn = 1;
            
        }
        else
        {
            printf("Response: %s\n", code);
            printf("Wrong credentials\n");
            loggedIn = 0;
        }
        return loggedIn;
      
}

// Helping function for reading and sending the credentials of a user
void read_and_send_credentials(Connection *conn) {
    char username[BUFLEN];
    char password[BUFLEN];

    printf("username=");
    scanf("%s", username);
    strcpy(conn->to_send[0], "\"username\": \"");
    strcat(conn->to_send[0], username);
    strcat(conn->to_send[0], "\"");

    printf("password=");
    scanf("%s", password);
    strcpy(conn->to_send[1], "\"password\": \"");
    strcat(conn->to_send[1], password);
    strcat(conn->to_send[1], "\"");

    printf("\n");
}

// helping function for getting the response code from the server
char *get_response_code(char *response) {
    return strtok(response, "\r\n");
}


int main(int argc, char *argv[])
{
    

    char *token = malloc(BUFLEN * sizeof(char));
    char **cookies = malloc(sizeof(char *));


    cookies[0] = malloc(BUFLEN * sizeof(char));
    int loggedIn = 0;

    // processing the commands from stdin
    while(1) 
    {
        
        int sockfd = open_connection(CONNECTION_IP, PORT, AF_INET, SOCK_STREAM, 0);

        char command[MAX_COMMAND_LENGTH];
        char **to_send = malloc(5 * sizeof(char *));

        int i = 0;
        while(i <= 4) {
            to_send[i] = malloc(BUFLEN);
            i++;
        }
        
        
        Connection conn = {token, NULL, NULL, sockfd, cookies, to_send};
        
        scanf("%s", command);

        if(strcmp(command, "exit") == 0) 
        {
            close_connection(sockfd);
            free(conn.token);
            free(conn.cookies[0]);
            free(conn.cookies);
            for (int i = 0; i <= 4; ++i) 
            {
                free(conn.to_send[i]);
            }
            free(conn.to_send);
            break;
        }

        if(strcmp(command, "register") == 0)
        {
            process_register_command(&conn);
        } else
        if(strcmp(command, "login") == 0)
        {
            if(loggedIn == 1) {
                printf("Already logged in\n");
            } else {
                loggedIn = process_login_command(&conn, loggedIn);

            }
             
        } else 
        if(strcmp(command, "enter_library") == 0)
        {
            process_enter_library_command(&conn);
        } else
        if(strcmp(command, "logout") == 0)
        {
            process_logout_command(&conn);
            loggedIn = 0;
        } else

        if(strcmp(command, "add_book") == 0) {
            process_add_book_command(&conn);
        }else

        
        if(strcmp(command, "get_book") == 0)
        {
            process_get_book_command(&conn);
        } else

        if(strcmp(command, "get_books") == 0)
        {
            process_get_books_command(&conn);
        } else
        
    
        if(strcmp(command, "delete_book") == 0)
        {
            process_delete_book_command(&conn);
        }else

         {
            printf("Not a valid command!\n");
        }
        
        close_connection(sockfd);
    }
    return 0;
}
