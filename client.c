#include "utils.h"
#include "parson.h"
#include "helpers.h"
#include "requests.h"
#include <ctype.h>

// variabile globale pentru comunicarea cu serverul
char *host = SERVER_IP;
int port = SERVER_PORT;

// variabile pentru cookie-uri si token de autentificare
char session_cookie[BUFFLEN];
char user_session_cookie[BUFFLEN];
char token[BUFFLEN];

void login_admin_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem deja conectati
    if (strlen(session_cookie) > 0) {
        printf("ERROR: Already logged in. Please logout first.\n");
        return;
    }
    char *message;
    char *response;

    char username[BUFFLEN];
    char password[BUFFLEN];

    // citim credentialele de autentificare
    printf("username = ");
    fflush(stdout);
    fgets(username, BUFFLEN, stdin);
    username[strcspn(username, "\n")] = 0;
    printf("password = ");
    fflush(stdout);
    fgets(password, BUFFLEN, stdin);
    password[strcspn(password, "\n")] = 0;

    // deschidem conexiunea cu serverul
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim datele pentru autentificare
    char **login_data = calloc(2, sizeof(char*));
    login_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(login_data[0], "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    // cream cererea POST pentru autentificare
    message = compute_post_request(host, "/api/v1/tema/admin/login", PAYLOAD_TYPE, login_data, 1, NULL, 0);

    // trimitem cererea la server
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // verificam daca autentificarea a avut succes
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        printf("ERROR: Invalid credentials\n");
        
        free(message);
        free(response);
        free(login_data[0]);
        free(login_data);
        close_connection(sockfd);
        return;
    }

    // extragem cookie-ul de sesiune din raspuns
    char *cookie_start = strstr(response, "Set-Cookie: ");
    DIE(cookie_start == NULL, "No cookie found in response");

    cookie_start += strlen("Set-Cookie: ");
    char *cookie_end = strstr(cookie_start, ";");
    DIE(cookie_end == NULL, "Malformed cookie in response");

    int cookie_len = cookie_end - cookie_start;
    strncpy(session_cookie, cookie_start, cookie_len > BUFFLEN-1 ? BUFFLEN-1 : cookie_len);
    session_cookie[cookie_len > BUFFLEN-1 ? BUFFLEN-1 : cookie_len] = '\0';

    printf("SUCCESS: Admin autentificat cu succes\n");

    // eliberam memoria
    free(message);
    free(response);
    free(login_data[0]);
    free(login_data);
    close_connection(sockfd);
}

void add_user_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati ca admin
    if (strlen(session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }
    char *message;
    char *response;

    char username[BUFFLEN];
    char password[BUFFLEN];

    // citim datele utilizatorului nou
    printf("username = ");
    fflush(stdout);
    fgets(username, BUFFLEN, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("password = ");
    fflush(stdout);
    fgets(password, BUFFLEN, stdin);
    password[strcspn(password, "\n")] = 0;

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");
    
    // pregatim datele utilizatorului
    char **login_data = calloc(2, sizeof(char*));
    login_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(login_data[0], "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    // adaugam cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], session_cookie);

    // cream cererea POST pentru adaugare utilizator
    message = compute_post_request(host, "/api/v1/tema/admin/users", PAYLOAD_TYPE, login_data, 1, cookies, 1);
    
    // trimitem cererea la server
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies); 

    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // verificam daca adaugarea a avut succes
    if (strstr(response, "HTTP/1.1 201") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplet/Corupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplet/Corupted information\n");
        }
    }
    else {
        printf("SUCCESS: Utilizator adăugat cu succes\n");
    }
    // eliberam memoria
    free(message);
    free(response);
    free(login_data[0]);
    free(login_data);
    fflush(stdout);
    close_connection(sockfd);
}

