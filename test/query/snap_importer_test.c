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
#define PATH_DBLP      ("/home/someusername/workspace_local/dblp.txt")
#define PATH_AMAZON    ("/home/someusername/workspace_local/amazon.txt")
#define PATH_YOUTUBE   ("/home/someusername/workspace_local/youtube.txt")
#define PATH_WIKIPEDIA ("/home/someusername/workspace_local/wikipeida.txt")
#define PATH_LIVE_JOURNAL                                                      \
    ("/home/someusername/workspace_local/live_journal.txt")
#define PATH_ORKUT      ("/home/someusername/workspace_local/orkut.txt")
#define PATH_FRIENDSTER ("/home/someusername/workspace_local/friendster.txt")

#define n(x) dict_ul_ul_get_direct(map[0], x)
#define r(x) dict_ul_ul_get_direct(map[1], x)

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
    heap_file* hf = prepare();
    import(hf, false, C_ELEGANS);

    assert(hf->n_nodes == C_ELEGANS_NO_NODES);
    assert(hf->n_rels == C_ELEGANS_NO_RELS);

    clean_up(hf);
}

void
test_email(void)
{
    heap_file* hf = prepare();
    import(hf, false, EMAIL_EU_CORE);

    assert(hf->n_nodes == EMAIL_EU_CORE_NO_NODES);
    assert(hf->n_rels == EMAIL_EU_CORE_NO_RELS);

    clean_up(hf);
}

void
test_dblp(void)
{
    heap_file* hf = prepare();
    import(hf, false, DBLP);

    assert(hf->n_nodes == DBLP_NO_NODES);
    assert(hf->n_rels == DBLP_NO_RELS);

    clean_up(hf);
}

void
test_amazon(void)
{
    heap_file* hf = prepare();
    import(hf, false, AMAZON);

    assert(hf->n_nodes == AMAZON_NO_NODES);
    assert(hf->n_rels == AMAZON_NO_RELS);

    clean_up(hf);
}

void
test_youtube(void)
{
    heap_file* hf = prepare();
    import(hf, false, YOUTUBE);

    assert(hf->n_nodes == YOUTUBE_NO_NODES);
    assert(hf->n_rels == YOUTUBE_NO_RELS);

    clean_up(hf);
}

void
test_wikipedia(void)
{
    heap_file* hf = prepare();
    import(hf, false, WIKIPEDIA);

    assert(hf->n_nodes == WIKIPEDIA_NO_NODES);
    assert(hf->n_rels == WIKIPEDIA_NO_RELS);

    clean_up(hf);
}

void
test_live_journal(void)
{
    heap_file* hf = prepare();
    import(hf, false, LIVE_JOURNAL);

    assert(hf->n_nodes == LIVE_JOURNAL_NO_NODES);
    assert(hf->n_rels == LIVE_JOURNAL_NO_RELS);

    clean_up(hf);
}

void
test_orkut(void)
{
    heap_file* hf = prepare();
    import(hf, false, ORKUT);

    assert(hf->n_nodes == ORKUT_NO_NODES);
    assert(hf->n_rels == ORKUT_NO_RELS);

    clean_up(hf);
}

