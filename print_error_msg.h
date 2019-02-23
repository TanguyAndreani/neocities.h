#define M(m) \
  fprintf(stderr, "error %d: " m, err); \
  break

void neocities_print_error_message(enum neocities_low_level_error err)
{
    switch (err) {

    case NEOCITIES_LLVL_OK:
        M("everything went okay internally, but it seems"
          " neocities reported an error, res.data.error.type should be"
          " printed after this");
    case NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT:
    case NEOCITIES_LLVL_ERR_CURL_EASY_INIT:
    case NEOCITIES_LLVL_ERR_CURL_MIME_INIT:
    case NEOCITIES_LLVL_ERR_CURL_MIME_ADDPART:
    case NEOCITIES_LLVL_ERR_CURL_MIME_NAME:
    case NEOCITIES_LLVL_ERR_CURL_MIME_DATA:
    case NEOCITIES_LLVL_ERR_CURL_SLIST_APPEND:
    case NEOCITIES_LLVL_ERR_CURL_EASY_SETOPT:
    case NEOCITIES_LLVL_ERR_CURL_EASY_PERFORM:
        M("libcurl failed, program should probably terminate now");
    case NEOCITIES_LLVL_ERR_CURL_MIME_FILEDATA:
        M("there is an issue with the file you want to upload");
    case NEOCITIES_LLVL_ERR_UNSUPPORTED_DELETE:
        M("delete is currently unsupported");
    case NEOCITIES_LLVL_ERR_BAD_ACTION:
        M("expected action to be a member of neocities_action");
    case NEOCITIES_LLVL_ERR_CURL_BUF_INIT:
        M("malloc failed in curl_buffer_init()");
    case NEOCITIES_LLVL_ERR_JSON_TOKENER_PARSE:
        M("json_tokener_parse_ex() failed in neocities_api()");
    case NEOCITIES_LLVL_ERR_RECEIVED_UNSUPPORTED_JSON:
        M("neocities responded with valid data which i can't understand");
    case NEOCITIES_LLVL_ERR_RECEIVED_INVALID_JSON:
        M("neocities responded with invalid json data");
    case NEOCITIES_LLVL_ERR_MALLOC_FAIL:
        M("out of memory");
    case NEOCITIES_LLVL_ERR_EXPECTED_STRING:
        M("expected the json object to be a string but got something else");
    case NEOCITIES_LLVL_ERR_EXPECTED_BOOL:
        M("expected the json object to be a boolean but got something else");
    case NEOCITIES_LLVL_ERR_EXPECTED_INT:
        M("expected the json object to be an int but got something else");
    case NEOCITIES_LLVL_ERR_EXPECTED_ARRAY:
        M("expected the json object to be an array but got something else");
    case NEOCITIES_LLVL_ERR_EXPECTED_STRING_OR_NULL:
        M("expected the json object to be a string or to be null but got"
          " something else");
    case NEOCITIES_LLVL_ERR_EXPECTED_OBJECT:
        M("expected the json object to be an object but got something else");
    case NEOCITIES_LLVL_ERR_RECEIVED_SOMETHING_ELSE:
        M("the type guessed by neocities_json_to_struct() doesn't match"
          " with the request that was made");
    default:
        return;
    }

    fputc('\n', stderr);

    return;
}

void neocities_print_error_message_api(enum neocities_api_level_error err)
{
    switch (err) {

    case SITE_NOT_FOUND:
        M("site not found");
    case UNSUPPORTED_ERROR:
        M("unknow (yet) error");
    default:
        return;
    }
    fputc('\n', stderr);

    return;
}

#undef M
