#include "louvain.h"

#include <stdlib.h>
#include <stdio.h>

#include "../access/in_memory_file.h"
#include "../data-struct/list_rel.h"
#include "../record/relationship.h"

// FIXME Implementation currently assumes undirected graphs

void update_partition(louvain_partition_t* p, unsigned long *part,
        unsigned long size) {
    // Renumber the communities in p
    unsigned long *renumber = calloc(p->size, sizeof(unsigned long));
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


louvain_graph_t* louvain_partition_to_graph(louvain_partition_t* p, louvain_graph_t* g) {
    size_t num_nodes = g->graph->node_id_counter;
    // Renumber communities
    unsigned long *renumber =
        (unsigned long *)calloc(num_nodes, sizeof(unsigned long));

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
    unsigned long *order = sort_by_partition(p->node_to_com, num_nodes);

    // Initialize meta graph
    louvain_graph_t* res = (louvain_graph_t*) malloc(sizeof(*res));
    res->total_weight = 0;
    res->graph = create_in_memory_file();

    for (size_t i = 0; i < last; ++i) {
        in_memory_create_node(res->graph);
    }

    // for each node (in community order), extract all edges to other communities
    // and build the graph
    init_neighbouring_communities(p);
    unsigned long old_com = p->node_to_com[order[0]];
    unsigned long cur_com = 0;
    double neigh_com_weight = 0;
    unsigned long node;
    last = 0;
    for (size_t i = 0; i <= p->size; i++) {
        // current node and current community with dummy values if out of bounds
        node = (i == p->size) ? 0 : order[i];
        cur_com =
            (i == p->size) ? cur_com + 1 : p->node_to_com[order[i]];

        // new community, write previous one
        if (old_com != cur_com) {
            // for all neighboring communities of current community add edges
            for (size_t j = 0; j < p->neigh_com_nb; j++) {
                neigh_com_weight = p->neigh_com_weights[p->neigh_com_pos[j]];
                in_memory_create_relationship_weighted(res->graph,
                        last, p->neigh_com_pos[j], (double) neigh_com_weight);
                res->total_weight += neigh_com_weight;
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
        get_neighbouring_communities_all(p, g, node);
    }
    printf("bad exit\n");
    return res;
}

void get_neighbouring_communities(louvain_partition_t* p, louvain_graph_t* g, unsigned long node) {
    unsigned long neigh;
    unsigned long neigh_com;
    double neigh_weight;
    p->neigh_com_pos[0] = p->node_to_com[node];
    p->neigh_com_weights[p->neigh_com_pos[0]] = 0.;
    p->neigh_com_nb = 1;

    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        neigh = rel->source_node == node ? rel->target_node : rel->source_node;
        neigh_com = p->node_to_com[neigh];
        neigh_weight = rel->weight;

        // if not a self-loop
        if (neigh != node) {
            // if community is new (weight == -1)
            if (p->neigh_com_weights[neigh_com] == -1) {
                p->neigh_com_pos[p->neigh_com_nb] = neigh_com;
                p->neigh_com_weights[neigh_com] = 0.;
                p->neigh_com_nb++;
            }
            p->neigh_com_weights[neigh_com] += neigh_weight;
        }
    }
    list_relationship_destroy(rels);
}

void get_neighbouring_communities_all(louvain_partition_t* p, louvain_graph_t* g, unsigned long node) {
    unsigned long neigh;
    unsigned long neigh_com;
    double neigh_weight;
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        neigh = rel->source_node == node ? rel->target_node : rel->source_node;
        neigh_com = p->node_to_com[neigh];
        neigh_weight = rel->weight;

        // if community is new
        if (p->neigh_com_weights[neigh_com] == -1) {
            p->neigh_com_pos[p->neigh_com_nb] = neigh_com;
            p->neigh_com_weights[neigh_com] = 0.;
            p->neigh_com_nb++;
        }
        p->neigh_com_weights[neigh_com] += neigh_weight;
    }
    list_relationship_destroy(rels);
}


int compare_by_partition(const void *a, const void *b, void *array2) {
    long diff = ((unsigned long *)array2)[*(unsigned long *)a]
        > ((unsigned long *)array2)[*(unsigned *)b];
    return (0 < diff) - (diff < 0);
}

unsigned long *sort_by_partition(unsigned long *part, unsigned long size) {
    unsigned long *nodes = (unsigned long *)malloc(size * sizeof(unsigned long));

    for (size_t i = 0; i < size; i++) {
        nodes[i] = i;
    }
    qsort_r(nodes, size, sizeof(unsigned long), compare_by_partition, (void *)part);
    return nodes;
}

double degree_weighted(louvain_graph_t* g, unsigned long node) {
    double res = 0.0;
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        res += rel->weight;
    }

    list_relationship_destroy(rels);
    return res;
}

