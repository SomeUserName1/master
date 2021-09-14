/*
 * @(#)snap_importer_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "page_cache.h"
#include "physical_database.h"
#include "query/snap_importer.h"

#include <assert.h>
#include <gperftools/profiler.h>
#include <string.h>

#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/htable.h"

#define DATASET_TEMP   ("/home/someusername/workspace_local/dataset.txt.gz")
#define PATH_CELEGANS  ("/home/someusername/workspace_local/celegans.txt")
#define PATH_EMAIL     ("/home/someusername/workspace_local/email_eu.txt")
#define PATH_DBLP      ("/home/someusername/workspace_local/hflp.txt")
#define PATH_AMAZON    ("/home/someusername/workspace_local/amazon.txt")
#define PATH_YOUTUBE   ("/home/someusername/workspace_local/youtube.txt")
#define PATH_WIKIPEDIA ("/home/someusername/workspace_local/wikipeida.txt")
#define PATH_LIVE_JOURNAL                                                      \
    ("/home/someusername/workspace_local/live_journal.txt")
#define PATH_ORKUT      ("/home/someusername/workspace_local/orkut.txt")
#define PATH_FRIENDSTER ("/home/someusername/workspace_local/friendster.txt")

heap_file*
prepare(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);
    allocate_pages(pdb, relationship_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );
    return hf;
}

void
clean_up(heap_file* hf)
{
    page_cache*   pc  = hf->cache;
    phy_database* pdb = pc->pdb;

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_celegans(void)
{
    dataset_t dataset = C_ELEGANS;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_CELEGANS) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_CELEGANS, false);

    assert(hf->n_nodes == C_ELEGANS_NO_NODES);
    assert(hf->n_rels == C_ELEGANS_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_email(void)
{
    dataset_t dataset = EMAIL_EU_CORE;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_EMAIL) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_EMAIL, false);

    assert(hf->n_nodes == EMAIL_EU_CORE_NO_NODES);
    assert(hf->n_rels == EMAIL_EU_CORE_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_dblp(void)
{
    dataset_t dataset = DBLP;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_DBLP) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_DBLP, false);

    assert(hf->n_nodes == DBLP_NO_NODES);
    assert(hf->n_rels == DBLP_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_amazon(void)
{
    dataset_t dataset = AMAZON;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_AMAZON) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_AMAZON, false);

    assert(hf->n_nodes == AMAZON_NO_NODES);
    assert(hf->n_rels == AMAZON_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_youtube(void)
{
    dataset_t dataset = YOUTUBE;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_YOUTUBE) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_YOUTUBE, false);

    assert(hf->n_nodes == YOUTUBE_NO_NODES);
    assert(hf->n_rels == YOUTUBE_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_wikipedia(void)
{
    //   dataset_t dataset = WIKIPEDIA;

    //   assert(download_dataset(dataset, DATASET_TEMP) == 0);
    //   assert(uncompress_dataset(DATASET_TEMP, PATH_WIKIPEDIA) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_WIKIPEDIA, false);

    assert(hf->n_nodes == WIKIPEDIA_NO_NODES);
    assert(hf->n_rels == WIKIPEDIA_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_live_journal(void)
{
    dataset_t dataset = LIVE_JOURNAL;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_LIVE_JOURNAL) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_LIVE_JOURNAL, false);

    assert(hf->n_nodes == LIVE_JOURNAL_NO_NODES);
    assert(hf->n_rels == LIVE_JOURNAL_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_orkut(void)
{
    dataset_t dataset = ORKUT;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_ORKUT) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_ORKUT, false);

    assert(hf->n_nodes == ORKUT_NO_NODES);
    assert(hf->n_rels == ORKUT_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_friendster(void)
{
    dataset_t dataset = FRIENDSTER;

    assert(download_dataset(dataset, DATASET_TEMP) == 0);
    assert(uncompress_dataset(DATASET_TEMP, PATH_FRIENDSTER) == 0);
    heap_file*  hf  = prepare();
    dict_ul_ul* map = import_from_txt(hf, PATH_FRIENDSTER, false);

    assert(hf->n_nodes == FRIENDSTER_NO_NODES);
    assert(hf->n_rels == FRIENDSTER_NO_RELS);

    dict_ul_ul_destroy(map);
    clean_up(hf);
}

void
test_get_url(void)
{
    assert(strcmp(get_url(C_ELEGANS), C_ELEGANS_URL) == 0);
    assert(strcmp(get_url(EMAIL_EU_CORE), EMAIL_EU_CORE_URL) == 0);
    assert(strcmp(get_url(DBLP), DBLP_URL) == 0);
    assert(strcmp(get_url(AMAZON), AMAZON_URL) == 0);
    assert(strcmp(get_url(YOUTUBE), YOUTUBE_URL) == 0);
    assert(strcmp(get_url(WIKIPEDIA), WIKIPEDIA_URL) == 0);
    assert(strcmp(get_url(LIVE_JOURNAL), LIVE_JOURNAL_URL) == 0);
    assert(strcmp(get_url(ORKUT), ORKUT_URL) == 0);
    assert(strcmp(get_url(FRIENDSTER), FRIENDSTER_URL) == 0);
}

void
test_get_no_nodes(void)
{
    assert(get_no_nodes(C_ELEGANS) == C_ELEGANS_NO_NODES);
    assert(get_no_nodes(EMAIL_EU_CORE) == EMAIL_EU_CORE_NO_NODES);
    assert(get_no_nodes(DBLP) == DBLP_NO_NODES);
    assert(get_no_nodes(AMAZON) == AMAZON_NO_NODES);
    assert(get_no_nodes(YOUTUBE) == YOUTUBE_NO_NODES);
    assert(get_no_nodes(WIKIPEDIA) == WIKIPEDIA_NO_NODES);
    assert(get_no_nodes(LIVE_JOURNAL) == LIVE_JOURNAL_NO_NODES);
    assert(get_no_nodes(ORKUT) == ORKUT_NO_NODES);
    assert(get_no_nodes(FRIENDSTER) == FRIENDSTER_NO_NODES);
}

void
test_get_no_rels(void)
{
    assert(get_no_rels(C_ELEGANS) == C_ELEGANS_NO_RELS);
    assert(get_no_rels(EMAIL_EU_CORE) == EMAIL_EU_CORE_NO_RELS);
    assert(get_no_rels(DBLP) == DBLP_NO_RELS);
    assert(get_no_rels(AMAZON) == AMAZON_NO_RELS);
    assert(get_no_rels(YOUTUBE) == YOUTUBE_NO_RELS);
    assert(get_no_rels(WIKIPEDIA) == WIKIPEDIA_NO_RELS);
    assert(get_no_rels(LIVE_JOURNAL) == LIVE_JOURNAL_NO_RELS);
    assert(get_no_rels(ORKUT) == ORKUT_NO_RELS);
    assert(get_no_rels(FRIENDSTER) == FRIENDSTER_NO_RELS);
}

int
main(void)
{
    //  test_celegans();
    //  printf("Snap importer test: celegenas imported successfully\n");
    //  test_email();
    //  printf("Snap importer test: email eu imported successfully\n");
    //  test_dblp();
    //  printf("Snap importer test: dblp imported successfully\n");
    //  test_amazon();
    //  printf("Snap importer test: amazon imported successfully\n");
    //  test_youtube();
    //  printf("Snap importer test: youtube imported successfully\n");
    test_wikipedia();
    printf("Snap importer test: wikipedia imported successfully\n");
    test_live_journal();
    printf("Snap importer test: live journal imported successfully\n");
    test_orkut();
    printf("Snap importer test: orkut imported successfully\n");
    test_friendster();
    printf("Snap importer test: friendster imported successfully\n");
    test_get_url();
    printf("Snap importer test: tested urls successfully\n");
    test_get_no_nodes();
    printf("Snap importer test: tested get no nodes successfully\n");
    test_get_no_rels();
    printf("Snap importer test: tested get no rels successfully\n");
}
