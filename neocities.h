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

enum neocities_error {
    NEOCITIES_OK,
    NEOCITIES_ERR_CURL_GLOBAL_INIT,
    NEOCITIES_ERR_JSON_TOKENER_NEW,
    NEOCITIES_ERR_CURL_EASY_INIT,
    NEOCITIES_ERR_CURL_MIME_INIT,
    NEOCITIES_ERR_CURL_MIME_ADDPART,
    NEOCITIES_ERR_CURL_MIME_NAME,
    NEOCITIES_ERR_CURL_MIME_FILEDATA,
    NEOCITIES_ERR_CURL_MIME_DATA,
    NEOCITIES_ERR_CURL_SLIST_APPEND,
    NEOCITIES_ERR_CURL_EASY_SETOPT,
    NEOCITIES_ERR_UNSUPPORTED_DELETE,
    NEOCITIES_ERR_BAD_ACTION,
    NEOCITIES_ERR_NEOCITIES_INIT,
    NEOCITIES_ERR_CURL_EASY_PERFORM,
    NEOCITIES_ERR_JSON_TOKENER_PARSE
};

typedef struct Neocities_ {
    char *buf;
    int size;
    int pos;
} Neocities;

int neocities_init(Neocities * neocities, size_t size)
{
    neocities->buf = malloc(sizeof(char) * size);

    if (neocities->buf == NULL)
        return 1;

    neocities->size = size;
    neocities->pos = 0;

    return 0;
}

void neocities_destroy(Neocities * neocities)
{
    free(neocities->buf);
    neocities->buf = NULL;

    neocities->size = 0;
    neocities->pos = 0;

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
            if (neocities->buf == NULL)
                return neocities->size;
        }

        neocities->buf[neocities->pos] = *(char *) (buffer + i);

        (neocities->pos)++;
        i++;
    }

    return i;
}

enum neocities_error neocities_api(const char *apikey, const char *action,
                                   const char *params, json_object ** obj)
{
    Neocities neocities;

    struct curl_slist *headers = NULL;

    CURL *curl;

    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    json_tokener *tok = json_tokener_new();

    char url_with_get_params[500];

    char auth_bearer[22 + 32 + 1] = "Authorization: Bearer ";
    strcat(auth_bearer, apikey);

    if (tok == NULL)
        return NEOCITIES_ERR_JSON_TOKENER_NEW;

    curl = curl_easy_init();

    if (curl == NULL)
        return NEOCITIES_ERR_CURL_EASY_INIT;

    if (strcmp(action, UPLOAD) == 0) {

        form = curl_mime_init(curl);

        if (form == NULL)
            return NEOCITIES_ERR_CURL_MIME_INIT;

        field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, "sendfile") != CURLE_OK)
            return NEOCITIES_ERR_CURL_MIME_NAME;

        if (curl_mime_filedata(field, params) != CURLE_OK)
            return NEOCITIES_ERR_CURL_MIME_FILEDATA;

        field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, "filename") != CURLE_OK)
            return NEOCITIES_ERR_CURL_MIME_NAME;

        if (curl_mime_data(field, params, CURL_ZERO_TERMINATED) !=
            CURLE_OK)
            return NEOCITIES_ERR_CURL_MIME_DATA;

        field = NULL;

        headers = curl_slist_append(headers, "Expect:");

        if (headers == NULL)
            return NEOCITIES_ERR_CURL_SLIST_APPEND;

        if (curl_easy_setopt(curl, CURLOPT_MIMEPOST, form) != CURLE_OK)
            return NEOCITIES_ERR_CURL_EASY_SETOPT;

        if (curl_easy_setopt(curl, CURLOPT_URL, action) != CURLE_OK)
            return NEOCITIES_ERR_CURL_EASY_SETOPT;

    } else if (strcmp(action, DELETE) == 0) {

        return NEOCITIES_ERR_UNSUPPORTED_DELETE;

    } else if (strcmp(action, INFO) == 0 || strcmp(action, LIST) == 0
               || strcmp(action, KEY) == 0) {

        strcpy(&url_with_get_params[0], action);

        if (strcmp(action, INFO) == 0 && strcmp(params, "") != 0)
            strcat(url_with_get_params, "?sitename=");
        else if (strcmp(action, LIST) == 0 && strcmp(params, "") != 0)
            strcat(url_with_get_params, "?path=");

        strcat(url_with_get_params, params);

        if (curl_easy_setopt(curl, CURLOPT_URL, url_with_get_params) !=
            CURLE_OK)
            return NEOCITIES_ERR_CURL_EASY_SETOPT;

    } else {

        return NEOCITIES_ERR_BAD_ACTION;

    }

    if (neocities_init(&neocities, 700) != 0)
        return NEOCITIES_ERR_NEOCITIES_INIT;

    headers = curl_slist_append(headers, auth_bearer);

    if (headers == NULL)
        return NEOCITIES_ERR_CURL_SLIST_APPEND;

    if (curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) != CURLE_OK)
        return NEOCITIES_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data) !=
        CURLE_OK)
        return NEOCITIES_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt
        (curl, CURLOPT_WRITEDATA, ((Neocities *) & neocities)))
        return NEOCITIES_ERR_CURL_EASY_SETOPT;

    if (curl_easy_perform(curl) != CURLE_OK)
        return NEOCITIES_ERR_CURL_EASY_PERFORM;

    curl_slist_free_all(headers);
    curl_mime_free(form);
    curl_easy_cleanup(curl);

    *obj =
        json_tokener_parse_ex(tok, (char *) neocities.buf, neocities.pos);

    neocities_dump(&neocities);
    neocities_destroy(&neocities);

    if (*obj == NULL)
        return NEOCITIES_ERR_JSON_TOKENER_PARSE;

    return NEOCITIES_OK;
}

#endif