double selfloop_weighted(louvain_graph_t* g, unsigned long node) {
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (rel->source_node == rel->target_node) {
            list_relationship_destroy(rels);
            return rel->weight;
        }
    }
    list_relationship_destroy(rels);
    return 0.0;
}

louvain_partition_t* create_louvain_partition(louvain_graph_t* g) {
    louvain_partition_t* p = malloc(sizeof(*p));

    p->size = g->graph->node_id_counter;

    p->node_to_com = malloc(p->size * sizeof(unsigned long));
    p->in = malloc(p->size * sizeof(double));
    p->tot = malloc(p->size * sizeof(double));

    p->neigh_com_weights = malloc(p->size * sizeof(double));
    p->neigh_com_pos = malloc(p->size * sizeof(unsigned long));
    p->neigh_com_nb = 0;

    for (size_t i = 0; i < p->size; i++) {
        p->node_to_com[i] = i;
        p->in[i] = selfloop_weighted(g, i);
        p->tot[i] = degree_weighted(g, i);
        p->neigh_com_weights[i] = -1;
        p->neigh_com_pos[i] = 0;
    }

    return p;
}

void louvain_part_destroy(louvain_partition_t* p) {
    free(p->in);
    free(p->tot);
    free(p->neigh_com_weights);
    free(p->neigh_com_pos);
    free(p->node_to_com);
    free(p);
}

void louvain_part_remove_node(louvain_partition_t* p, louvain_graph_t* g, unsigned long node,
        unsigned long comm, double dnodecomm) {
    p->in[comm] -= 2 * dnodecomm + selfloop_weighted(g, node);
    p->tot[comm] -= degree_weighted(g, node);
}

void louvain_part_insert_node(louvain_partition_t *p, louvain_graph_t *g, unsigned long node,
        unsigned long comm, double dnodecomm) {
    p->in[comm] += 2 * dnodecomm + selfloop_weighted(g, node);
    p->tot[comm] += degree_weighted(g, node);
    p->node_to_com[node] = comm;
}

double gain(louvain_partition_t* p, louvain_graph_t* g, unsigned long comm,
        double dnodecomm, double d_node) { // degc ? long double???
    double totc = p->tot[comm];
    double m2 = g->total_weight;

    return (dnodecomm - totc * d_node / m2);
}

double compute_modularity(louvain_partition_t* p, louvain_graph_t* g) {
    double q = 0.0F;
    double m2 = g->total_weight;

    for (size_t i = 0; i < p->size; i++) {
        if (p->tot[i] > 0.0L) {
            q += p->in[i] - (p->tot[i] * p->tot[i]) / m2;
        }
    }

    return q / m2;
}

void init_neighbouring_communities(louvain_partition_t* p) {
    for (size_t i = 0; i < p->neigh_com_nb; i++) {
        p->neigh_com_weights[p->neigh_com_pos[i]] = -1;
    }
    p->neigh_com_nb = 0;
}

