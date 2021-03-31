#include "../../src/import/snap_importer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/data-struct/dict_ul.h"

#define DATASET_TEMP ("/home/someusername/workspace_local/dataset.txt.gz")
#define PATH_CELEGANS ("/home/someusername/workspace_local/celegans.txt")
#define PATH_EMAIL ("/home/someusername/workspace_local/email_eu.txt")
#define PATH_DBLP ("/home/someusername/workspace_local/dblp.txt")
#define PATH_AMAZON ("/home/someusername/workspace_local/amazon.txt")
#define PATH_YOUTUBE ("/home/someusername/workspace_local/youtube.txt")
#define PATH_WIKIPEDIA ("/home/someusername/workspace_local/wikipeida.txt")
#define PATH_LIVE_JOURNAL                                                      \
    ("/home/someusername/workspace_local/live_journal.txt")
#define PATH_ORKUT ("/home/someusername/workspace_local/orkut.txt")
#define PATH_FRIENDSTER ("/home/someusername/workspace_local/friendster.txt")

void
test_celegans(void)
{
    dataset_t dataset = C_ELEGANS;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_CELEGANS) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_CELEGANS);

    assert(dict_ul_node_size(db->cache_nodes) == C_ELEGANS_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == C_ELEGANS_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_email(void)
{
    dataset_t dataset = EMAIL_EU_CORE;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_CELEGANS) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_CELEGANS);

    assert(dict_ul_node_size(db->cache_nodes) == EMAIL_EU_CORE_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == EMAIL_EU_CORE_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_dblp(void)
{
    dataset_t dataset = DBLP;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_DBLP) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_DBLP);

    assert(dict_ul_node_size(db->cache_nodes) == DBLP_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == DBLP_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_amazon(void)
{
    dataset_t dataset = AMAZON;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_AMAZON) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_AMAZON);

    assert(dict_ul_node_size(db->cache_nodes) == AMAZON_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == AMAZON_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_youtube(void)
{
    dataset_t dataset = YOUTUBE;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_YOUTUBE) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_YOUTUBE);

    assert(dict_ul_node_size(db->cache_nodes) == YOUTUBE_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == YOUTUBE_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_wikipedia(void)
{
    dataset_t dataset = WIKIPEDIA;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_WIKIPEDIA) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_WIKIPEDIA);

    assert(dict_ul_node_size(db->cache_nodes) == WIKIPEDIA_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == WIKIPEDIA_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_live_journal(void)
{
    dataset_t dataset = LIVE_JOURNAL;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_LIVE_JOURNAL) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_LIVE_JOURNAL);

    assert(dict_ul_node_size(db->cache_nodes) == LIVE_JOURNAL_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == LIVE_JOURNAL_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_orkut(void)
{
    dataset_t dataset = ORKUT;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_ORKUT) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_ORKUT);

    assert(dict_ul_node_size(db->cache_nodes) == ORKUT_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == ORKUT_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

void
test_friendster(void)
{
    dataset_t dataset = FRIENDSTER;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_FRIENDSTER) == 0);
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(db, PATH_FRIENDSTER);

    assert(dict_ul_node_size(db->cache_nodes) == FRIENDSTER_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == FRIENDSTER_NO_RELS);

    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}

int
main(void)
{
    test_celegans();
    test_email();
    test_dblp();
    test_amazon();
    // test_youtube();
    // test_wikipedia();
    // test_live_journal();
    // test_orkut();
    // test_friendster();
}
