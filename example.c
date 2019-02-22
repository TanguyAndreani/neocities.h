#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json.h>

#include "dtparser.h"

#include "neocities.h"

#define CURL_ERROR NEOCITIES_LLVL_ERR_CURL_GLOBAL_INIT
#define OK NEOCITIES_LLVL_OK

int main(void)
{
    neocities_res res;

    int i, err;

    char date[33] = { 0 };

    if (curl_global_init(CURL_GLOBAL_SSL) != 0)
        return CURL_ERROR;

    if ((err = neocities_api_ex(APIKEY, LIST, "", &res)) != OK)
        return err;

    puts("## Listing 5 files ##");

    printf("success: %d\n", res.result);

    for (i = 0; i < 5; i++) {
        rfc5322_date_create(res.data.list.files[i].updated_at, date, 32);
        printf("path: %s\n  last_update: %s\n  is_dir: %d\n  size: %d\n",
               res.data.list.files[i].path, date,
               res.data.list.files[i].is_directory,
               res.data.list.files[i].size);
    }

    neocities_destroy_list(&res);

    if ((err = neocities_api_ex(APIKEY, INFO, "", &res)) != OK)
        return err;

    printf("## Site infos (%s) ##\n", res.data.info.sitename);

    printf("success: %d, views: %d, hits: %d, domain: %s\n", res.result,
           res.data.info.views, res.data.info.hits, res.data.info.domain);

    puts("tags:");

    i = 0;
    while (res.data.info.tags[i] != NULL) {
        printf("- %s\n", res.data.info.tags[i]);
        i++;
    }

    rfc5322_date_create(res.data.info.created_at, date, 32);
    printf("created_at: %s\n", date);

    rfc5322_date_create(res.data.info.last_updated, date, 32);
    printf("last_updated: %s\n", date);

    neocities_destroy_info(&res);

    puts("## Failing deliberately ##");

    if ((err = neocities_api_ex(APIKEY, INFO, "donotexist", &res)) != OK) {
        printf("result: %d, error_type: %d\n", res.result,
               res.data.error.type);
        neocities_destroy_error(&res);
    } else {
        neocities_destroy_info(&res);
    }

    puts("## Uploading a file ##");

    if ((err = neocities_api_ex(APIKEY, UPLOAD, "deps.txt", &res)) != OK) {
        printf("result: %d, error_type: %d\n", res.result,
               res.data.error.type);

        // not mandatory if res only used once
        neocities_destroy_error(&res);
    } else {
        printf("result: %d\n", res.result);
    }

    curl_global_cleanup();

    return 0;
}