void get_users_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati ca admin
    if (strlen(session_cookie) == 0) {
        printf("ERROR: Not logged in as admin. Please login first.\n");
        return;
    }
    char *message;
    char *response;

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // adaugam cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], session_cookie);

    // cream cererea GET pentru lista utilizatorilor
    message = compute_get_request(host, "/api/v1/tema/admin/users", NULL, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // verificam si procesam raspunsul
    if(strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplet/Corupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplet/Corupted information\n");
        }
    } else{
        // parsam json-ul cu lista de utilizatori
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                JSON_Array *users_array = json_object_get_array(root_object, "users");

                printf("SUCCESS: Lista utilizatorilor");
                
                // afisam fiecare utilizator din lista
                if (users_array) {
                    size_t count = json_array_get_count(users_array);
                    for (size_t i = 0; i < count; i++) {
                        JSON_Object *user = json_array_get_object(users_array, i);
                        const char *username = json_object_get_string(user, "username");
                        const char *password = json_object_get_string(user, "password");
                        
                        printf("#%ld:%s:%s\n", i, username, password);
                    }
                }
                json_value_free(root_value);
            }
        }
    }
    // eliberam memoria
    free(message);
    free(response);
    fflush(stdout);
    close_connection(sockfd);
}

void logout_admin_cmd(){
    // verificam daca suntem conectati ca admin
    if (strlen(session_cookie) == 0) {
        printf("ERROR: Not logged in as admin. Please login first.\n");
        return;
    }

    char **cookies = NULL;
    char *message = NULL;
    char *response = NULL;
    int sockfd = -1;

    // deschidem conexiunea
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));

    strcpy(cookies[0], session_cookie);

    // cream cererea GET pentru delogare
    message = compute_get_request(host, "/api/v1/tema/admin/logout", NULL, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // verificam daca delogarea a avut succes
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        printf("ERROR: Logout failed\n");
    } else {
        printf("SUCCESS: Admin delogat\n");
        // stergem cookie-ul de sesiune
        memset(session_cookie, 0, BUFFLEN);
    }

    // eliberam memoria
    free(message);
    free(response);
    fflush(stdout);
    close_connection(sockfd);
}

void login_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem deja conectati ca utilizator
    if (strlen(user_session_cookie) != 0) {
        printf("ERROR: Already logged in. Logout first\n");
        return;
    }

    char *message;
    char *response;
    char admin_username[BUFFLEN];
    char username[BUFFLEN];
    char password[BUFFLEN];

    // citim datele de autentificare
    printf("admin_username = ");
    fflush(stdout);
    fgets(admin_username, BUFFLEN, stdin);
    admin_username[strcspn(admin_username, "\n")] = 0;

    printf("username = ");
    fflush(stdout);
    fgets(username, BUFFLEN, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("password = ");
    fflush(stdout);
    fgets(password, BUFFLEN, stdin);
    password[strcspn(password, "\n")] = 0;

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim datele de autentificare
    char **login_data = calloc(2, sizeof(char*));
    login_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(login_data[0], "{\"admin_username\":\"%s\",\"username\":\"%s\",\"password\":\"%s\"}", admin_username, username, password);

    // folosim cookie-ul de sesiune admin daca exista
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], session_cookie);
    
    // cream cererea POST pentru autentificare utilizator
    message = compute_post_request(host, "/api/v1/tema/user/login", PAYLOAD_TYPE, login_data, 1, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);
    
    // primim raspunsul
    response = receive_from_server(sockfd);

    // verificam daca autentificarea a avut succes
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplet/Corupted information\n");
                json_value_free(root_value);
            }
        }
        else {
            printf("ERROR: Incomplet/Corupted information\n");
        }
        free(message);
        free(response);
        free(login_data[0]);
        free(login_data);
        close_connection(sockfd);
        return;
    } else {
        // extragem cookie-ul de sesiune pentru utilizator
        char *cookie_start = strstr(response, "Set-Cookie: ");
        DIE(cookie_start == NULL, "No cookie found in response");

        cookie_start += strlen("Set-Cookie: ");
        char *cookie_end = strstr(cookie_start, ";");
        DIE(cookie_end == NULL, "Malformed cookie in response");

        int cookie_len = cookie_end - cookie_start;
        strncpy(user_session_cookie, cookie_start, cookie_len > BUFFLEN-1 ? BUFFLEN-1 : cookie_len);
        user_session_cookie[cookie_len > BUFFLEN-1 ? BUFFLEN-1 : cookie_len] = '\0';

        printf("SUCCESS: Autentificare reușită\n");
    }
    // eliberam memoria
    free(message);
    free(response);
    free(login_data[0]);
    free(login_data);
    fflush(stdout);
    close_connection(sockfd);
}

