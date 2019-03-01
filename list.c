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

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [path] \n", argv[0]);
        exit(200);              // no enum member share this number
    }

    if ((err = neocities_global_init()) != 0)
        return err;

    if ((err =
         neocities_api_ex(APIKEY, list, (argc == 2) ? argv[1] : "",
                          &res)) != 0 || res.result == -1) {
        neocities_print_error_message(err);
        if (res.type == -1)     // reserved for errors
            neocities_print_error_message(res.data.error.type);
        else
            neocities_destroy(&res);
        return err;
    }

#define has_blanks(s) \
    strchr(s, ' ') != NULL

    for (i = 0; i < res.data.list.length; i++) {
        if (has_blanks(res.data.list.files[i].path))
            printf("'%s'", res.data.list.files[i].path);
        else
            printf("%s", res.data.list.files[i].path);
        if (res.data.list.files[i].is_directory == 1)
            putchar('/');
        putchar('\n');
    }

#undef has_blanks

    neocities_destroy(&res);

    curl_global_cleanup();

    return 0;
}
