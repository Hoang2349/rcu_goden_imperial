/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "AppHttpServer.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "appSpiffs.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_spiffs.h"

httpd_handle_t server = NULL;

/**
 * @brief Handler to redirect incoming GET request for /index.html to This can
 * be overridden by uploading file with same name
 * @param httpd_req_t *req Pointer contains information of 1 httpd request
 */
static esp_err_t indexHtmlGetHandler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "307 Temporary Redirect");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}

/**
 * @brief Handler to respond with an icon file embedded in flash. Browsers
 * expect to GET website icon at URI /favicon.ico .This can be overridden by
 * uploading file with same name be overridden by uploading file with same name
 * @param httpd_req_t *req Pointer contains information of 1 httpd request
 */
static esp_err_t faviconGetHandler(httpd_req_t *req)
{
    // extern const unsigned char favicon_ico_start[]
    // asm("_binary_favicon_ico_start"); extern const unsigned char
    // favicon_ico_end[] asm("_binary_favicon_ico_end"); const size_t
    // favicon_ico_size = (favicon_ico_end - favicon_ico_start);
    // httpd_resp_set_type(req, "image/x-icon");
    // httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
    return ESP_OK;
}

/**
 * @brief Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories
 * @param httpd_req_t *req: Pointer contains information of 1 httpd request
 * @param const char *dirPath: name path file HTML
 */
