#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "neocities.h"

int main(void)
{
    json_object *obj = NULL;

    curl_global_init(CURL_GLOBAL_SSL);


//  obj = neocities_api(APIKEY, INFO, "lainzine");
    obj = neocities_api(APIKEY, INFO, "");
    printf("%s\n",
           json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
    json_object_put(obj);


    obj = neocities_api(APIKEY, UPLOAD, "neocities.h");
    printf("%s\n",
           json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
    json_object_put(obj);


    curl_global_cleanup();

    return 0;
}
