#pragma once

/*
 * the following constants are not to be used elsewhere than in this 
 * file, they are undefined at the end of this header
 */

#define BASEURL "https://neocities.org/api/"

/* POST */
#define UPLOAD_URL BASEURL "upload"
#define DELETE_URL BASEURL "delete"

/* GET */
#define LIST_URL BASEURL "list"
#define INFO_URL BASEURL "info"
#define KEY_URL  BASEURL "key"


/* those are part of public api */

#define neocities_global_init() \
    ((curl_global_init(CURL_GLOBAL_SSL) != 0) \
    ? NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT \
    : 0)

#define neocities_global_cleanup() \
    curl_global_cleanup();

/* to use with neocities_api[_ex] */
enum neocities_action {
    upload,
    delete,
    list,
    info,
    key
};

/* this stands for LOW LEVEL ERROR */

enum neocities_low_level_error {
    NEOCITIES_LLVL_OK = 0,

    NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT,

    /*
     * the following members are not to be used elsewhere than in this 
     * file; you can throw them in neocities_print_error_message().
     */

    NEOCITIES_LLVL_ERR_JSON_TOKENER_NEW,
    NEOCITIES_LLVL_ERR_CURL_EASY_INIT,
    NEOCITIES_LLVL_ERR_CURL_MIME_INIT,
    NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART,
    NEOCITIES_LLVL_ERR_CURL_MIME_NAME,
    NEOCITIES_LLVL_ERR_CURL_MIME_FILEDATA,
    NEOCITIES_LLVL_ERR_CURL_MIME_DATA,
    NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND,
    NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT,
    NEOCITIES_LLVL_ERR_CURL_EASY_PERFORM,
    NEOCITIES_LLVL_ERR_UNSUPPORTED_DELETE,
    NEOCITIES_LLVL_ERR_BAD_ACTION,
    NEOCITIES_LLVL_ERR_CURL_BUF_INIT,
    NEOCITIES_LLVL_ERR_JSON_TOKENER_PARSE,
    NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON,
    NEOCITIES_LLVL_ERR_RECEIVED_INVALID_JSON,
    NEOCITIES_LLVL_ERR_MALLOC_FAIL,
    NEOCITIES_LLVL_ERR_EXPECTED_STRING,
    NEOCITIES_LLVL_ERR_EXPECTED_BOOL,
    NEOCITIES_LLVL_ERR_EXPECTED_INT,
    NEOCITIES_LLVL_ERR_EXPECTED_ARRAY,
    NEOCITIES_LLVL_ERR_EXPECTED_STRING_OR_NULL,
    NEOCITIES_LLVL_ERR_EXPECTED_OBJECT,
    NEOCITIES_LLVL_ERR_RECEIVED_SOMETHING_ELSE
};

/*
 * neocities_*_ are structs that holds all the useful data
 *
 * see neocities_res for a more general type
 */

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

/*
 * you might want to use those in your code
 */

enum neocities_api_level_error {
    SITE_NOT_FOUND,
    UNSUPPORTED_ERROR
};

struct neocities_error_ {
    enum neocities_api_level_error type;
};

/*
 * NEOCITIES_NO_TYPE_YET works for successful request that didn't have an
 * info or list field such as the upload call
 */

enum neocities_res_data {
    NEOCITIES_NO_TYPE_YET,
    NEOCITIES_INFO,
    NEOCITIES_LIST,
    NEOCITIES_ERROR = -1
};

typedef struct neocities_res_ {
    int result;
    enum neocities_res_data type;

    union {
        struct neocities_list_ list;
        struct neocities_info_ info;
        struct neocities_error_ error;
    } data;
} neocities_res;

/*
 * this is an internal structure that shouldn't be used elsewhere.
 */

typedef struct curl_buffer_ {
    char *buf;

    int size;
    int pos;
} curl_buffer;

int curl_buffer_init(curl_buffer * curl_buf, size_t size)
{
    curl_buf->buf = malloc(sizeof(char) * size);

    if (curl_buf->buf == NULL)
        return 1;

    curl_buf->size = size;
    curl_buf->pos = 0;

    return 0;
}

