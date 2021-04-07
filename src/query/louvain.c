#include "louvain.h"

#include <stdio.h>
#include <stdlib.h>

#include "../access/in_memory_file.h"
#include "../data-struct/list_rel.h"
#include "../record/relationship.h"

louvain_graph_t*
louvain_graph_init(in_memory_file_t* db)
{
    if (!db) {

        printf("louvain: louvain_graph_init: Passed NULL ptr as argument!\n");
        exit(-1);
    }

    louvain_graph_t* result = malloc(sizeof(*result));

    if (!result) {
        printf("louvain: louvain_graph_init: Allocating memory failed!\n");
        exit(-1);
    }

    result->graph             = db;
    result->m2                = 0;
    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t*      rel  = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        result->m2 += rel->weight;
    }

    result->m2 *= 2;

    list_relationship_destroy(rels);

    return result;
}

void
louvain_graph_destroy(louvain_graph_t* g)
{
    in_memory_file_destroy(g->graph);
    free(g);
}

louvain_partition_t*
create_louvain_partition(louvain_graph_t* g)
{
    if (!g) {
        printf("louvain: create_louvain_partition: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    louvain_partition_t* p = malloc(sizeof(*p));

    if (!p) {
        printf(
              "louvain: create_louvain_partition: Allocating memory failed!\n");
        exit(-1);
    }

    p->size = g->graph->node_id_counter;

    p->node_to_com = malloc(p->size * sizeof(unsigned long));
    p->in          = malloc(p->size * sizeof(double));
    p->tot         = malloc(p->size * sizeof(double));

    p->neigh_com_weights = malloc(p->size * sizeof(double));
    p->neigh_com_pos     = malloc(p->size * sizeof(unsigned long));
    p->neigh_com_nb      = 0;

    for (size_t i = 0; i < p->size; i++) {
        p->node_to_com[i]       = i;
        p->in[i]                = selfloop_weighted(g, i);
        p->tot[i]               = degree_weighted(g, i);
        p->neigh_com_weights[i] = -1;
        p->neigh_com_pos[i]     = 0;
    }

    return p;
}

void
louvain_part_destroy(louvain_partition_t* p)
{
    if (!p) {
        printf("louvain: louvain_part_destroy: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    free(p->in);
    free(p->tot);
    free(p->neigh_com_weights);
    free(p->neigh_com_pos);
    free(p->node_to_com);
    free(p);
}

void
update_partition(louvain_partition_t* p,
                 unsigned long*       part,
                 unsigned long        size)
{
    if (!p || !part) {
        printf("louvain: update partition: Passed NULL ptr as argument!\n");
        exit(-1);
    }
    // Renumber the communities in p
    unsigned long* renumber = calloc(p->size, sizeof(unsigned long));

    if (!renumber) {
        printf("louvain: update partition: Allocating memory failed!\n");
        exit(-1);
    }

    unsigned long last = 1;

    for (size_t i = 0; i < p->size; i++) {
        if (renumber[p->node_to_com[i]] == 0) {
            renumber[p->node_to_com[i]] = last++;
        }
    }

    // Update part with the renumbered communities in p
    for (size_t i = 0; i < size; i++) {
        part[i] = renumber[p->node_to_com[part[i]]] - 1;
    }

    free(renumber);
}

louvain_graph_t*
louvain_partition_to_graph(louvain_partition_t* p, louvain_graph_t* g)
{
    if (!p || !g) {
        printf("louvain: louvain_partition_to_graph: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    size_t num_nodes = p->size;

    // Renumber communities
    unsigned long* renumber = calloc(num_nodes, sizeof(unsigned long));

    if (!renumber) {
        printf("louvain: louvain_partition_to_graph: Allocating memory "
               "failed!\n");
        exit(-1);
    }

    unsigned long last = 1;
    for (size_t node = 0; node < num_nodes; node++) {
        if (renumber[p->node_to_com[node]] == 0) {
            renumber[p->node_to_com[node]] = last++;
        }
    }

    for (size_t node = 0; node < num_nodes; node++) {
        p->node_to_com[node] = renumber[p->node_to_com[node]] - 1;
    }
    free(renumber);

    // sort nodes according to their community
    unsigned long* order = sort_by_partition(p->node_to_com, num_nodes);

    // Initialize meta graph
    louvain_graph_t* res = malloc(sizeof(*res));

    if (!res) {
        printf("louvain: louvain_partition_to_graph: Allocating memory "
               "failed!\n");
        exit(-1);
    }

    res->m2    = 0;
    res->graph = create_in_memory_file();

    for (size_t i = 0; i < last; ++i) {
        in_memory_create_node(res->graph);
    }

    // for each node (in community order), extract all edges to other
    // communities and build the graph
    init_neighbouring_communities(p);
    unsigned long old_com = p->node_to_com[order[0]];
    unsigned long cur_com = 0;
    double        neigh_com_weight;
    unsigned long node;
    last = 0;
    for (size_t i = 0; i <= p->size; i++) {
        // current node and current community with dummy values if out of bounds
        node    = (i == p->size) ? 0 : order[i];
        cur_com = (i == p->size) ? cur_com + 1 : p->node_to_com[order[i]];

        // new community, write previous one
        if (old_com != cur_com) {
            // for all neighboring communities of current community add edges
            for (size_t j = 0; j < p->neigh_com_nb; j++) {
                neigh_com_weight = p->neigh_com_weights[p->neigh_com_pos[j]];
                in_memory_create_relationship_weighted(
                      res->graph,
                      last,
                      p->neigh_com_pos[j],
                      (double)neigh_com_weight);
                res->m2 += neigh_com_weight;
            }

            if (i == p->size) {
                free(order);
                return res;
            }

            last++;
            old_com = cur_com;
            init_neighbouring_communities(p);
        }
        // add neighbors of node i
        get_neighbouring_communities(p, g, node, true);
    }
    printf("Should be unreachable\n");
    exit(-1);
}

int
compare_by_partition(const void* a, const void* b, void* array2)
{
    long diff = ((unsigned long*)array2)[*(unsigned long*)a]
                > ((unsigned long*)array2)[*(unsigned*)b];
    return (0 < diff) - (diff < 0);
}

unsigned long*
sort_by_partition(unsigned long* part, unsigned long size)
{
    if (!part) {
        printf("louvain: sort_by_partition: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    unsigned long* nodes = malloc(size * sizeof(unsigned long));

    if (!nodes) {
        printf("louvain: sort_by_partition: Allocating memory failed!\n");

        exit(-1);
    }

    for (size_t i = 0; i < size; i++) {
        nodes[i] = i;
    }
    qsort_r(nodes,
            size,
            sizeof(unsigned long),
            compare_by_partition,
            (void*)part);
    return nodes;
}

void
init_neighbouring_communities(louvain_partition_t* p)
{
    if (!p) {
        printf("louvain: init_neighbouring_communities: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    for (size_t i = 0; i < p->neigh_com_nb; i++) {
        p->neigh_com_weights[p->neigh_com_pos[i]] = -1;
    }
    p->neigh_com_nb = 0;
}

void
get_neighbouring_communities(louvain_partition_t* p,
                             louvain_graph_t*     g,
                             unsigned long        node,
                             bool                 all)
{
    if (!p || !g) {
        printf("louvain: get_neighbouring_communities: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    unsigned long        neigh;
    unsigned long        neigh_com;
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t*      rel  = NULL;

    if (!all) {
        p->neigh_com_pos[0]                       = p->node_to_com[node];
        p->neigh_com_weights[p->neigh_com_pos[0]] = 0.;
        p->neigh_com_nb                           = 1;
    }

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel   = list_relationship_get(rels, i);
        neigh = rel->source_node == node ? rel->target_node : rel->source_node;
        neigh_com = p->node_to_com[neigh];

        // if not a self-loop
        if (all || neigh != node) {
            // if community is new (weight == -1)
            if (p->neigh_com_weights[neigh_com] == -1) {
                p->neigh_com_pos[p->neigh_com_nb] = neigh_com;
                p->neigh_com_weights[neigh_com]   = 0.;
                p->neigh_com_nb++;
            }
            p->neigh_com_weights[neigh_com] += rel->weight;
        }
    }
    list_relationship_destroy(rels);
}

double
degree_weighted(louvain_graph_t* g, unsigned long node)
{
    if (!g) {
        printf("louvain: degree_weighted: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    double               res  = 0.0;
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t*      rel  = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        res += rel->weight;
    }

    list_relationship_destroy(rels);
    return res;
}

double
selfloop_weighted(louvain_graph_t* g, unsigned long node)
{
    if (!g) {
        printf("louvain: selfloop_weighted: Passed NULL ptr as "
               "argument!\n");
        exit(-1);
    }

    relationship_t* rel =
          in_memory_contains_relationship_from_to(g->graph, node, node, BOTH);

    return rel == NULL ? 0.0 : rel->weight;
}

double
compute_modularity(louvain_partition_t* p, louvain_graph_t* g)
{
    if (!p || !g) {
        printf("louvain: compute_modularity: Passed NULL ptr as argument!\n");
        exit(-1);
    }

    if (g->m2 == 0) {
        return 0;
    }

    double q = 0.0F;

    for (size_t i = 0; i < p->size; i++) {
        if (p->tot[i] > 0.0L) {
            q += p->in[i] - (p->tot[i] * p->tot[i]) / g->m2;
        }
    }

    return q / g->m2;
}

double
louvain_one_level(louvain_partition_t* p, louvain_graph_t* g)
{
    if (!p || !g || p->size == 0 || g->m2 == 0) {
        printf("louvain: louvain_one_level: Passed NULL ptr as argument or "
               "zero-sized partition!\n");
        exit(-1);
    }

    unsigned long  rand_pos;
    unsigned long  tmp;
    unsigned long  nb_moves;
    double         start_modularity = compute_modularity(p, g);
    double         new_modularity   = start_modularity;
    double         cur_modularity;
    unsigned long  node;
    unsigned long  old_com;
    unsigned long  new_com;
    unsigned long  best_com;
    double         weighted_deg;
    double         best_com_w;
    double         best_gain;
    double         new_gain;
    unsigned long* rand_ord = calloc(p->size, sizeof(*rand_ord));

    if (!rand_ord) {
        printf("louvain: louvain_one_level: Allocating memory failed!\n");
        exit(-1);
    }

    // generate a random order for nodes' movements
    for (size_t i = 0; i < p->size; i++) {
        rand_ord[i] = i;
    }

    for (size_t i = 0; i < p->size - 1; i++) {
        rand_pos           = rand() % (p->size);
        tmp                = rand_ord[i];
        rand_ord[i]        = rand_ord[rand_pos];
        rand_ord[rand_pos] = tmp;
    }

    // repeat while
    //   there are some nodes moving
    //   or there is an improvement of quality greater than a given epsilon
    do {
        cur_modularity = new_modularity;
        nb_moves       = 0;

        // for each node:
        //   remove the node from its community
        //   compute the gain for its insertion in all neighboring communities
        //   insert it in the best community with the highest gain
        for (size_t i = 0; i < p->size; i++) {
            node         = rand_ord[i];
            old_com      = p->node_to_com[node];
            weighted_deg = degree_weighted(g, node);

            // computation of all neighboring communities of current node
            init_neighbouring_communities(p);
            get_neighbouring_communities(p, g, node, false);

            // remove node from its current community
            p->in[old_com] -= 2 * p->neigh_com_weights[old_com]
                              + selfloop_weighted(g, node);
            p->tot[old_com] -= degree_weighted(g, node);

            // compute the gain for all neighboring communities
            // default choice is the former community
            best_com   = old_com;
            best_com_w = 0L;
            best_gain  = 0L;
            for (size_t j = 0; j < p->neigh_com_nb; j++) {
                new_com  = p->neigh_com_pos[j];
                new_gain = (p->neigh_com_weights[new_com]
                            - p->tot[new_com] * weighted_deg / g->m2);

                if (new_gain > best_gain) {
                    best_com   = new_com;
                    best_com_w = p->neigh_com_weights[new_com];
                    best_gain  = new_gain;
                }
            }
            // insert node in the nearest community
            p->in[best_com] += 2 * best_com_w + selfloop_weighted(g, node);
            p->tot[best_com] += degree_weighted(g, node);
            p->node_to_com[node] = best_com;

            if (best_com != old_com) {
                nb_moves++;
            }
        }
        new_modularity = compute_modularity(p, g);
    } while (nb_moves != 0
             && new_modularity - cur_modularity > MIN_IMPROVEMENT);

    free(rand_ord);

    return new_modularity - start_modularity;
}

unsigned long*
louvain(in_memory_file_t* db)
{
    if (!db) {
        printf("louvain: louvain: Passed NULL ptr as argument!\n");
        exit(-1);
    }

    unsigned long  original_size = db->node_id_counter;
    unsigned long* partition     = calloc(original_size, sizeof(unsigned long));

    if (!partition) {
        printf("louvain: louvain: Allocating memory failed!\n");
        exit(-1);
    }

    louvain_graph_t*     g        = louvain_graph_init(db);
    louvain_graph_t*     original = g;
    louvain_graph_t*     g2;
    louvain_partition_t* gp;
    double               improvement;

    // Initialize partition with trivial communities
    for (size_t i = 0; i < original_size; i++) {
        partition[i] = i;
    }

    gp          = create_louvain_partition(original);
    improvement = louvain_one_level(gp, original);
    update_partition(gp, partition, original_size);
    g = louvain_partition_to_graph(gp, original);
    louvain_part_destroy(gp);

    // Execution of Louvain method
    while (improvement >= MIN_IMPROVEMENT) {
        gp          = create_louvain_partition(g);
        improvement = louvain_one_level(gp, g);
        update_partition(gp, partition, original_size);

        g2 = louvain_partition_to_graph(gp, g);

        louvain_graph_destroy(g);
        louvain_part_destroy(gp);
        g = g2;
    }

    louvain_graph_destroy(g);
    free(original);

    return partition;
}