void logout_cmd(){
    // verificam daca suntem conectati ca utilizator
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    char **cookies = NULL;
    char *message = NULL;
    char *response = NULL;
    int sockfd = -1;

    // deschidem conexiunea
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));

    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru delogare
    message = compute_get_request(host, "/api/v1/tema/user/logout", NULL, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // verificam daca delogarea a avut succes
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        printf("ERROR: Logout failed\n");
    } else {
        printf("SUCCESS: Utilizator delogat\n");
        // stergem cookie-ul de sesiune pentru utilizator
        memset(user_session_cookie, 0, BUFFLEN);
    }

    // eliberam memoria
    free(message);
    free(response);
    fflush(stdout);
    close_connection(sockfd);
}

void get_access_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati ca utilizator
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }
    char *message;
    char *response;

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru obtinere token
    message = compute_get_request(host, "/api/v1/tema/library/access", NULL, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplet/Corupted information\n");
                }
                json_value_free(root_value);
        } else {
            printf("ERROR: Incomplet/Corupted information\n");
        }
    } else {
        // extragem token-ul JWT
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *tmp_token = json_object_get_string(root_object, "token");
                if (tmp_token) {
                    strncpy(token, tmp_token, BUFFLEN - 1);
                    token[BUFFLEN - 1] = '\0';
                }
                
                printf("SUCCESS: Token JWT primit\n");
                json_value_free(root_value);
            }
        }
    }
    // eliberam memoria
    free(message);
    free(response);
    fflush(stdout);
    close_connection(sockfd);
}

void get_movies_cmd() {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru obtinerea filmelor
    message = compute_get_request(host, "/api/v1/tema/library/movies", NULL, cookies, 1);
    
    // adaugam token-ul JWT in header-ul de autorizare
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r\n", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }
    
    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        // parsam si afisam filmele
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                if (json_value_get_type(root_value) == JSONArray) {
                    // cazul in care raspunsul e un array direct
                    JSON_Array *movies_array = json_value_get_array(root_value);
                    size_t count = json_array_get_count(movies_array);
                    
                    printf("SUCCESS: Lista filmelor:\n");
                    
                    for (size_t i = 0; i < count; i++) {
                        JSON_Object *movie = json_array_get_object(movies_array, i);
                        int id = (int)json_object_get_number(movie, "id");
                        const char *title = json_object_get_string(movie, "title");
                        
                        printf("#%d:%s\n", id, title);
                    }
                } else {
                    // cazul in care raspunsul e un obiect care contine un array
                    JSON_Object *root_object = json_value_get_object(root_value);
                    JSON_Array *movies_array = json_object_get_array(root_object, "movies");
                    
                    printf("SUCCESS: Lista filmelor:\n");

                    if (movies_array) {
                        size_t count = json_array_get_count(movies_array);
                        for (size_t i = 0; i < count; i++) {
                            JSON_Object *movie = json_array_get_object(movies_array, i);
                            int id = (int)json_object_get_number(movie, "id");
                            const char *title = json_object_get_string(movie, "title");
                            
                            printf("#%d:%s\n", id, title);
                        }
                    }
                }
                json_value_free(root_value);
            }
        }
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
}

