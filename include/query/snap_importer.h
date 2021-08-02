#ifndef SNAP_IMPORTER_H
#define SNAP_IMPORTER_H

#include "access/heap_file.h"
#include "data-struct/htable.h"

#define C_ELEGANS_URL                                                          \
    ("https://snap.stanford.edu/data/C-elegans-frontal.txt.gz")
#define EMAIL_EU_CORE_URL                                                      \
    ("https://snap.stanford.edu/data/email-Eu-core.txt.gz")
#define DBLP_URL                                                               \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-dblp.ungraph.txt.gz")
#define AMAZON_URL                                                             \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-amazon.ungraph.txt.gz")
#define YOUTUBE_URL                                                            \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-youtube.ungraph.txt.gz")
#define WIKIPEDIA_URL ("https://snap.stanford.edu/data/wiki-topcats.txt.gz")
#define LIVE_JOURNAL_URL                                                       \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-lj.ungraph.txt.gz")
#define ORKUT_URL                                                              \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-orkut.ungraph.txt.gz")
#define FRIENDSTER_URL                                                         \
    ("https://snap.stanford.edu/data/bigdata/communities/"                     \
     "com-friendster.ungraph.txt.gz")

#define C_ELEGANS_NO_NODES     (131)
#define EMAIL_EU_CORE_NO_NODES (1005)
#define DBLP_NO_NODES          (317080)
#define AMAZON_NO_NODES        (334863)
#define YOUTUBE_NO_NODES       (1134890)
#define WIKIPEDIA_NO_NODES     (1791489)
#define LIVE_JOURNAL_NO_NODES  (3997962)
#define ORKUT_NO_NODES         (3072441)
#define FRIENDSTER_NO_NODES    (65608366)

#define C_ELEGANS_NO_RELS     (764)
#define EMAIL_EU_CORE_NO_RELS (25571)
#define DBLP_NO_RELS          (1049866)
#define AMAZON_NO_RELS        (925872)
#define YOUTUBE_NO_RELS       (2987624)
#define WIKIPEDIA_NO_RELS     (28511807)
#define LIVE_JOURNAL_NO_RELS  (34681189)
#define ORKUT_NO_RELS         (117185083)
#define FRIENDSTER_NO_RELS    (1806067135)

/* Number of columns/values per row encoding a relationship in a text file */
#define ENTRIES 2

typedef enum dataset
{
    C_ELEGANS,     /* 19 KB */
    EMAIL_EU_CORE, /* 0.1 MB */
    DBLP,          /* 3.9 MB */
    AMAZON,        /* 4.1 MB */
    YOUTUBE,       /* 10 MB */
    WIKIPEDIA,     /* 95 MB */
    LIVE_JOURNAL,  /* 119 MB */
    ORKUT,         /* 1.6 MB */
    FRIENDSTER     /* 8.7 GB */
} dataset_t;

/* Downloading and uncompressing */
const char*
get_url(dataset_t data);

unsigned long int
get_no_nodes(dataset_t data);

unsigned long int
get_no_rels(dataset_t data);

int
download_dataset(dataset_t data, const char* gz_path);

int
uncompress_dataset(const char* gz_path, const char* out_path);

/* Parsing and importing */
dict_ul_ul*
import_from_txt(heap_file* hf, const char* path, bool weighted);
#endif
