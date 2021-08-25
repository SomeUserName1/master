#include "query/snap_importer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <zconf.h>
#include <zlib.h>

#include "access/in_memory_graph.h"
#include "constants.h"
#include "data-struct/htable.h"

#define SET_BINARY_MODE(file)
/* 512 KB Buffer/Chunk size */
#define CHUNK        (1 << 19)
#define STATUS_LINES (100000)
#define TIMEOUT      (999)

static size_t
write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

int
download_dataset(dataset_t data, const char* gz_path)
{
    CURL*    curl;
    CURLcode result;
    FILE*    gz_file;

    curl = curl_easy_init();
    if (!curl) {
        printf("%s", "Failed to initialize curl");
        return -1;
    }

    gz_file = fopen(gz_path, "wb");
    if (!gz_file) {
        printf("%s", "Failed to open file to download dataset to");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, get_url(data));
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)gz_file);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        printf("%s %s\n",
               "Failed to download the dataset with curl error: ",
               curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        return -1;
    }

    fclose(gz_file);
    curl_easy_cleanup(curl);

    return 0;
}

int
uncompress_dataset(const char* gz_path, const char* out_path)
{
    int           ret;
    unsigned      have;
    z_stream      stream;
    FILE*         out_file;
    FILE*         in_gz_file;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    stream.zalloc   = Z_NULL;
    stream.zfree    = Z_NULL;
    stream.opaque   = Z_NULL;
    stream.avail_in = 0;
    stream.next_in  = Z_NULL;
    ret             = inflateInit2(&stream, (32 + MAX_WBITS));
    if (ret != Z_OK) {
        return -1;
    }

    out_file = fopen(out_path, "wb");
    if (out_file == NULL) {
        printf("%s", "couldn't open the output file");
        inflateEnd(&stream);
        return -1;
    }

    in_gz_file = fopen(gz_path, "rb");
    if (in_gz_file == NULL) {
        printf("%s", "couldn't open the input file as gzipped");
        fclose(out_file);
        inflateEnd(&stream);
        return -1;
    }

    do {
        stream.avail_in = fread(in, 1, CHUNK, in_gz_file);
        if (ferror(in_gz_file)) {
            inflateEnd(&stream);
            fclose(out_file);
            fclose(in_gz_file);
            return -1;
        }

        stream.next_in = in;
        do {
            stream.avail_out = CHUNK;
            stream.next_out  = out;

            ret = inflate(&stream, Z_SYNC_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    printf("Error: %s\n", stream.msg);
                    inflateEnd(&stream);
                    fclose(out_file);
                    fclose(in_gz_file);
                    return -1;
            }

            have = CHUNK - stream.avail_out;
            if (fwrite(out, 1, have, out_file) != have || ferror(out_file)) {
                inflateEnd(&stream);
                fclose(out_file);
                fclose(in_gz_file);
                return -1;
            }
        } while (stream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    fclose(out_file);
    fclose(in_gz_file);
    inflateEnd(&stream);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

const char*
get_url(dataset_t data)
{
    const char* result;
    switch (data) {
        case C_ELEGANS:
            result = C_ELEGANS_URL;
            break;

        case EMAIL_EU_CORE:
            result = EMAIL_EU_CORE_URL;
            break;

        case DBLP:
            result = DBLP_URL;
            break;

        case AMAZON:
            result = AMAZON_URL;
            break;

        case YOUTUBE:
            result = YOUTUBE_URL;
            break;

        case WIKIPEDIA:
            result = WIKIPEDIA_URL;
            break;

        case LIVE_JOURNAL:
            result = LIVE_JOURNAL_URL;
            break;

        case ORKUT:
            result = ORKUT_URL;
            break;

        case FRIENDSTER:
            result = FRIENDSTER_URL;
            break;
    }

    return result;
}

unsigned long int
get_no_nodes(dataset_t data)
{
    unsigned long int result;
    switch (data) {
        case C_ELEGANS:
            result = C_ELEGANS_NO_NODES;
            break;

        case EMAIL_EU_CORE:
            result = EMAIL_EU_CORE_NO_NODES;
            break;

        case DBLP:
            result = DBLP_NO_NODES;
            break;

        case AMAZON:
            result = AMAZON_NO_NODES;
            break;

        case YOUTUBE:
            result = YOUTUBE_NO_NODES;
            break;

        case WIKIPEDIA:
            result = WIKIPEDIA_NO_NODES;
            break;

        case LIVE_JOURNAL:
            result = LIVE_JOURNAL_NO_NODES;
            break;

        case ORKUT:
            result = ORKUT_NO_NODES;
            break;

        case FRIENDSTER:
            result = FRIENDSTER_NO_NODES;
            break;
    }

    return result;
}

unsigned long int
get_no_rels(dataset_t data)
{
    unsigned long int result;
    switch (data) {
        case C_ELEGANS:
            result = C_ELEGANS_NO_RELS;
            break;

        case EMAIL_EU_CORE:
            result = EMAIL_EU_CORE_NO_RELS;
            break;

        case DBLP:
            result = DBLP_NO_RELS;
            break;

        case AMAZON:
            result = AMAZON_NO_RELS;
            break;

        case YOUTUBE:
            result = YOUTUBE_NO_RELS;
            break;

        case WIKIPEDIA:
            result = WIKIPEDIA_NO_RELS;
            break;

        case LIVE_JOURNAL:
            result = LIVE_JOURNAL_NO_RELS;
            break;

        case ORKUT:
            result = ORKUT_NO_RELS;
            break;

        case FRIENDSTER:
            result = FRIENDSTER_NO_RELS;
            break;
    }
    return result;
}

dict_ul_ul*
import_from_txt(heap_file* hf, const char* path, bool weighted)
{
    if (!hf || !path) {
        printf("snap importer - import from txt: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    int import_fields = weighted ? 3 : 2;

    unsigned long from_to[2];
    double        weight;
    char          buf[CHUNK];
    size_t        lines        = 1;
    dict_ul_ul*   txt_to_db_id = d_ul_ul_create();
    unsigned long db_id        = 0;
    char          label[MAX_STR_LEN];
    int           label_len;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("snap importer - import from txt: Failed to open file to read "
               "from");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, sizeof(buf), in_file)) {
        if (lines % STATUS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        if (*buf == '#') {
            continue;
        }

        if (weighted) {
            if (sscanf(buf, "%lu %lu %lf\n", &from_to[0], &from_to[1], &weight)
                != 3) {
                printf("%s\n",
                       "snap importer - import from txt: Failed to read "
                       "input\n");
                exit(EXIT_FAILURE);
            }
        } else {
            if (sscanf(buf, "%lu %lu\n", &from_to[0], &from_to[1]) != 2) {
                printf("%s\n",
                       "snap importer - import from txt: Failed to read "
                       "input\n");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < import_fields; ++i) {
            if (dict_ul_ul_contains(txt_to_db_id, from_to[i])) {
                from_to[i] = dict_ul_ul_get_direct(txt_to_db_id, from_to[i]);
            } else {
                label_len = snprintf(NULL, 0, "%lu", from_to[i]);
                if (label_len
                    != snprintf(label, label_len + 1, "%lu", from_to[i])) {
                    printf("snap importer - import from txt: failed to write "
                           "id as label!\n");
                    exit(EXIT_FAILURE);
                }

                db_id = create_node(hf, label);
                dict_ul_ul_insert(txt_to_db_id, from_to[i], db_id);
                from_to[i] = db_id;
            }
        }

        if (weighted) {
            create_relationship(hf, from_to[0], from_to[1], weight, "\0");
        } else {
            create_relationship(hf, from_to[0], from_to[1], 1, "\0");
        }
        lines++;
    }

    fclose(in_file);

    return txt_to_db_id;
}

dict_ul_ul*
in_memory_import_from_txt(in_memory_graph* g, const char* path, bool weighted)
{
    if (!g || !path) {
        printf("snap importer - import from txt: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    int import_fields = weighted ? 3 : 2;

    unsigned long from_to[2];
    double        weight;
    char          buf[CHUNK];
    size_t        lines        = 1;
    dict_ul_ul*   txt_to_db_id = d_ul_ul_create();
    unsigned long db_id        = 0;
    char          label[MAX_STR_LEN];
    int           label_len;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("snap importer - import from txt: Failed to open file to read "
               "from");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, sizeof(buf), in_file)) {
        if (lines % STATUS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        if (*buf == '#') {
            continue;
        }

        if (weighted) {
            if (sscanf(buf, "%lu %lu %lf\n", &from_to[0], &from_to[1], &weight)
                != 3) {
                printf("%s\n",
                       "snap importer - import from txt: Failed to read "
                       "input\n");
                exit(EXIT_FAILURE);
            }
        } else {
            if (sscanf(buf, "%lu %lu\n", &from_to[0], &from_to[1]) != 2) {
                printf("%s\n",
                       "snap importer - import from txt: Failed to read "
                       "input\n");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < import_fields; ++i) {
            if (dict_ul_ul_contains(txt_to_db_id, from_to[i])) {
                from_to[i] = dict_ul_ul_get_direct(txt_to_db_id, from_to[i]);
            } else {
                label_len = snprintf(NULL, 0, "%lu", from_to[i]);
                if (label_len
                    != snprintf(label, label_len + 1, "%lu", from_to[i])) {
                    printf("snap importer - import from txt: failed to write "
                           "id as label!\n");
                    exit(EXIT_FAILURE);
                }

                db_id = in_memory_create_node(g, label);
                dict_ul_ul_insert(txt_to_db_id, from_to[i], db_id);
                from_to[i] = db_id;
            }
        }

        if (weighted) {
            in_memory_create_relationship_weighted(
                  g, from_to[0], from_to[1], weight, "\0");
        } else {
            in_memory_create_relationship(g, from_to[0], from_to[1], "\0");
        }

        lines++;
    }

    fclose(in_file);

    return txt_to_db_id;
}
