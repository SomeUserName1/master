#include "snap_importer.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#ifdef WIN
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include "zlib.h"
#include <curl/curl.h>

#include "../access/in_memory_file.h"

#define SET_BINARY_MODE(file)
/* 512 KB Buffer/Chunk size */
#define CHUNK 524288
/* max Line Size 255 chars */
#define MAX_LINE_LENGTH 255


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

int download_dataset(dataset_t data, const char* gz_path) {
    CURL* curl;
    CURLcode result;
    FILE* gz_file;

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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) gz_file);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        printf("%s %s\n", "Failed to download the dataset with curl error: ",
                curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        return -1;
    }

    fclose(gz_file);
    curl_easy_cleanup(curl);

    return 0;
}

int uncompress_dataset(const char* gz_path, const char* out_path) {
    int ret;
    unsigned have;
    z_stream stream;
    FILE* out_file;
    FILE* in_gz_file;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    int err_no = 0;
    const char* emsg;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    ret = inflateInit2(&stream, (32 + MAX_WBITS));
    if (ret != Z_OK) {
        zlib_error(ret);
        return -1;
    }

    out_file = fopen(out_path, "wb");
    in_gz_file = fopen(gz_path, "rb");

    if (out_file == NULL) {
        printf("%s", "couldn't open the output file");
        inflateEnd(&stream);
        return -1;
    }
    if (in_gz_file == NULL) {
        printf("%s", "couldn't open the input file as gzipped");
        fclose(out_file);
        inflateEnd(&stream);
        return -1;
    }

    do {
        stream.avail_in  = fread(in, 1, CHUNK, in_gz_file);
        if (ferror(in_gz_file)) {
            inflateEnd(&stream);
            fclose(out_file);
            fclose(in_gz_file);
            return -1;
        }

        stream.next_in = in;
        do {
            stream.avail_out = CHUNK;
            stream.next_out = out;

            ret = inflate(&stream, Z_SYNC_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    printf("Error: %s\n", stream.msg);
                    inflateEnd(&stream);
                    zlib_error(ret);
                    fclose(out_file);
                    fclose(in_gz_file);
                    return -1;
            }

            have = CHUNK - stream.avail_out;
            if (fwrite(out, 1, have, out_file) != have || ferror(out_file)) {
                inflateEnd(&stream);
                zlib_error(ret);
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

void zlib_error(int ret) {
    fputs("Uncompressing failed: ", stderr);
    switch (ret) {
    case Z_ERRNO:
            fputs("error reading the input or writing the outputfile\n",
                    stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

const char* get_url(dataset_t data) {
    const char* result;
    switch (data) {
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
    }
    return result;
}

unsigned long int get_no_nodes(dataset_t data) {
    unsigned long int result;
    switch (data) {
        case EMAIL_EU_CORE:
            result = EMAIL_EU_CORE_NO_NODES;

        case DBLP:
            result = DBLP_NO_NODES;

        case AMAZON:
            result = AMAZON_NO_NODES;

        case YOUTUBE:
            result = YOUTUBE_NO_NODES;

        case WIKIPEDIA:
            result = WIKIPEDIA_NO_NODES;

        case LIVE_JOURNAL:
            result = LIVE_JOURNAL_NO_NODES;

        case ORKUT:
            result = ORKUT_NO_NODES;

        case FRIENDSTER:
            result = FRIENDSTER_NO_NODES;
    }
    return result;

}

unsigned long int get_no_rels(dataset_t data) {
    unsigned long int result;
    switch (data) {
        case EMAIL_EU_CORE:
            result = EMAIL_EU_CORE_NO_RELS;

        case DBLP:
            result = DBLP_NO_RELS;

        case AMAZON:
            result = AMAZON_NO_RELS;

        case YOUTUBE:
            result = YOUTUBE_NO_RELS;

        case WIKIPEDIA:
            result = WIKIPEDIA_NO_RELS;

        case LIVE_JOURNAL:
            result = LIVE_JOURNAL_NO_RELS;

        case ORKUT:
            result = ORKUT_NO_RELS;

        case FRIENDSTER:
            result = FRIENDSTER_NO_RELS;
    }
    return result;

}

int import_from_txt(const char* path) {
    char in[MAX_LINE_LENGTH];
    unsigned long int fromTo[2];
    char* end;
    int j = 0;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from");
        return -1;
    }

    while(fgets(in, MAX_LINE_LENGTH - 1, in_file)) {
        for (unsigned long i = strtoul(in, &end, 10); in != end; i = strtoul(in, &end, 10)) {
 //           create_node(i);
            fromTo[j] = i;
            ++j;
        }
//        create_relationship(fromTo[0], fromTo[1]);
        j = 0;
    }

    fclose(in_file);

    return 0;
}
