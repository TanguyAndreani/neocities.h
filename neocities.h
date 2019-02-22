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

enum neocities_low_level_error {
    NEOCITIES_LLVL_OK,
    NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT,
    NEOCITIES_LLVL_ERR_JSON_TOKENER_NEW,
    NEOCITIES_LLVL_ERR_CURL_EASY_INIT,
    NEOCITIES_LLVL_ERR_CURL_MIME_INIT,
    NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART,
    NEOCITIES_LLVL_ERR_CURL_MIME_NAME,
    NEOCITIES_LLVL_ERR_CURL_MIME_FILEDATA,
    NEOCITIES_LLVL_ERR_CURL_MIME_DATA,
    NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND,
    NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT,
    NEOCITIES_LLVL_ERR_UNSUPPORTED_DELETE,
    NEOCITIES_LLVL_ERR_BAD_ACTION,
    NEOCITIES_LLVL_ERR_NEOCITIES_INIT,
    NEOCITIES_LLVL_ERR_CURL_EASY_PERFORM,
    NEOCITIES_LLVL_ERR_JSON_TOKENER_PARSE,
    NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON,
    NEOCITIES_LLVL_ERR_RECEIVED_INVALID_JSON,
    NEOCITIES_LLVL_ERR_NO_SUCCESS,
    NEOCITIES_LLVL_ERR_MALLOC_FAIL
};

/* Not prefixed for convenience */
enum neocities_api_level_error {
    SITE_NOT_FOUND,
    UNSUPPORTED_ERROR
};

typedef struct Neocities_ {
    char *buf;

    int size;
    int pos;
} Neocities;

struct neocities_info_ {
    char *sitename;             /* default: NULL */
    int32_t hits;               /* default: -1 */
    int32_t views;
    time_t created_at;          /* default: 0 */
    time_t last_updated;
    char *domain;

    /* you're not allowed to have more tags anyway */
    char *tags[20];
};

struct neocities_list_ {
    struct neocities_file_ *files;
    int length;
};

struct neocities_file_ {
    char *path;
    int is_directory;
    int size;
    time_t updated_at;
};

struct neocities_error_ {
    int type;
};

typedef struct neocities_res_ {
    int result;

    union {
        struct neocities_list_ list;
        struct neocities_info_ info;
        struct neocities_error_ error;
    } data;
} neocities_res;

enum neocities_res_data {
    NEOCITIES_NO_TYPE_YET,
    NEOCITIES_INFO_STRUCT,
    NEOCITIES_LIST_STRUCT,
    NEOCITIES_ERROR_STRUCT
};

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

