#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json.h>

#include "dtparser.h"

#include "neocities.h"

#define CURL_ERROR NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT
#define OK NEOCITIES_LLVL_OK

int main(int argc, char *argv[])
{
    neocities_res res;

    int i, err;

    char date[33] = { 0 };

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [path] \n", argv[0]);
        exit(200);              // no enum member share this number
    }

    if (curl_global_init(CURL_GLOBAL_SSL) != 0)
        return CURL_ERROR;

    if ((err =
         neocities_api_ex(APIKEY, list, (argc == 2) ? argv[1] : "",
                          &res)) != OK)
        return err;

    for (i = 0; i < res.data.list.length; i++) {
        printf("%s", res.data.list.files[i].path);
        if (res.data.list.files[i].is_directory == 1)
            putchar('/');
        putchar('\n');
    }

    neocities_destroy(&res);

    curl_global_cleanup();

    return 0;
}