void add_movie_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    char title[BUFFLEN];
    char description[BUFFLEN];
    int year;
    double rating;

    // citim detaliile filmului
    printf("title = ");
    fflush(stdout);
    fgets(title, BUFFLEN, stdin);
    title[strcspn(title, "\n")] = 0;

    printf("year = ");
    fflush(stdout);
    char year_str[BUFFLEN];
    fgets(year_str, BUFFLEN, stdin);
    year_str[strcspn(year_str, "\n")] = 0;
    year = atoi(year_str);

    printf("description = ");
    fflush(stdout);
    fgets(description, BUFFLEN, stdin);
    description[strcspn(description, "\n")] = 0;

    printf("rating = ");
    fflush(stdout);
    char rating_str[BUFFLEN];
    fgets(rating_str, BUFFLEN, stdin);
    rating_str[strcspn(rating_str, "\n")] = 0;
    rating = atof(rating_str);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim datele filmului
    char **movie_data = calloc(2, sizeof(char*));
    movie_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(movie_data[0], "{\"title\":\"%s\",\"year\":%d,\"description\":\"%s\",\"rating\":%.1f}", title, year, description, rating);

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea POST pentru adaugare film
    message = compute_post_request(host, "/api/v1/tema/library/movies", PAYLOAD_TYPE, movie_data, 1, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);
    free(movie_data[0]);
    free(movie_data);
    
    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 201") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information else\n");
        }
    } else {
        printf("SUCCESS: Film adăugat\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void get_movie_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    int id;

    // citim id-ul filmului
    printf("id = ");
    fflush(stdout);
    char id_str[BUFFLEN];
    fgets(id_str, BUFFLEN, stdin);
    id_str[strcspn(id_str, "\n")] = 0;
    id = atoi(id_str);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // construim URL-ul pentru filmul specific
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/movies/%d", id);

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru obtinerea detaliilor filmului
    message = compute_get_request(host, url, NULL, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r\n", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if(strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        // parsam si afisam detaliile filmului
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                JSON_Object *movie = json_value_get_object(root_value);
                
                const char *title = json_object_get_string(movie, "title");
                int year = (int)json_object_get_number(movie, "year");
                const char *description = json_object_get_string(movie, "description");
                const char *rating = json_object_get_string(movie, "rating");
                
                printf("SUCCESS: Detalii film\n");
                printf("title: %s\n", title);
                printf("year: %d\n", year);
                if (description)
                    printf("description: %s\n", description);
                printf("rating: %s\n", rating);
                
                json_value_free(root_value);
            }
        }
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void update_movie_cmd() {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    int id;
    char title[BUFFLEN];
    char description[BUFFLEN];
    int year;
    char rating[BUFFLEN];

    // citim id-ul filmului si noile detalii
    printf("id = ");
    fflush(stdout);
    char id_str[BUFFLEN];
    fgets(id_str, BUFFLEN, stdin);
    id_str[strcspn(id_str, "\n")] = 0;
    id = atoi(id_str);

    printf("title = ");
    fflush(stdout);
    fgets(title, BUFFLEN, stdin);
    title[strcspn(title, "\n")] = 0;

    printf("year = ");
    fflush(stdout);
    char year_str[BUFFLEN];
    fgets(year_str, BUFFLEN, stdin);
    year_str[strcspn(year_str, "\n")] = 0;
    year = atoi(year_str);

    printf("description = ");
    fflush(stdout);
    fgets(description, BUFFLEN, stdin);
    description[strcspn(description, "\n")] = 0;

    printf("rating = ");
    fflush(stdout);
    
    fgets(rating, BUFFLEN, stdin);
    rating[strcspn(rating, "\n")] = 0;

    // construim URL-ul pentru filmul care va fi actualizat
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/movies/%d", id);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim datele actualizate ale filmului
    char **movie_data = calloc(2, sizeof(char*));
    movie_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(movie_data[0], "{\"title\":\"%s\",\"year\":%d,\"description\":\"%s\",\"rating\":%s}", title, year, description, rating);

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea PUT pentru actualizarea filmului
    message = compute_put_request(host, url, PAYLOAD_TYPE, movie_data, 1, cookies, 1);
    
    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);
    free(movie_data[0]);
    free(movie_data);
    
    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Filmul cu id=%d nu există!\n", id);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Filmul cu id=%d nu există!\n", id);
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Film actualizat\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void delete_movie_cmd() {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    int id;

    // citim id-ul filmului de sters
    printf("id = ");
    fflush(stdout);
    char id_str[BUFFLEN];
    fgets(id_str, BUFFLEN, stdin);
    id_str[strcspn(id_str, "\n")] = 0;
    id = atoi(id_str);

    // construim URL-ul pentru filmul care va fi sters
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/movies/%d", id);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea DELETE pentru stergerea filmului
    message = compute_delete_request(host, url, NULL, cookies, 1);
    
    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);
    
    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Filmul cu id=%d nu există!\n", id);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Filmul cu id=%d nu există!\n", id);
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Film șters cu succes\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void add_movie_to_collection_cmd(int collection_id, int movie_id) {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;

    // construim URL-ul pentru adaugarea filmului in colectie
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/collections/%d/movies", collection_id);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim datele cu id-ul filmului
    char **movie_data = calloc(2, sizeof(char*));
    movie_data[0] = calloc(LINELEN, sizeof(char));
    sprintf(movie_data[0], "{\"id\":%d}", movie_id);

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea POST pentru adaugarea filmului in colectie
    message = compute_post_request(host, url, PAYLOAD_TYPE, movie_data, 1, cookies, 1);
    
    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);
    free(movie_data[0]);
    free(movie_data);
    
    // primim raspunsul
    response = receive_from_server(sockfd);
    
    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL && strstr(response, "HTTP/1.1 201") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Film adăugat în colecție\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void add_collection_cmd() {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    char title[BUFFLEN];
    int num_movies;

    // citim titlul colectiei
    printf("title = ");
    fflush(stdout);
    fgets(title, BUFFLEN, stdin);
    title[strcspn(title, "\n")] = 0;

    // citim numarul de filme
    printf("num_movies = ");
    fflush(stdout);
    char num_movies_str[BUFFLEN];
    fgets(num_movies_str, BUFFLEN, stdin);
    num_movies_str[strcspn(num_movies_str, "\n")] = 0;
    num_movies = atoi(num_movies_str);
    
    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");
    
    // pregatim datele colectiei
    char **collection_data = calloc(2, sizeof(char*));
    collection_data[0] = calloc(LINELEN * 2, sizeof(char));
    
    sprintf(collection_data[0], "{\"title\":\"%s\"}", title);
    
    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);
    
    // cream cererea POST pentru crearea colectiei
    message = compute_post_request(host, "/api/v1/tema/library/collections", PAYLOAD_TYPE, collection_data, 1, cookies, 1);
    
    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';
    
    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }
    
    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);
    free(collection_data[0]);
    free(collection_data);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // extragem id-ul colectiei create
    int collection_id = -1;
    
    char *json_response = basic_extract_json_response(response);
    DIE(json_response == NULL, "Error extracting JSON from response");
    
    JSON_Value *root_value = json_parse_string(json_response);
    if (root_value) {
        if (json_value_get_type(root_value) == JSONObject) {
            JSON_Object *root_object = json_value_get_object(root_value);
            collection_id = (int)json_object_get_number(root_object, "id");
        }
        json_value_free(root_value);
    }
    
    // adaugam filmele in colectie
    for (int i = 0; i < num_movies; i++) {
        printf("movie_id[%d] = ", i);
        fflush(stdout);
        char movie_id[20];
        fgets(movie_id, 20, stdin);
        movie_id[strcspn(movie_id, "\n")] = 0;
        add_movie_to_collection_cmd(collection_id, atoi(movie_id));
    }
    
    // procesam raspunsul pentru crearea colectiei
    if (strstr(response, "HTTP/1.1 201") == NULL && strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Colecție adăugată\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void get_collections_cmd() {
    // curat buffer-ul de intrare
    fflush(stdin);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;
    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru obtinerea colectiilor
    message = compute_get_request(host, "/api/v1/tema/library/collections", NULL, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);
    
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Lista colecțiilor:\n");
        // parsam si afisam colectiile
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                JSON_Array *collections_array = NULL;

                if (json_value_get_type(root_value) == JSONObject) {
                    // cazul in care raspunsul e un obiect care contine un array
                    JSON_Object *root_object = json_value_get_object(root_value);
                    collections_array = json_object_get_array(root_object, "collections");
                }
                else if (json_value_get_type(root_value) == JSONArray) {
                    // cazul in care raspunsul e un array direct
                    collections_array = json_value_get_array(root_value);
                }
        
                if (collections_array) {
                    size_t count = json_array_get_count(collections_array);
                    for (size_t i = 0; i < count; i++) {
                        JSON_Object *collection = json_array_get_object(collections_array, i);
                        int id = json_object_get_number(collection, "id");
                        const char *title = json_object_get_string(collection, "title");
                        
                        printf("#%d: ", id);
                        printf("%s", title);

                        if (i < count - 1) {
                            printf("\n");
                        }
                    }
                }
                json_value_free(root_value);
            }
        }
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void get_collection_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);
    long id;
    char id_str[BUFFLEN];

    // citim id-ul colectiei
    printf("id = ");
    fflush(stdout);
    if (!fgets(id_str, BUFFLEN, stdin)) {
        printf("ERROR: failed to read collection id\n");
        return;
    }
    id_str[strcspn(id_str, "\n")] = 0;
    id = atol(id_str);

    // verificam daca suntem conectati si avem acces la librarie
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }

    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }

    char *message;
    char *response;

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // construim URL-ul pentru colectia specifica
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/collections/%ld", id);

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea GET pentru obtinerea detaliilor colectiei
    message = compute_get_request(host, url, NULL, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Colecția cu id=%ld nu există!\n", id);
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Colecția cu id=%ld nu există!\n", id);
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        char *json_response = basic_extract_json_response(response);
        if (json_response) {
            JSON_Value *root_value = json_parse_string(json_response);
            if (root_value) {
                JSON_Object *collection = json_value_get_object(root_value);
                const char *title = json_object_get_string(collection, "title");
                const char *owner = json_object_get_string(collection, "owner");
                
                printf("SUCCESS: Detalii colecție\n");
                printf("title: %s\n", title);
                if (owner)
                    printf("owner: %s\n", owner);
                
                JSON_Array *movies = json_object_get_array(collection, "movies");
                if (movies) {
                    size_t movies_count = json_array_get_count(movies);
                    for (size_t j = 0; j < movies_count; j++) {
                        JSON_Object *movie = json_array_get_object(movies, j);
                        int movie_id = (int)json_object_get_number(movie, "id");
                        const char *movie_title = json_object_get_string(movie, "title");
                        printf("#%d: %s\n", movie_id, movie_title);
                    }
                }
                json_value_free(root_value);
            }
        }
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void delete_collection_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }
    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }
    char *message;
    char *response;
    int id;

    // citim id-ul colectiei de sters
    printf("id = ");
    fflush(stdout);
    char id_str[BUFFLEN];
    fgets(id_str, BUFFLEN, stdin);
    id_str[strcspn(id_str, "\n")] = 0;
    id = atoi(id_str);

    // construim URL-ul pentru colectia care va fi stearsa
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/collections/%d", id);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea DELETE pentru stergerea colectiei
    message = compute_delete_request(host, url, NULL, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
        if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Colecția cu id=%d nu există!\n", id);
                else if (strstr(response, "HTTP/1.1 403") != NULL)
                    printf("ERROR: Nu sunteți owner-ul colecției!\n");
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Colecția cu id=%d nu există!\n", id);
        } else if (strstr(response, "HTTP/1.1 403") != NULL) {
            printf("ERROR: Nu sunteți owner-ul colecției!\n");
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Colecție ștearsă\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void delete_movie_from_collection_cmd(){
    // curat buffer-ul de intrare
    fflush(stdin);
    if (strlen(user_session_cookie) == 0) {
        printf("ERROR: Not logged in. Please login first.\n");
        return;
    }
    if (strlen(token) == 0) {
        printf("ERROR: Do not have access to the library.\n");
        return;
    }
    char *message;
    char *response;
    int collection_id;
    int movie_id;

    // citim id-urile colectiei si filmului de sters
    printf("collection_id = ");
    fflush(stdout);
    char collection_id_str[BUFFLEN];
    fgets(collection_id_str, BUFFLEN, stdin);
    collection_id_str[strcspn(collection_id_str, "\n")] = 0;
    collection_id = atoi(collection_id_str);

    printf("movie_id = ");
    fflush(stdout);
    char movie_id_str[BUFFLEN];
    fgets(movie_id_str, BUFFLEN, stdin);
    movie_id_str[strcspn(movie_id_str, "\n")] = 0;
    movie_id = atoi(movie_id_str);

    // construim URL-ul pentru stergerea filmului din colectie
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/library/collections/%d/movies/%d", collection_id, movie_id);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], user_session_cookie);

    // cream cererea DELETE pentru stergerea filmului din colectie
    message = compute_delete_request(host, url, NULL, cookies, 1);

    // adaugam header-ul de autorizare cu token JWT
    char auth_header[BUFFLEN * 2];
    snprintf(auth_header, BUFFLEN * 2 - 1, "\nAuthorization: Bearer %s\r", token);
    auth_header[BUFFLEN * 2 - 1] = '\0';

    // inseram header-ul de autorizare in cerere
    char *header_end = strstr(message, "\r\n\r\n");
    if (header_end) {
        int prefix_len = header_end - message;
        char *new_message = calloc(strlen(message) + strlen(auth_header) + 10, sizeof(char));
        
        strncpy(new_message, message, prefix_len);
        strcat(new_message, auth_header);
        strcat(new_message, header_end);
        
        free(message);
        message = new_message;
    }

    // trimitem cererea la server
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);

    response = receive_from_server(sockfd);

        if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Colecția sau filmul nu există!\n");
                else if (strstr(response, "HTTP/1.1 403") != NULL)
                    printf("ERROR: Nu sunteți owner-ul colecției!\n");
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Colecția sau filmul nu există!\n");
        } else if (strstr(response, "HTTP/1.1 403") != NULL) {
            printf("ERROR: Nu sunteți owner-ul colecției!\n");
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Film șters din colecție\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}