enum neocities_low_level_error neocities_api(const char *apikey,
                                             const char *action,
                                             const char *params,
                                             json_object ** jobj)
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
        return NEOCITIES_LLVL_ERR_JSON_TOKENER_NEW;

    curl = curl_easy_init();

    if (curl == NULL)
        return NEOCITIES_LLVL_ERR_CURL_EASY_INIT;

    if (action != NULL && strcmp(action, UPLOAD) == 0) {

        form = curl_mime_init(curl);

        if (form == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_INIT;

        field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, "sendfile") != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_NAME;

        if (curl_mime_filedata(field, params) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_FILEDATA;

        field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, "filename") != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_NAME;

        if (curl_mime_data(field, params, CURL_ZERO_TERMINATED) !=
            CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_DATA;

        field = NULL;

        headers = curl_slist_append(headers, "Expect:");

        if (headers == NULL)
            return NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND;

        if (curl_easy_setopt(curl, CURLOPT_MIMEPOST, form) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

        if (curl_easy_setopt(curl, CURLOPT_URL, action) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    } else if (action != NULL && strcmp(action, DELETE) == 0) {

        return NEOCITIES_LLVL_ERR_UNSUPPORTED_DELETE;

    } else if (action != NULL
               && (strcmp(action, INFO) == 0 || strcmp(action, LIST) == 0
                   || strcmp(action, KEY) == 0)) {

        strcpy(&url_with_get_params[0], action);

        if (strcmp(action, INFO) == 0 && strcmp(params, "") != 0)

            strcat(url_with_get_params, "?sitename=");

        else if (strcmp(action, LIST) == 0 && strcmp(params, "") != 0)

            strcat(url_with_get_params, "?path=");

        strcat(url_with_get_params, params);

        if (curl_easy_setopt(curl, CURLOPT_URL, url_with_get_params) !=
            CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    } else {

        return NEOCITIES_LLVL_ERR_BAD_ACTION;

    }

    if (neocities_init(&neocities, 700) != 0)
        return NEOCITIES_LLVL_ERR_NEOCITIES_INIT;

    headers = curl_slist_append(headers, auth_bearer);

    if (headers == NULL)
        return NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND;

    if (curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) != CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data) !=
        CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt
        (curl, CURLOPT_WRITEDATA, ((Neocities *) & neocities)))
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_perform(curl) != CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_PERFORM;

    curl_slist_free_all(headers);
    curl_mime_free(form);
    curl_easy_cleanup(curl);

    *jobj =
        json_tokener_parse_ex(tok, (char *) neocities.buf, neocities.pos);

    /* https://groups.google.com/forum/#!topic/json-c/CMvkXKXqtWs */
    if (tok->char_offset != neocities.pos) {
        json_tokener_free(tok);
        return NEOCITIES_LLVL_ERR_RECEIVED_INVALID_JSON;
    }

    json_tokener_free(tok);

#ifdef NEOCITIES_DEBUG
    neocities_dump(&neocities);
#endif
    neocities_destroy(&neocities);

    if (*jobj == NULL)
        return NEOCITIES_LLVL_ERR_JSON_TOKENER_PARSE;

    if (json_object_get_type(*jobj) != json_type_object)
        return NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON;

    return NEOCITIES_LLVL_OK;
}

void neocities_destroy_list(neocities_res * res)
{
    int i = 0;

    for (; i < res->data.list.length; i++)
        free(res->data.list.files[i].path);

    if (res->data.list.files != NULL)
        free(res->data.list.files);

    return;
}

void neocities_destroy_info(neocities_res * res)
{
    int i = 0;

    while (res->data.info.tags[i] != NULL) {
        free(res->data.info.tags[i]);
        i++;
    }

    if (res->data.info.sitename != NULL)
        free(res->data.info.sitename);

    if (res->data.info.domain != NULL)
        free(res->data.info.domain);

    return;
}

void neocities_destroy_error(neocities_res *res) {
    res->data.error.type = -1;

    return;
}

enum neocities_low_level_error neocities_json_to_struct(json_object * jobj,
                                              neocities_res * res)
{

    json_object_iter jobj_iter, jobj_iter_info, jobj_iter_list;

    int seen_result = 0;

    int i = 0;

    char *tmp_string = NULL;
    array_list *tmp_array = NULL;
    json_object *tmp_object = NULL;

    enum neocities_res_data current_type = NEOCITIES_NO_TYPE_YET;

    json_object_object_foreachC(jobj, jobj_iter) {

        if (jobj_iter.key != NULL
            && strcmp(jobj_iter.key, "error_type") == 0) {

            tmp_string = json_object_get_string(jobj_iter.val);

            if (tmp_string != NULL && strcmp(tmp_string,
                                             "site_not_found") == 0)

                res->data.error.type = SITE_NOT_FOUND;

            else

                res->data.error.type = UNSUPPORTED_ERROR;


        } else if (jobj_iter.key != NULL
                   && strcmp(jobj_iter.key, "result") == 0) {

            if (seen_result == 1)
                return NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON;
            else
                seen_result = 1;

            tmp_string = json_object_get_string(jobj_iter.val);

            if (tmp_string != NULL && strcmp
                (json_object_get_string(jobj_iter.val), "success") != 0) {

                res->result = -1;

                continue;

            } else {

                res->result = 0;

            }

        } else if (strcmp(jobj_iter.key, "info") == 0
                   && current_type == NEOCITIES_NO_TYPE_YET) {

            current_type = NEOCITIES_INFO_STRUCT;

            res->data.info.sitename = NULL;
            res->data.info.views = -1;
            res->data.info.hits = -1;
            res->data.info.created_at = (time_t) 0;
            res->data.info.last_updated = (time_t) 0;
            res->data.info.domain = NULL;
            res->data.info.tags[0] = NULL;

            json_object_object_foreachC(jobj_iter.val, jobj_iter_info) {

                if (strcmp(jobj_iter_info.key, "views") == 0) {

                    res->data.info.views =
                        json_object_get_int(jobj_iter_info.val);

                } else if (strcmp(jobj_iter_info.key, "hits") == 0) {

                    res->data.info.hits =
                        json_object_get_int(jobj_iter_info.val);

                } else if (strcmp(jobj_iter_info.key, "sitename") == 0) {

                    res->data.info.sitename =
                        strdup(json_object_get_string(jobj_iter_info.val));

                    if (res->data.info.sitename == NULL)
                        return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

                } else if (strcmp(jobj_iter_info.key, "domain") == 0) {

                    res->data.info.domain =
                        strdup(json_object_get_string(jobj_iter_info.val));

                    if (res->data.info.domain == NULL)
                        return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

                } else if (strcmp(jobj_iter_info.key, "created_at") == 0) {

                    tmp_string = json_object_get_string
                        (jobj_iter_info.val);

                    rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                       &res->data.info.created_at, true);

                } else if (strcmp(jobj_iter_info.key, "last_updated") == 0) {

                    tmp_string = json_object_get_string
                        (jobj_iter_info.val);

                    rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                       &res->data.info.last_updated, true);

                } else if (strcmp(jobj_iter_info.key, "tags") == 0) {

                    tmp_array = json_object_get_array(jobj_iter_info.val);

                    for (i = 0; i < tmp_array->length; i++) {

                        res->data.info.tags[i] =
                            strdup(json_object_get_string
                                   ((tmp_array->array)[i]));

                        if (res->data.info.tags[i] == NULL)
                            return NEOCITIES_LLVL_ERR_MALLOC_FAIL;


                    }

                    res->data.info.tags[i] = NULL;
                }

            }

            break;

        } else if (strcmp(jobj_iter.key, "files") == 0
                   && current_type == NEOCITIES_NO_TYPE_YET) {

            current_type = NEOCITIES_LIST_STRUCT;

            res->data.list.files = NULL;
            res->data.list.length = 0;
            res->data.list.length =
                json_object_array_length(jobj_iter.val);

            res->data.list.files =
                malloc(sizeof(struct neocities_file_) *
                       res->data.list.length);

            if (res->data.list.files == NULL)
                return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

            tmp_array = json_object_get_array(jobj_iter.val);

            for (i = 0; i < res->data.list.length; i++) {

                (res->data.list.files)[i].path = NULL;
                (res->data.list.files)[i].is_directory = -1;
                (res->data.list.files)[i].size = -1;
                (res->data.list.files)[i].updated_at = (time_t) 0;

                json_object_object_foreachC(tmp_array->array[i],
                                            jobj_iter_list) {

                    if (strcmp(jobj_iter_list.key, "path") == 0) {

                        (res->data.list.files)[i].path =
                            strdup(json_object_get_string
                                   (jobj_iter_list.val));

                        if ((res->data.list.files)[i].path == NULL)
                            return NEOCITIES_LLVL_ERR_MALLOC_FAIL;


                    } else if (strcmp(jobj_iter_list.key, "updated_at") ==
                               0) {

                        tmp_string = json_object_get_string
                            (jobj_iter_list.val);

                        rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                           &(res->data.list.files)[i].
                                           updated_at, true);

                    } else if (strcmp(jobj_iter_list.key, "size") == 0) {

                        (res->data.list.files)[i].size =
                            json_object_get_int(jobj_iter_list.val);

                    } else if (strcmp(jobj_iter_list.key, "is_directory")
                               == 0) {

                        (res->data.list.files)[i].is_directory =
                            (json_object_get_boolean(jobj_iter_list.val) ==
                             TRUE ? 1 : 0);

                    } else {

                        /* ignore ipfs thing */

                        continue;
                    }

                }

            }

            break;
        } else {

            if (res->result == 0) // UPLOAD

                return NEOCITIES_LLVL_OK;

            return NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON;

        }
    }

    return NEOCITIES_LLVL_OK;
}

enum neocities_low_level_error neocities_api_ex(const char *apikey,
                                                const char *action,
                                                const char *params,
                                                neocities_res * res)
{
    int err;

    json_object *jobj = NULL;

    if ((err =
         neocities_api(apikey, action, params, &jobj)) != NEOCITIES_LLVL_OK)
        return err;

    err = neocities_json_to_struct(jobj, res);

    json_object_put(jobj);

    return err;
}

#endif
