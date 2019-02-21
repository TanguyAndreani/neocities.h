#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json.h>

#include "neocities.h"

int main(void)
{
    json_object *jobj;
    int err;

    if (curl_global_init(CURL_GLOBAL_SSL) != 0)
        return NEOCITIES_ERR_CURL_GLOBAL_INIT;

    if ((err =
         neocities_api(APIKEY, INFO, "lainzine", &jobj)) != NEOCITIES_OK)
        return err;

    printf("%s\n",
           json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY));
    // fflush(stdout);

    curl_global_cleanup();

    return 0;
}
