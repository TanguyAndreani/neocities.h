#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json.h>

#include "dtparser.h"

#include "neocities.h"
#include "print_error_msg.h"

#define CURL_ERROR NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT
#define OK NEOCITIES_LLVL_OK

int main(int argc, char *argv[])
{
    neocities_res res;

    int i, err;

    char date[33] = { 0 };

    if (argc != 2) {
        /* ./dir/file is valid
           ./some-utf8-character/file is valid
           ./dir/some-utf8-character is invalid

           by libcurl design, it will default to
           sendfile

           after the last /, filename should be [a-zA-Z\.]+
         */

        fprintf(stderr, "Usage: %s filename\n\n", argv[0]);
        exit(200);              // no enum member share this number
    }

    if (curl_global_init(CURL_GLOBAL_SSL) != 0)
        return CURL_ERROR;

    if ((err = neocities_api_ex(APIKEY, upload, argv[1], &res)) != OK
        || res.result != 0) {
        neocities_print_error_message(err);
        if (res.type == NEOCITIES_ERROR_STRUCT)
            neocities_print_error_message_api(res.data.error.type);
        return err;
    }

    //neocities_destroy(&res);

    curl_global_cleanup();

    return 0;
}
