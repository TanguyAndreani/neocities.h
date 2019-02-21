#ifndef NEOCITIES_H
#define NEOCITIES_H

#include "apikey.h"

#define BASEURL "https://neocities.org/api/"

/* POST */
#define UPLOAD BASEURL "upload"
#define DELETE BASEURL "delete"

/* GET */
#define LIST BASEURL "list"
#define INFO BASEURL "info"
#define KEY  BASEURL "key"

typedef struct Neocities_ {
    char *buf;
    int size;
    int pos;
} Neocities;

void neocities_init(Neocities * neocities, size_t size)
{
    neocities->buf = malloc(sizeof(char) * size);
    neocities->size = size;
    neocities->pos = 0;

    return;
}

void neocities_destroy(Neocities * neocities)
{
    free(neocities->buf);

    return;
}

void neocities_dump(Neocities * neocities)
{
    int i = 0;

    while (i < neocities->pos) {
        fputc(neocities->buf[i], stderr);
        i++;
    }

    return;
}

size_t write_data(void *buffer, size_t size, size_t nmemb,
                  Neocities * neocities)
{
    int i = 0;
    while (i < nmemb) {

        if (neocities->pos >= neocities->size) {
            neocities->size += nmemb;
            neocities->buf =
                realloc(neocities->buf, sizeof(char) * neocities->size);
        }

        neocities->buf[neocities->pos] = *(char *) (buffer + i);

        (neocities->pos)++;
        i++;
    }

    return i;
}

json_object *neocities_api(const char *apikey, const char *action,
                           const char *params)
{
    Neocities neocities;

    struct curl_slist *headers;

    CURL *curl;

    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    json_object *empty_error =
        json_object_new_string("neocities_api() failed somewhere.");

    json_tokener *tok = json_tokener_new();
    json_object *obj = NULL;

    char url_with_get_params[500];

    headers = NULL;

    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "couldn't initialize an easy handle.\n");
        return empty_error;
    }

    if (strcmp(action, UPLOAD) == 0) {

        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "sendfile");
        curl_mime_filedata(field, params);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "filename");
        curl_mime_data(field, params, CURL_ZERO_TERMINATED);

        headers = curl_slist_append(headers, "Expect:");

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_URL, action);

    } else if (strcmp(action, DELETE) == 0) {

        fprintf(stderr, "DELETE is currently unsupported.\n");
        return empty_error;

    } else if (strcmp(action, INFO) == 0 || strcmp(action, LIST) == 0
               || strcmp(action, KEY) == 0) {

        strcpy(&url_with_get_params[0], action);

        if (strcmp(action, INFO) == 0 && strcmp(params, "") != 0)
            strcat(url_with_get_params, "?sitename=");
        else if (strcmp(action, LIST) == 0 && strcmp(params, "") != 0)
            strcat(url_with_get_params, "?path=");

        strcat(url_with_get_params, params);
        curl_easy_setopt(curl, CURLOPT_URL, url_with_get_params);

    } else {

        fprintf(stderr, "bad action.\n");
        return empty_error;

    }

    char auth_bearer[22 + 32 + 1] = "Authorization: Bearer ";
    strcat(auth_bearer, apikey);

    neocities_init(&neocities, 700);

    headers = curl_slist_append(headers, auth_bearer);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ((Neocities *) & neocities));

    if (curl_easy_perform(curl) != CURLE_OK) {
        fprintf(stderr, "curl failed.\n");
        return empty_error;
    }

    curl_slist_free_all(headers);
    curl_mime_free(form);
    curl_easy_cleanup(curl);

    obj =
        json_tokener_parse_ex(tok, (char *) neocities.buf, neocities.pos);

    neocities_destroy(&neocities);

    return obj;
}

#endif
