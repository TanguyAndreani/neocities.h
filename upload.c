#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json.h>

#include "dtparser.h"

#include "neocities.h"
#include "print_error_msg.h"

#include "apikey.h"

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

    if ((err = neocities_global_init()) != 0)
        return err;

    if ((err = neocities_api_ex(APIKEY, upload, argv[1], &res)) != 0
        || res.result == -1) {
        neocities_print_error_message(err);
        if (res.type == -1)     // reserved for errors
            neocities_print_error_message_api(res.data.error.type);
        return err;
    }
    //neocities_destroy(&res);

    curl_global_cleanup();

    return 0;
}
