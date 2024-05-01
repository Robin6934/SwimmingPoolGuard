/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"

static const char *REST_TAG = "esp-rest";
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

/* Simple handler for getting temperature data */
static esp_err_t temperature_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "raw", esp_random() % 20);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t swimming_pool_get_receiped(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");

    // Create the root of the JSON object
    cJSON *root = cJSON_CreateObject();

    // Adding the recipe name
    cJSON_AddStringToObject(root, "name", "Swimming Pool");

    // Adding the ingredients as an array
    cJSON *ingredients = cJSON_AddArrayToObject(root, "ingredients");
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("30ml Vodka"));
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("30ml Malibu"));
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("20ml Cream of Coconut"));
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("120ml Pineapple Juice"));
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("20ml Blue Curaçao"));
    cJSON_AddItemToArray(ingredients, cJSON_CreateString("Ice cubes"));

    // Adding the preparation steps
    cJSON_AddStringToObject(root, "preparation", "Fill a shaker with ice cubes. Add vodka, cream of coconut, pineapple juice, blue curaçao, and sweet and sour mix. Shake well. Strain into a chilled highball glass filled with ice cubes. Garnish with a slice of pineapple and a cherry.");

    // Convert the entire JSON object into a string
    char *response_string = cJSON_Print(root);

    // Send the JSON response
    httpd_resp_send(req, response_string, HTTPD_RESP_USE_STRLEN);

    // Free the JSON object
    cJSON_Delete(root);
    free(response_string);

    return ESP_OK;
}

esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &system_info_get_uri);

    /* URI handler for fetching temperature data */
    httpd_uri_t temperature_data_get_uri = {
        .uri = "/api/v1/temp/raw",
        .method = HTTP_GET,
        .handler = temperature_data_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &temperature_data_get_uri);

        /* URI handler for fetching temperature data */
    httpd_uri_t swimmingpool_receipe_get_uri = {
        .uri = "/api/v1/cocktail/swimmingpool",
        .method = HTTP_GET,
        .handler = swimming_pool_get_receiped,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &swimmingpool_receipe_get_uri);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}
