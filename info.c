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

    char date_ca[33] = { 0 };
    char date_lu[33] = { 0 };

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [sitename]\n", argv[0]);
        exit(200);              // no enum member share this number
    }

    if (curl_global_init(CURL_GLOBAL_SSL) != 0)
        return CURL_ERROR;

    if ((err =
         neocities_api_ex(APIKEY, info, ((argc == 1) ? "" : argv[1]),
                          &res)) != OK || res.result != 0) {
        neocities_print_error_message(err);
        if (res.type == NEOCITIES_ERROR_STRUCT)
            fprintf(stderr, "error_type: %d\n", res.data.error.type);
        else
            neocities_destroy(&res);
        return err;
    }

    if (res.data.info.sitename != NULL)
        printf("sitename     %s\n", res.data.info.sitename);

    if (res.data.info.views != -1)
        printf("views        %d\n", res.data.info.views);

    if (res.data.info.hits != -1)
        printf("hits         %d\n", res.data.info.hits);

    if (res.data.info.created_at != 0) {
        rfc5322_date_create(res.data.info.created_at, date_ca, 32);
        printf("created_at   %s\n", date_ca);
    }

    if (res.data.info.last_updated != 0) {
        rfc5322_date_create(res.data.info.last_updated, date_lu, 32);
        printf("last_updated %s\n", date_lu);
    }

    if (res.data.info.domain != NULL) {
        printf("domain       %s\n", res.data.info.domain);
    }

    if (res.data.info.tags[0] != NULL) {
        printf("tags         [");
        i = 0;
        while (res.data.info.tags[i + 1] != NULL) {
            printf("\"%s\", ", res.data.info.tags[i]);
            i++;
        }
        printf("\"%s\"]\n", res.data.info.tags[i]);
    }

    neocities_destroy(&res);

    curl_global_cleanup();

    return 0;
}