static esp_err_t httpRespDirHtml(httpd_req_t *req, const char *dirPath)
{
    char entryPath[FILE_PATH_MAX];
    char entrySize[16];
    const char *entryType;

    struct dirent *entry;
    struct stat entryStat;

    DIR *dir = opendir(dirPath);
    const size_t dirpathLen = strlen(dirPath);

    /* Retrieve the base path of file storage to construct the full path */
    strlcpy(entryPath, dirPath, sizeof(entryPath));

    if (!dir)
    {
        ESP_LOGE(__FUNCTION__, "Failed to stat dir : %s", dirPath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                            "Directory does not exist");
        return ESP_FAIL;
    }

    /* Send HTML file header */
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");

    /* Get handle to embedded file upload script */
    extern const unsigned char uploadScriptStart[] asm(
        "_binary_upload_script_html_start");
    extern const unsigned char uploadScriptEnd[] asm(
        "_binary_upload_script_html_end");
    const size_t uploadScriptSize = (uploadScriptEnd - uploadScriptStart);

    /* Add file upload form and script which on execution sends a POST request
     * to /upload */
    httpd_resp_send_chunk(req, (const char *)uploadScriptStart,
                          uploadScriptSize);

    /* Send file-list table definition and column labels */
    httpd_resp_sendstr_chunk(req,
                             "<table class=\"fixed\" border=\"1\">"
                             "<col width=\"800px\" /><col width=\"300px\" "
                             "/><col width=\"300px\" /><col width=\"100px\" />"
                             "<thead><tr><th>Name</th><th>Type</th><th>Size "
                             "(Bytes)</th><th>Delete</th></tr></thead>"
                             "<tbody>");

    /* Iterate over all files / folders and fetch their names and sizes */
    while ((entry = readdir(dir)) != NULL)
    {
        entryType = (entry->d_type == DT_DIR ? "directory" : "file");

        strlcpy(entryPath + dirpathLen, entry->d_name,
                sizeof(entryPath) - dirpathLen);
        if (stat(entryPath, &entryStat) == -1)
        {
            ESP_LOGE(__FUNCTION__, "Failed to stat %s : %s", entryType,
                     entry->d_name);
            continue;
        }
        sprintf(entrySize, "%ld", entryStat.st_size);
        ESP_LOGI(__FUNCTION__, "Found %s : %s (%s bytes)", entryType,
                 entry->d_name, entrySize);

        /* Send chunk of HTML file containing table entries with file name and
         * size */
        httpd_resp_sendstr_chunk(req, "<tr><td><a href=\"");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            httpd_resp_sendstr_chunk(req, "/");
        }
        httpd_resp_sendstr_chunk(req, "\">");
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "</a></td><td>");
        httpd_resp_sendstr_chunk(req, entryType);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, entrySize);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/delete");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(
            req, "\"><button type=\"submit\">Delete</button></form>");
        httpd_resp_sendstr_chunk(req, "</td></tr>\n");
    }
    closedir(dir);

    /* Finish the file list table */
    httpd_resp_sendstr_chunk(req, "</tbody></table>");

    /* Send remaining chunk of HTML file to complete it */
    httpd_resp_sendstr_chunk(req, "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

/**
 * @brief Set HTTP response content type according to file extension
 * @param httpd_req_t *req: Pointer contains information of 1 httpd request
 * @param const char *fileName: name file input
 */
static esp_err_t setContentTypeFromFile(httpd_req_t *req, const char *fileName)
{
    if (IS_FILE_EXT(fileName, ".pdf"))
    {
        return httpd_resp_set_type(req, "application/pdf");
    }
    else if (IS_FILE_EXT(fileName, ".html"))
    {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(fileName, ".jpeg"))
    {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(fileName, ".ico"))
    {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */

/**
 * @brief Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path)
 * @param char *dest: ponter to destination need clone
 * @param const char *basePath: name Path
 * @param const char *uri: string uri need parse
 * @param size_t destSize: size of dest
 * @return Path get from uri or NULL
 */
static const char *getPathFromUri(char *dest, const char *basePath,
                                     const char *uri, size_t destSize)
{
    const size_t basePathLen = strlen(basePath);
    size_t pathLen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathLen = MIN(pathLen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathLen = MIN(pathLen, hash - uri);
    }

    if (basePathLen + pathLen + 1 > destSize)
    {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, basePath);
    strlcpy(dest + basePathLen, uri, pathLen + 1);

    /* Return pointer to path, skipping the base */
    return dest + basePathLen;
}

/**
 * @brief Handler to download a file kept on the server
 * @param httpd_req_t *req Pointer contains information of 1 httpd request
 */
static esp_err_t downloadGetHandler(httpd_req_t *req)
{
    char filePath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat fileStat;

    const char *fileName = getPathFromUri(
        filePath, ((struct fileServerData *)req->user_ctx)->base_path,
        req->uri, sizeof(filePath));

    ESP_LOGI(__FUNCTION__, "File path: %s", filePath);

    if (!fileName)
    {
        ESP_LOGE(__FUNCTION__, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Filename too long");
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents */
    if (fileName[strlen(fileName) - 1] == '/')
    {
        return httpRespDirHtml(req, filePath);
    }

    if (stat(filePath, &fileStat) == -1)
    {
        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths */
        if (strcmp(fileName, "/index.html") == 0)
        {
            return indexHtmlGetHandler(req);
        }
        else if (strcmp(fileName, "/favicon.ico") == 0)
        {
            return faviconGetHandler(req);
        }
        ESP_LOGE(__FUNCTION__, "Failed to stat file : %s", filePath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filePath, "r");
    if (!fd)
    {
        ESP_LOGE(__FUNCTION__, "Failed to read existing file : %s", filePath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(__FUNCTION__, "Sending file : %s (%ld bytes)...", fileName,
             fileStat.st_size);
    setContentTypeFromFile(req, fileName);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct fileServerData *)req->user_ctx)->scratch;
    size_t chunkSize;
    do
    {
        /* Read file in chunks into the scratch buffer */
        chunkSize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunkSize > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunkSize) != ESP_OK)
            {
                fclose(fd);
                ESP_LOGE(__FUNCTION__, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                    "Failed to send file");
                return ESP_FAIL;
            }
        }

        /* Keep looping till the whole file is sent */
    } while (chunkSize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(__FUNCTION__, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/**
 * @brief Handler to upload a file onto the server
 * @param httpd_req_t *req: Pointer contains information of 1 httpd request
 */
static esp_err_t uploadPostHandler(httpd_req_t *req)
{
    char filePath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat fileStat;

    /* Skip leading "/upload" from URI to get fileName */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *fileName = getPathFromUri(
        filePath, ((struct fileServerData *)req->user_ctx)->base_path,
        req->uri + sizeof("/upload") - 1, sizeof(filePath));
    if (!fileName)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (fileName[strlen(fileName) - 1] == '/')
    {
        ESP_LOGE(__FUNCTION__, "Invalid fileName : %s", fileName);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Invalid fileName");
        return ESP_FAIL;
    }

    if (stat(filePath, &fileStat) == 0)
    {
        ESP_LOGE(__FUNCTION__, "File already exists : %s", filePath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE)
    {
        ESP_LOGE(__FUNCTION__, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than " MAX_FILE_SIZE_STR
                            "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = fopen(filePath, "w");
    if (!fd)
    {
        ESP_LOGE(__FUNCTION__, "Failed to create file : %s", filePath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(__FUNCTION__, "Receiving file : %s...", fileName);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((struct fileServerData *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0)
    {
        ESP_LOGI(__FUNCTION__, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf,
                                       MIN(remaining, SCRATCH_BUFSIZE))) <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filePath);

            ESP_LOGE(__FUNCTION__, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to receive file");
            return ESP_FAIL;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd)))
        {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filePath);

            ESP_LOGE(__FUNCTION__, "File write failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to write file to storage");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(__FUNCTION__, "File reception complete");

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

/**
 * @brief Handler to delete a file from the server
 * @param httpd_req_t *req Pointer contains information of 1 httpd request
 */
static esp_err_t deletePostHandler(httpd_req_t *req)
{
    char filePath[FILE_PATH_MAX];
    struct stat fileStat;

    /* Skip leading "/delete" from URI to get fileName */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *fileName = getPathFromUri(
        filePath, ((struct fileServerData *)req->user_ctx)->base_path,
        req->uri + sizeof("/delete") - 1, sizeof(filePath));
    if (!fileName)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (fileName[strlen(fileName) - 1] == '/')
    {
        ESP_LOGE(__FUNCTION__, "Invalid fileName : %s", fileName);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Invalid fileName");
        return ESP_FAIL;
    }

    if (stat(filePath, &fileStat) == -1)
    {
        ESP_LOGE(__FUNCTION__, "File does not exist : %s", fileName);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_FAIL;
    }

    ESP_LOGI(__FUNCTION__, "Deleting file : %s", fileName);
    /* Delete file */
    unlink(filePath);

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

/**
 * @brief Function to start the file server
 * @param const char *basePath: input string Path file server
 */
esp_err_t startFileServer(const char *basePath)
{
    struct fileServerData *serverData = NULL;

    if (serverData)
    {
        ESP_LOGE(__FUNCTION__, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    serverData = calloc(1, sizeof(struct fileServerData));
    if (!serverData)
    {
        ESP_LOGE(__FUNCTION__, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(serverData->base_path, basePath, sizeof(serverData->base_path));

    // httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(__FUNCTION__, "Starting HTTP Server on port: '%d'",
             config.server_port);
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Failed to start file server!");
        return ESP_FAIL;
    }

    /* URI handler for getting uploaded files */
    httpd_uri_t fileDownLoad = {
        .uri = "/*",  // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = downloadGetHandler,
        .user_ctx = serverData  // Pass server data as context
    };
    httpd_register_uri_handler(server, &fileDownLoad);

    /* URI handler for uploading files to server */
    httpd_uri_t fileUpload = {
        .uri = "/upload/*",  // Match all URIs of type /upload/path/to/file
        .method = HTTP_POST,
        .handler = uploadPostHandler,
        .user_ctx = serverData  // Pass server data as context
    };
    httpd_register_uri_handler(server, &fileUpload);

    /* URI handler for deleting files from server */
    httpd_uri_t fileDelete = {
        .uri = "/delete/*",  // Match all URIs of type /delete/path/to/file
        .method = HTTP_POST,
        .handler = deletePostHandler,
        .user_ctx = serverData  // Pass server data as context
    };
    httpd_register_uri_handler(server, &fileDelete);
    return ESP_OK;
}

esp_err_t stopFileServer()
{
    if (server)
    {
        if (httpd_stop(server) != ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Failed to Stop server!");
            return ESP_FAIL;
        }
        ESP_LOGI(__FUNCTION__, "Stop httpd server");
        return ESP_OK;
    }
    return ESP_OK;
}