void delete_user_cmd(){

    fflush(stdin);
    if (strlen(session_cookie) == 0) {
        printf("ERROR: Not logged in as admin. Please login first.\n");
        return;
    }

    char *message;
    char *response;
    char username[50];

    // citim username-ul utilizatorului de sters
    printf("username = ");
    fflush(stdout);
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    // construim URL-ul pentru stergerea utilizatorului
    char url[BUFFLEN];
    sprintf(url, "/api/v1/tema/admin/users/%s", username);

    // deschidem conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Error opening connection");

    // pregatim cookie-ul de sesiune
    char **cookies = calloc(2, sizeof(char*));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], session_cookie);

    // cream cererea DELETE pentru stergerea utilizatorului
    message = compute_delete_request(host, url, NULL, cookies, 1);

    // trimitem cererea la server
    send_to_server(sockfd, message);
    free(cookies[0]);
    free(cookies);

    // primim raspunsul
    response = receive_from_server(sockfd);

    // procesam raspunsul
    if (strstr(response, "HTTP/1.1 200") == NULL) {
        char *error_json = basic_extract_json_response(response);
        if (error_json) {
            JSON_Value *root_value = json_parse_string(error_json);
            if (root_value) {
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *error_msg = json_object_get_string(root_object, "error");
                if (error_msg)
                    printf("ERROR: %s\n", error_msg);
                else if (strstr(response, "HTTP/1.1 404") != NULL)
                    printf("ERROR: Utilizatorul cu username=%s nu există!\n", username);
                else if (strstr(response, "HTTP/1.1 403") != NULL)
                    printf("ERROR: Nu aveți drepturi de administrator!\n");
                else
                    printf("ERROR: Incomplete/Corrupted information\n");
                json_value_free(root_value);
            }
        } else if (strstr(response, "HTTP/1.1 404") != NULL) {
            printf("ERROR: Utilizatorul cu username=%s nu există!\n", username);
        } else if (strstr(response, "HTTP/1.1 403") != NULL) {
            printf("ERROR: Nu aveți drepturi de administrator!\n");
        } else {
            printf("ERROR: Incomplete/Corrupted information\n");
        }
    } else {
        printf("SUCCESS: Utilizator șters\n");
    }

    // eliberam memoria
    free(message);
    free(response);
    close_connection(sockfd);
    fflush(stdout);
}