void
test_friendster(void)
{
    heap_file* hf = prepare();
    import(hf, false, FRIENDSTER);

    assert(hf->n_nodes == FRIENDSTER_NO_NODES);
    assert(hf->n_rels == FRIENDSTER_NO_RELS);

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

void // NOLINTNEXTLINE
test_rel_chain(void)
{
    heap_file* hf = prepare();

    dict_ul_ul** map = import_from_txt(hf, PATH_EMAIL, false, EMAIL_EU_CORE);

    node_t*         node = read_node(hf, 0);
    relationship_t* rel  = read_relationship(hf, node->first_relationship);
    free(node);
    unsigned long next_id;
    // 1.

    assert(rel->id == r(0));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(1));
    assert(rel->prev_rel_source == r(25511));
    assert(rel->next_rel_source == r(411));
    assert(rel->prev_rel_target == r(25223));
    assert(rel->next_rel_target == r(225));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 2.
    assert(rel->id == r(411));
    assert(rel->source_node == n(17));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(410));
    assert(rel->next_rel_source == r(412));
    assert(rel->prev_rel_target == r(0));
    assert(rel->next_rel_target == r(2181));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 3.
    assert(rel->id == r(2181));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(316));
    assert(rel->prev_rel_source == r(411));
    assert(rel->next_rel_source == r(2265));
    assert(rel->prev_rel_target == r(1737));
    assert(rel->next_rel_target == r(2265));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 4.
    assert(rel->id == r(2265));
    assert(rel->source_node == n(316));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(2181));
    assert(rel->next_rel_source == r(2982));
    assert(rel->prev_rel_target == r(2181));
    assert(rel->next_rel_target == r(2388));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);

    // 5.
    assert(rel->id == r(2388));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(146));
    assert(rel->prev_rel_source == r(2265));
    assert(rel->next_rel_source == r(2430));
    assert(rel->prev_rel_target == r(125));
    assert(rel->next_rel_target == r(2430));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 6.
    assert(rel->id == r(2430));
    assert(rel->source_node == n(146));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(2388));
    assert(rel->next_rel_source == r(2434));
    assert(rel->prev_rel_target == r(2388));
    assert(rel->next_rel_target == r(3476));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 7.
    assert(rel->id == r(3476));
    assert(rel->source_node == n(581));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(3350));
    assert(rel->next_rel_source == r(3501));
    assert(rel->prev_rel_target == r(2430));
    assert(rel->next_rel_target == r(3854));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 8.
    assert(rel->id == r(3854));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(268));
    assert(rel->prev_rel_source == r(3476));
    assert(rel->next_rel_source == r(4278));
    assert(rel->prev_rel_target == r(3081));
    assert(rel->next_rel_target == r(4375));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 9.
    assert(rel->id == r(4278));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(581));
    assert(rel->prev_rel_source == r(3854));
    assert(rel->next_rel_source == r(4741));
    assert(rel->prev_rel_target == r(4237));
    assert(rel->next_rel_target == r(4327));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 10.
    assert(rel->id == r(4741));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(221));
    assert(rel->prev_rel_source == r(4278));
    assert(rel->next_rel_source == r(5671));
    assert(rel->prev_rel_target == r(4729));
    assert(rel->next_rel_target == r(4742));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 11.
    assert(rel->id == r(5671));
    assert(rel->source_node == n(218));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(5670));
    assert(rel->next_rel_source == r(5672));
    assert(rel->prev_rel_target == r(4741));
    assert(rel->next_rel_target == r(5742));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 12.
    assert(rel->id == r(5742));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(18));
    assert(rel->prev_rel_source == r(5671));
    assert(rel->next_rel_source == r(5744));
    assert(rel->prev_rel_target == r(5439));
    assert(rel->next_rel_target == r(5768));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 13.
    assert(rel->id == r(5744));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(734));
    assert(rel->prev_rel_source == r(5742));
    assert(rel->next_rel_source == r(5751));
    assert(rel->prev_rel_target == r(5345));
    assert(rel->next_rel_target == r(5751));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 14.
    assert(rel->id == r(5751));
    assert(rel->source_node == n(734));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(5744));
    assert(rel->next_rel_source == r(8606));
    assert(rel->prev_rel_target == r(5744));
    assert(rel->next_rel_target == r(5768));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 15.
    assert(rel->id == r(5768));
    assert(rel->source_node == n(18));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(5742));
    assert(rel->next_rel_source == r(9364));
    assert(rel->prev_rel_target == r(5751));
    assert(rel->next_rel_target == r(5826));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 16.
    assert(rel->id == r(5826));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(178));
    assert(rel->prev_rel_source == r(5768));
    assert(rel->next_rel_source == r(5871));
    assert(rel->prev_rel_target == r(5441));
    assert(rel->next_rel_target == r(5871));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 17.
    assert(rel->id == r(5871));
    assert(rel->source_node == n(178));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(5826));
    assert(rel->next_rel_source == r(6081));
    assert(rel->prev_rel_target == r(5826));
    assert(rel->next_rel_target == r(5875));
    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 18.
    assert(rel->id == r(5875));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(380));
    assert(rel->prev_rel_source == r(5871));
    assert(rel->next_rel_source == r(6301));
    assert(rel->prev_rel_target == r(5606));
    assert(rel->next_rel_target == r(5935));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 19.
    assert(rel->id == r(6301));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(5875));
    assert(rel->next_rel_source == r(6618));
    assert(rel->prev_rel_target == r(5875));
    assert(rel->next_rel_target == r(6618));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 20.
    assert(rel->id == r(6618));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(459));
    assert(rel->prev_rel_source == r(6301));
    assert(rel->next_rel_source == r(7100));
    assert(rel->prev_rel_target == r(6433));
    assert(rel->next_rel_target == r(6735));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 21.
    assert(rel->id == r(7100));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(215));
    assert(rel->prev_rel_source == r(6618));
    assert(rel->next_rel_source == r(7960));
    assert(rel->prev_rel_target == r(6839));
    assert(rel->next_rel_target == r(7689));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 22.
    assert(rel->id == r(7960));
    assert(rel->source_node == n(221));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(6983));
    assert(rel->next_rel_source == r(8856));
    assert(rel->prev_rel_target == r(7100));
    assert(rel->next_rel_target == r(8517));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 23.
    assert(rel->id == r(8517));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(250));
    assert(rel->prev_rel_source == r(7960));
    assert(rel->next_rel_source == r(8518));
    assert(rel->prev_rel_target == r(8049));
    assert(rel->next_rel_target == r(8548));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 24.
    assert(rel->id == r(8518));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(148));
    assert(rel->prev_rel_source == r(8517));
    assert(rel->next_rel_source == r(8548));
    assert(rel->prev_rel_target == r(6191));
    assert(rel->next_rel_target == r(10196));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 25.
    assert(rel->id == r(8548));
    assert(rel->source_node == n(250));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(8517));
    assert(rel->next_rel_source == r(8723));
    assert(rel->prev_rel_target == r(8518));
    assert(rel->next_rel_target == r(9155));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 26.
    assert(rel->id == r(9155));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(73));
    assert(rel->prev_rel_source == r(8548));
    assert(rel->next_rel_source == r(9696));
    assert(rel->prev_rel_target == r(5786));
    assert(rel->next_rel_target == r(9250));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 27.
    assert(rel->id == r(9696));
    assert(rel->source_node == n(74));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(9695));
    assert(rel->next_rel_source == r(9697));
    assert(rel->prev_rel_target == r(9155));
    assert(rel->next_rel_target == r(10385));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 28.
    assert(rel->id == r(10385));
    assert(rel->source_node == n(248));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(10266));
    assert(rel->next_rel_source == r(11555));
    assert(rel->prev_rel_target == r(9696));
    assert(rel->next_rel_target == r(10654));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 29.
    assert(rel->id == r(10654));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(498));
    assert(rel->prev_rel_source == r(10385));
    assert(rel->next_rel_source == r(10655));
    assert(rel->prev_rel_target == r(10504));
    assert(rel->next_rel_target == r(10655));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 30.
    assert(rel->id == r(10655));
    assert(rel->source_node == n(498));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(10654));
    assert(rel->next_rel_source == r(10662));
    assert(rel->prev_rel_target == r(10654));
    assert(rel->next_rel_target == r(10895));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 31.
    assert(rel->id == r(10895));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(226));
    assert(rel->prev_rel_source == r(10655));
    assert(rel->next_rel_source == r(10990));
    assert(rel->prev_rel_target == r(10832));
    assert(rel->next_rel_target == r(11106));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 32.
    assert(rel->id == r(10990));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(101));
    assert(rel->prev_rel_source == r(10895));
    assert(rel->next_rel_source == r(11093));
    assert(rel->prev_rel_target == r(10750));
    assert(rel->next_rel_target == r(12640));

    next_id = rel->next_rel_source;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 33.
    assert(rel->id == r(11093));
    assert(rel->source_node == n(377));
    assert(rel->target_node == n(0));
    assert(rel->prev_rel_source == r(10715));
    assert(rel->next_rel_source == r(11102));
    assert(rel->prev_rel_target == r(10990));
    assert(rel->next_rel_target == r(11096));

    next_id = rel->next_rel_target;
    free(rel);
    rel = read_relationship(hf, next_id);
    // 34.
    assert(rel->id == r(11096));
    assert(rel->source_node == n(0));
    assert(rel->target_node == n(218));
    assert(rel->prev_rel_source == r(11093));
    assert(rel->next_rel_source == r(11102));
    assert(rel->prev_rel_target == r(11083));
    assert(rel->next_rel_target == r(11106));

    free(rel);

    dict_ul_ul_destroy(map[0]);
    dict_ul_ul_destroy(map[1]);
    free(map);

    page_cache*   pc  = hf->cache;
    phy_database* pdb = hf->cache->pdb;
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_get_nodes_large(void)
{
    heap_file* hf = prepare();

    import(hf, false, YOUTUBE);

    array_list_node* nodes = get_nodes(hf);
    assert(array_list_node_size(nodes) == hf->n_nodes);
    assert(array_list_node_size(nodes) == YOUTUBE_NO_NODES);

    array_list_node_destroy(nodes);
    clean_up(hf);
}

void
test_get_relationships_large(void)
{
    heap_file* hf = prepare();

    import(hf, false, YOUTUBE);

    array_list_relationship* rels = get_relationships(hf);
    assert(array_list_relationship_size(rels) == hf->n_rels);
    assert(array_list_relationship_size(rels) == YOUTUBE_NO_RELS);

    array_list_relationship_destroy(rels);
    clean_up(hf);
}

int
main(void)
{
    test_celegans();
    printf("Snap importer test: celegenas imported successfully\n");
    test_email();
    printf("Snap importer test: email eu imported successfully\n");
    test_dblp();
    printf("Snap importer test: dblp imported successfully\n");
    test_amazon();
    printf("Snap importer test: amazon imported successfully\n");
    test_youtube();
    printf("Snap importer test: youtube imported successfully\n");
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
    test_rel_chain();
    printf("snap importer test: test rel chain successful!\n");
    test_get_nodes_large();
    printf("snap importer test: test get nodes successful!\n");
    test_get_relationships_large();
    printf("snap importer test: test get relationships successful!\n");
}