double louvain_one_level(louvain_partition_t* p, louvain_graph_t* g) {
    unsigned long rand_pos;
    unsigned long tmp;
    unsigned long nb_moves;
    double start_modularity = compute_modularity(p, g);
    double new_modularity = start_modularity;
    double cur_modularity;
    unsigned long node;
    unsigned long old_com;
    unsigned long new_com;
    unsigned long best_com;
    double weighted_deg;
    double best_com_w;
    double best_gain;
    double new_gain;

    // generate a random order for nodes' movements
    unsigned long *rand_ord = malloc(p->size * sizeof(unsigned long));

    for (size_t i = 0; i < p->size; i++) {
        rand_ord[i] = i;
    }
    for (size_t i = 0; i < p->size - 1; i++) {
        rand_pos = rand() % (p->size);
        tmp = rand_ord[i];
        rand_ord[i] = rand_ord[rand_pos];
        rand_ord[rand_pos] = tmp;
    }

    // repeat while
    //   there are some nodes moving
    //   or there is an improvement of quality greater than a given epsilon
    do {
        cur_modularity = new_modularity;
        nb_moves = 0;

        // for each node:
        //   remove the node from its community
        //   compute the gain for its insertion in all neighboring communities
        //   insert it in the best community with the highest gain
        for (size_t i = 0; i < g->graph->node_id_counter; i++) {
            node = rand_ord[i];
            old_com = p->node_to_com[node];
            weighted_deg = degree_weighted(g, node);

            // computation of all neighboring communities of current node
            init_neighbouring_communities(p);
            get_neighbouring_communities(p, g, node);

            // remove node from its current community
            louvain_part_remove_node(p, g, node, old_com, p->neigh_com_weights[old_com]);

            // compute the gain for all neighboring communities
            // default choice is the former community
            best_com = old_com;
            best_com_w = 0L;
            best_gain = 0L;
            for (size_t j = 0; j < p->neigh_com_nb; j++) {
                new_com = p->neigh_com_pos[j];
                new_gain = gain(p, g, new_com, p->neigh_com_weights[new_com], weighted_deg);

                if (new_gain > best_gain) {
                    best_com = new_com;
                    best_com_w = p->neigh_com_weights[new_com];
                    best_gain = new_gain;
                }
            }
            // insert node in the nearest community
            louvain_part_insert_node(p, g, node, best_com, best_com_w);

            if (best_com != old_com) {
                nb_moves++;
            }
        }
        new_modularity = compute_modularity(p, g);
    } while (nb_moves != 0 && new_modularity - cur_modularity > MIN_IMPROVEMENT);

    free(rand_ord);

    return new_modularity - start_modularity;
}

unsigned long* louvain(in_memory_file_t* db) {
    unsigned long original_size = db->node_id_counter;
    unsigned long* partition = calloc(original_size, sizeof(unsigned long));
    louvain_graph_t* g = louvain_graph_init(db);
    louvain_graph_t* original = g;
    louvain_graph_t *g2;
    louvain_partition_t* gp;
    double improvement;

    // Initialize partition with trivial communities
    for (size_t i = 0; i < original_size; i++) {
        partition[i] = i;
    }

    gp = create_louvain_partition(original);
    improvement = louvain_one_level(gp, original);
    update_partition(gp, partition, original_size);
    g = louvain_partition_to_graph(gp, original);
    louvain_part_destroy(gp);

    // Execution of Louvain method
    while (improvement >= MIN_IMPROVEMENT) {
        gp = create_louvain_partition(g);
        improvement = louvain_one_level(gp, g);
        update_partition(gp, partition, original_size);

        g2 = louvain_partition_to_graph(gp, g);

        // TODO dont free
        louvain_graph_destroy(g);
        louvain_part_destroy(gp);
        g = g2;
    }

    free(original);
    return partition;
}

louvain_graph_t* louvain_graph_init(in_memory_file_t* db) {
    louvain_graph_t* result = malloc(sizeof(*result));
    result->graph = db;
    result->total_weight = 0;
    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        result->total_weight += rel->weight;
    }
    // Double the weight as we assume undirected edges for now.
    result->total_weight *= 2;
    // reference C impl did this, check against cpp impl.
    list_relationship_destroy(rels);

    return result;
}

void louvain_graph_destroy(louvain_graph_t* g) {
    in_memory_file_destroy(g->graph);
    free(g);
}