void curl_buffer_destroy(curl_buffer * curl_buf)
{
    free(curl_buf->buf);
    curl_buf->buf = NULL;

    curl_buf->size = 0;
    curl_buf->pos = 0;

    return;
}

void curl_buffer_dump(curl_buffer * curl_buf)
{
    int i = 0;

    while (i < curl_buf->pos) {
        fputc(curl_buf->buf[i], stderr);
        i++;
    }

    return;
}

size_t write_data(void *buffer, size_t size, size_t nmemb,
                  curl_buffer * curl_buf)
{
    int i = 0;
    while (i < nmemb) {

        if (curl_buf->pos >= curl_buf->size) {

            curl_buf->size += nmemb;

            curl_buf->buf =
                realloc(curl_buf->buf, sizeof(char) * curl_buf->size);

            if (curl_buf->buf == NULL)
                return curl_buf->size;

        }

        curl_buf->buf[curl_buf->pos] = *(char *) (buffer + i);

        (curl_buf->pos)++;
        i++;
    }

    return i;
}

/*
 * perform an api call and parse the resulting json into a json_object
 */

enum neocities_low_level_error neocities_api(const char *apikey,
                                             enum neocities_action action,
                                             const char *params,
                                             json_object ** jobj)
{
    curl_buffer curl_buf;

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

    if (action == upload) {

        form = curl_mime_init(curl);

        if (form == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_INIT;

        field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, params) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_NAME;

        if (curl_mime_filedata(field, params) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_FILEDATA;

        /* seems useless :( */

        /*field = curl_mime_addpart(form);

        if (field == NULL)
            return NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART;

        if (curl_mime_name(field, "filename") != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_NAME;

        if (curl_mime_data(field, params, CURL_ZERO_TERMINATED) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_MIME_DATA;*/

        field = NULL;

        headers = curl_slist_append(headers, "Expect:");

        if (headers == NULL)
            return NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND;

        if (curl_easy_setopt(curl, CURLOPT_MIMEPOST, form) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

        if (curl_easy_setopt(curl, CURLOPT_URL, UPLOAD_URL) != CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    } else if (action == delete) {

        return NEOCITIES_LLVL_ERR_UNSUPPORTED_DELETE;

    } else if (action == info || action == list || action == key) {

        memset(&url_with_get_params[0], '\0', 500);

        /* appends p to our url string */
#define ap(p) \
            strcat(url_with_get_params, p)

        if (action == key) {

            ap(KEY_URL);

        } else if (action == info) {

            ap(INFO_URL);

            if (strcmp(params, "") != 0)
                ap("?sitename=");

        } else if (action == list) {

            ap(LIST_URL);

            if (strcmp(params, "") != 0)
                ap("?path=");

        }

        ap(params);

#undef ap

        if (curl_easy_setopt(curl, CURLOPT_URL, url_with_get_params) !=
            CURLE_OK)
            return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    } else {

        return NEOCITIES_LLVL_ERR_BAD_ACTION;

    }

    if (curl_buffer_init(&curl_buf, 700) != 0)
        return NEOCITIES_LLVL_ERR_CURL_BUF_INIT;

    headers = curl_slist_append(headers, auth_bearer);

    if (headers == NULL)
        return NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND;

    if (curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) != CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data) != CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_setopt
        (curl, CURLOPT_WRITEDATA, ((curl_buffer *) & curl_buf)))
        return NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT;

    if (curl_easy_perform(curl) != CURLE_OK)
        return NEOCITIES_LLVL_ERR_CURL_EASY_PERFORM;

    curl_slist_free_all(headers);
    curl_mime_free(form);
    curl_easy_cleanup(curl);

    /* our function write_data set curl_buf.pos to the length of the 
       received data */
    *jobj = json_tokener_parse_ex(tok, (char *) curl_buf.buf, curl_buf.pos);

    /* https://groups.google.com/forum/#!topic/json-c/CMvkXKXqtWs */
    if (tok->char_offset != curl_buf.pos) {
        json_tokener_free(tok);
        return NEOCITIES_LLVL_ERR_RECEIVED_INVALID_JSON;
    }

    json_tokener_free(tok);

#ifdef NEOCITIES_DEBUG
    curl_buffer_dump(&curl_buf);
#endif
    curl_buffer_destroy(&curl_buf);

    if (*jobj == NULL)
        return NEOCITIES_LLVL_ERR_JSON_TOKENER_PARSE;

    if (json_object_get_type(*jobj) != json_type_object)
        return NEOCITIES_LLVL_ERR_EXPECTED_OBJECT;

    return NEOCITIES_LLVL_OK;
}

/*
 * those are functions used by neocities_destroy()
 */

static void neocities_destroy_list(neocities_res * res)
{
    int i = 0;

    res->data.list.length = -1;

    for (; i < res->data.list.length; i++)
        free(res->data.list.files[i].path);

    if (res->data.list.files != NULL)
        free(res->data.list.files);

    return;
}

static void neocities_destroy_info(neocities_res * res)
{
    int i = 0;

    res->data.info.views = -1;
    res->data.info.hits = -1;
    res->data.info.created_at = (time_t)0;
    res->data.info.last_updated = (time_t)0;

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

static void neocities_destroy_error(neocities_res * res)
{
    res->data.error.type = -1;

    return;
}

/*
 * you should use this to free memory AND reset values to their defaults
 */

void neocities_destroy(neocities_res * res)
{

    switch (res->type) {
    case NEOCITIES_NO_TYPE_YET:
        break;
    case NEOCITIES_INFO:
        neocities_destroy_info(res);
        break;
    case NEOCITIES_LIST:
        neocities_destroy_list(res);
        break;
    case NEOCITIES_ERROR:
        neocities_destroy_error(res);
        break;
    }

    res->type = NEOCITIES_NO_TYPE_YET;

    return;
}

/*
 * i took the guess approach here, this function determines which type
 * is the most likely and tries to fill res fields as much as possible
 *
 * fields are reset to their default values as soon as the type is
 * recognized
 *
 * the function will exit in case their is a type error in the json
 */

enum neocities_low_level_error neocities_json_to_struct(json_object * jobj,
                                                        neocities_res * res)
{

    json_object_iter jobj_iter, jobj_iter_info, jobj_iter_list;

    int seen_result = 0;

    int i = 0;

    char *tmp_string = NULL;
    array_list *tmp_array = NULL;
    json_object *tmp_object = NULL;

    res->type = NEOCITIES_NO_TYPE_YET;

    json_object_object_foreachC(jobj, jobj_iter) {

/* undefined on same level */
#define is_not(jobj, type) (json_object_get_type(jobj) != type)

#define key_is(s) (strcmp(jobj_iter.key, s) == 0)

        if (jobj_iter.key == NULL)
            continue;

        if key_is
            ("error_type") {

            res->result = -1;     // in case wasn't sent in usual order
            res->type = NEOCITIES_ERROR;
            res->data.error.type = UNSUPPORTED_ERROR;

            if is_not
                (jobj_iter.val, json_type_string)
                    return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

            tmp_string = json_object_get_string(jobj_iter.val);

#define error_is(s) (tmp_string != NULL && (strcmp(tmp_string, s) == 0))

            if error_is
                ("site_not_found")
                    res->data.error.type = SITE_NOT_FOUND;

#undef error_is

            break;

        } else if key_is
            ("result") {

            if (seen_result == 1)
                return NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON;
            else
                seen_result = 1;

            if is_not
                (jobj_iter.val, json_type_string)
                    return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

            tmp_string = json_object_get_string(jobj_iter.val);

#define result_is(s) (tmp_string != NULL && (strcmp(tmp_string, s) == 0))

            if result_is
                ("success")
                    res->result = 0;
            else
                res->result = -1;

#undef result_is

        } else if (key_is("info")
                   && res->type == NEOCITIES_NO_TYPE_YET) {

            res->type = NEOCITIES_INFO;

            res->data.info.sitename = NULL;
            res->data.info.views = -1;
            res->data.info.hits = -1;
            res->data.info.created_at = (time_t)0;
            res->data.info.last_updated = (time_t)0;
            res->data.info.domain = NULL;
            res->data.info.tags[0] = NULL;

            if is_not
                (jobj_iter.val, json_type_object)
                    return NEOCITIES_LLVL_ERR_EXPECTED_OBJECT;

            json_object_object_foreachC(jobj_iter.val, jobj_iter_info) {

#define INFO_key_is(s) (strcmp(jobj_iter_info.key, s) == 0)

                if (jobj_iter_info.key == NULL)
                    continue;

                if INFO_key_is
                    ("views") {

                    if is_not
                        (jobj_iter_info.val, json_type_int)
                            return NEOCITIES_LLVL_ERR_EXPECTED_INT;

                    res->data.info.views =
                        json_object_get_int(jobj_iter_info.val);

                } else if INFO_key_is
                    ("hits") {

                    if is_not
                        (jobj_iter_info.val, json_type_int)
                            return NEOCITIES_LLVL_ERR_EXPECTED_INT;

                    res->data.info.hits =
                        json_object_get_int(jobj_iter_info.val);

                } else if INFO_key_is
                    ("sitename") {

                    if is_not
                        (jobj_iter_info.val, json_type_string)
                            return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

                    res->data.info.sitename =
                        strdup(json_object_get_string(jobj_iter_info.val));

                    if (res->data.info.sitename == NULL)
                        return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

                } else if INFO_key_is
                    ("domain") {

                    if (is_not(jobj_iter_info.val, json_type_string)
                        && is_not(jobj_iter_info.val, json_type_null))
                        return NEOCITIES_LLVL_ERR_EXPECTED_STRING_OR_NULL;

                    tmp_string = json_object_get_string(jobj_iter_info.val);

                    if (tmp_string != NULL) {
                        res->data.info.domain = strdup(tmp_string);

                        if (res->data.info.domain == NULL)
                            return NEOCITIES_LLVL_ERR_MALLOC_FAIL;
                    }

                } else if INFO_key_is
                    ("created_at") {

                    if is_not
                        (jobj_iter_info.val, json_type_string)
                            return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

                    tmp_string = json_object_get_string(jobj_iter_info.val);

                    rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                       &res->data.info.created_at, true);

                } else if INFO_key_is
                    ("last_updated") {

                    if (is_not(jobj_iter_info.val, json_type_string)
                        && is_not(jobj_iter_info.val, json_type_null))
                        return NEOCITIES_LLVL_ERR_EXPECTED_STRING_OR_NULL;

                    tmp_string = json_object_get_string(jobj_iter_info.val);

                    if (tmp_string != NULL)
                        rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                           &res->data.info.last_updated, true);

                } else if INFO_key_is
                    ("tags") {

                    if is_not
                        (jobj_iter_info.val, json_type_array)
                            return NEOCITIES_LLVL_ERR_EXPECTED_ARRAY;

                    tmp_array = json_object_get_array(jobj_iter_info.val);

                    i = 0;

                    for (; i < tmp_array->length; i++) {

                        if is_not
                            ((tmp_array->array)[i], json_type_string)
                            return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

                        res->data.info.tags[i] =
                            strdup(json_object_get_string
                                   ((tmp_array->array)[i]));

                        if (res->data.info.tags[i] == NULL)
                            return NEOCITIES_LLVL_ERR_MALLOC_FAIL;


                    }

                    res->data.info.tags[i] = NULL;
                    }
#undef INFO_key_is

            }

            break;              /* return when all the fields are processed */

        } else if (key_is("files")
                   && res->type == NEOCITIES_NO_TYPE_YET) {

            res->type = NEOCITIES_LIST;

            res->data.list.files = NULL;
            res->data.list.length = 0;

            if is_not
                (jobj_iter.val, json_type_array)
                    return NEOCITIES_LLVL_ERR_EXPECTED_ARRAY;

            res->data.list.length = json_object_array_length(jobj_iter.val);

            res->data.list.files =
                malloc(sizeof(struct neocities_file_) * res->data.list.length);

            if (res->data.list.files == NULL)
                return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

            tmp_array = json_object_get_array(jobj_iter.val);

            for (i = 0; i < res->data.list.length; i++) {

                (res->data.list.files)[i].path = NULL;
                (res->data.list.files)[i].is_directory = -1;
                (res->data.list.files)[i].size = -1;
                (res->data.list.files)[i].updated_at = (time_t)0;

#define LIST_key_is(s) (strcmp(jobj_iter_list.key, s) == 0)

                json_object_object_foreachC(tmp_array->array[i],
                                            jobj_iter_list) {

                    if (jobj_iter_list.key == NULL)
                        continue;

                    if LIST_key_is
                        ("path") {

                        if is_not
                            (jobj_iter_list.val, json_type_string)
                                return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

                        (res->data.list.files)[i].path =
                            strdup(json_object_get_string(jobj_iter_list.val));

                        if ((res->data.list.files)[i].path == NULL)
                            return NEOCITIES_LLVL_ERR_MALLOC_FAIL;

                    } else if LIST_key_is
                        ("updated_at") {

                        if is_not
                            (jobj_iter_list.val, json_type_string)
                                return NEOCITIES_LLVL_ERR_EXPECTED_STRING;

                        tmp_string = json_object_get_string
                            (jobj_iter_list.val);

                        if (tmp_string == NULL)
                            continue;

                        rfc5322_date_parse(tmp_string, strlen(tmp_string),
                                           &(res->data.list.
                                             files)[i].updated_at, true);

                    } else if LIST_key_is
                        ("size") {

                        if is_not
                            (jobj_iter_list.val, json_type_int)
                                return NEOCITIES_LLVL_ERR_EXPECTED_INT;

                        (res->data.list.files)[i].size =
                            json_object_get_int(jobj_iter_list.val);

                    } else if LIST_key_is
                        ("is_directory") {

                        if is_not
                            (jobj_iter_list.val, json_type_boolean)
                                return NEOCITIES_LLVL_ERR_EXPECTED_BOOL;

                        /* TRUE is defined by libjson-c */
                        (res->data.list.files)[i].is_directory =
                            (json_object_get_boolean(jobj_iter_list.val) ==
                             ((json_bool)1) ? 1 : 0);

                        }

                    /*else {

                       return NEOCITIES_LLVL_ERR_UNSUPPORTED_JSON;

                       } */

                }

#undef LIST_key_is

            }                   /* for each file in list */

            break;              /* return when all the files are processed */

        }
        /* if key_is() ... */
#undef key_is
#undef is_not

    }                           /* for each json object */

    return NEOCITIES_LLVL_OK;
}

/* simple function used for convenience */

enum neocities_res_data action_to_res_data_type(enum neocities_action action)
{
    switch (action) {
    case upload:
        return NEOCITIES_NO_TYPE_YET;
    case list:
        return NEOCITIES_LIST;
    case info:
        return NEOCITIES_INFO;
    }
}

/* the function you probably looked for */

enum neocities_low_level_error neocities_api_ex(const char *apikey,
                                                enum neocities_action
                                                action, const char *params,
                                                neocities_res * res)
{
    int err;

    json_object *jobj = NULL;

    if ((err =
         neocities_api(apikey, action, params, &jobj)) != NEOCITIES_LLVL_OK)
        return err;

    err = neocities_json_to_struct(jobj, res);

    if (res->result == 0 && res->type != action_to_res_data_type(action))
        return NEOCITIES_LLVL_ERR_RECEIVED_SOMETHING_ELSE;

    json_object_put(jobj);

    return err;
}

#undef UPLOAD_URL
#undef DELETE_URL

#undef LIST_URL
#undef INFO_URL
#undef KEY_URL

#undef BASEURL