int main() {
    memset(session_cookie, 0, BUFFLEN);
    memset(user_session_cookie, 0, BUFFLEN);

    char command[BUFFLEN];
    
    
    while (1) {
        fgets(command, BUFFLEN, stdin);
        
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strstr(command, "login_admin") != NULL) {
            login_admin_cmd();
        } else if (strcmp(command, "add_user") == 0) {
            add_user_cmd();
        } else if (strcmp(command, "get_users") == 0) {
            get_users_cmd();
        } else if (strcmp(command, "logout_admin") == 0) {
            logout_admin_cmd();
        } else if (strcmp(command, "login") == 0) {
            login_cmd();
        } else if (strcmp(command, "logout") == 0) {
            logout_cmd();
        } else if (strcmp(command, "get_access") == 0) {
            get_access_cmd();
        } else if (strcmp(command, "get_movies") == 0) {
            get_movies_cmd();
        } else if (strcmp(command, "add_movie") == 0) {
            add_movie_cmd();
        } else if (strcmp(command, "get_movie") == 0) {
            get_movie_cmd();
        } else if (strcmp(command, "update_movie") == 0) {
            update_movie_cmd();
        } else if (strcmp(command, "delete_movie") == 0) {
            delete_movie_cmd();
        } else if (strcmp(command, "add_collection") == 0) {
            add_collection_cmd();
        } else if (strcmp(command, "get_collections") == 0) {
            get_collections_cmd();
        } else if (strcmp(command, "add_movie_to_collection") == 0) {
            int collection_id;
            int movie_id;

            printf("collection_id = ");
            fflush(stdout);
            char collection_id_str[BUFFLEN];
            fgets(collection_id_str, BUFFLEN, stdin);
            collection_id_str[strcspn(collection_id_str, "\n")] = 0;
            collection_id = atoi(collection_id_str);

            printf("movie_id = ");
            fflush(stdout);
            char movie_id_str[BUFFLEN];
            fgets(movie_id_str, BUFFLEN, stdin);
            movie_id_str[strcspn(movie_id_str, "\n")] = 0;
            movie_id = atoi(movie_id_str);
            add_movie_to_collection_cmd(collection_id, movie_id);
        } else if (strcmp(command, "get_collection") == 0) {
            get_collection_cmd();
        } else if (strcmp(command, "delete_collection") == 0) {
            delete_collection_cmd();
        } else if (strcmp(command, "delete_movie_from_collection") == 0) {
            delete_movie_from_collection_cmd();
        } else if (strcmp(command, "delete_user") == 0) {
            delete_user_cmd();
        } else printf("Unknown command\n");
    }
    
    return 0;
}