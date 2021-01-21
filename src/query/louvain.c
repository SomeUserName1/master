#include "louvain.h"

#include "../data-struct/list_rel.h"

// FIXME Implementation currently assumes undirected graphs with nodes enumerated from 0 to n-1



unsigned long update_partition(louvain_partition_t* p, unsigned long *part,
        unsigned long size) {
    // Renumber the communities in p
    unsigned long *renumber = calloc(p->size, sizeof(unsigned long));
    unsigned long i, last = 1;
    for (i = 0; i < p->size; i++) {
        if (renumber[p->node_to_com[i]] == 0) {
            renumber[p->node_to_com[i]] = last++;
        }
    }

    // Update part with the renumbered communities in p
    for (i = 0; i < size; i++) {
        part[i] = renumber[p->node_to_com[part[i]]] - 1;
    }

    free(renumber);
    return last - 1;
}


louvain_graph_t* louvain_partition_to_graph(louvain_partition_t* p, louvain_graph_t* g) {
    unsigned long node, i, j;
    size_t num_nodes = dict_ul_node_size(g->graph->cache_nodes);
    // Renumber communities
    unsigned long *renumber =
        (unsigned long *)malloc(num_nodes * sizeof(unsigned long));

    for (node = 0; node < num_nodes; node++)
        renumber[node] = 0;

    unsigned long last = 1;
    for (node = 0; node < num_nodes; node++) {
        if (renumber[p->node_to_com[node]] == 0) {
            renumber[p->node_to_com[node]] = last++;
        }
    }

    for (node = 0; node < num_nodes; node++) {
        p->node_to_com[node] = renumber[p->node_to_com[node]] - 1;
    }

    // sort nodes according to their community
    unsigned long *order = sort_by_partition(p->node_to_com, num_nodes);

    // Initialize meta graph
    louvain_graph_t* res = (louvain_graph_t* )malloc(sizeof(adjlist));
    unsigned long long e1 = NLINKS2;
    res->total_weight = 0;
    res->map = NULL;

    // for each node (in community order), extract all edges to other communities
    // and build the graph
    init_neighbouring_communities(p);
    unsigned long old_com = p->node_to_com[order[0]];
    unsigned long currentComm;
    for (i = 0; i <= p->size; i++) {
        // current node and current community with dummy values if out of bounds
        node = (i == p->size) ? 0 : order[i];
        currentComm =
            (i == p->size) ? currentComm + 1 : p->node_to_com[order[i]];

        // new community, write previous one
        if (old_com != currentComm) {
            res->cd[old_com + 1] = res->cd[old_com] + p->neigh_com_nb;
            // for all neighboring communities of current community
            for (j = 0; j < p->neigh_com_nb; j++) {
                unsigned long neighComm = p->neigh_com_pos[j];
                long double neighCommWeight = p->neigh_com_weights[p->neigh_com_pos[j]];
                // add edge in res
                res->adj[res->e] = neighComm;
                res->weights[res->e] = neighCommWeight;
                res->totalWeight += neighCommWeight;
                (res->e)++;
                // reallocate edges and weights if necessary
                if (res->e == e1) {
                    e1 *= 2;
                    res->adj =
                        (unsigned long *)realloc(res->adj, e1 * sizeof(unsigned long));
                    res->weights =
                        (long double *)realloc(res->weights, e1 * sizeof(long double));
                    if (res->adj == NULL || res->weights == NULL) {
                        printf("error during memory allocation\n");
                        exit(0);
                    }
                }
            }
            if (i == p->size) {
                res->adj =
                    (unsigned long *)realloc(res->adj, res->e * sizeof(unsigned long));
                res->weights =
                    (long double *)realloc(res->weights, res->e * sizeof(long double));
                free(order);
                free(renumber);

                return res;
            }
            old_com = currentComm;
            init_neighbouring_communities(p);
        }
        // add neighbors of node i
        neighbouring_communities_all(p, g, node);
    }
    printf("bad exit\n");
    return res;
}

void get_neighbouring_communities(louvain_partition_t* p, louvain_graph_t* g, unsigned long node) {
    unsigned long long i;
    unsigned long neigh, neighComm;
    long double neighW;
    p->neigh_com_pos[0] = p->node_to_com[node];
    p->neigh_com_weights[p->neigh_com_pos[0]] = 0.;
    p->neigh_com_nb = 1;

    // for all neighbors of node, add weight to the corresponding community
    for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
        neigh = g->adj[i];
        neighComm = p->node_to_com[neigh];
        neighW = (g->weights == NULL) ? 1.0 : g->weights[i];

        // if not a self-loop
        if (neigh != node) {
            // if community is new (weight == -1)
            if (p->neigh_com_weights[neighComm] == -1) {
                p->neigh_com_pos[p->neigh_com_nb] = neighComm;
                p->neigh_com_weights[neighComm] = 0.;
                p->neigh_com_nb++;
            }
            p->neigh_com_weights[neighComm] += neighW;
        }
    }
}

void get_neighbouring_communities_all(louvain_partition_t* p, louvain_graph_t* g, unsigned long node) {
    unsigned long long i;
    unsigned long neigh, neighComm;
    long double neighW;

    for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
        neigh = g->adj[i];
        neighComm = p->node_to_com[neigh];
        neighW = (g->weights == NULL) ? 1.0 : g->weights[i];

        // if community is new
        if (p->neigh_com_weights[neighComm] == -1) {
            p->neigh_com_pos[p->neigh_com_nb] = neighComm;
            p->neigh_com_weights[neighComm] = 0.;
            p->neigh_com_nb++;
        }
        p->neigh_com_weights[neighComm] += neighW;
    }
}


int compare_by_partition(const void *a, const void *b, void *array2) {
    return ((unsigned long *)array2)[*(unsigned long *)a]
        == ((unsigned long *)array2)[*(unsigned *)b];
}

unsigned long *sort_by_partition(unsigned long *part, unsigned long size) {
    unsigned long i;
    unsigned long *nodes = (unsigned long *)malloc(size * sizeof(unsigned long));

    for (i = 0; i < size; i++) {
        nodes[i] = i;
    }
    qsort_r(nodes, size, sizeof(unsigned long), compare_by_partition, (void *)part);
    return nodes;
}

inline long double degree_weighted(louvain_graph_t* g, unsigned long node) {
    long double res = 0.0;
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        res += rel->weight;
    }
    return res;
}

inline long double selfloop_weighted(louvain_graph_t* g, unsigned long node) {
    list_relationship_t* rels = in_memory_expand(g->graph, node, BOTH);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        if (rel->source_node == rel->target_node) {
            return rel->weight;
        }
    }
    return 0.0;
}

louvain_partition_t* create_louvain_partition(louvain_graph_t* g) {
    louvain_partition_t* p = malloc(sizeof(*p));

    p->size = dict_ul_node_size(g->graph->cache_nodes);

    p->node_to_com = malloc(p->size * sizeof(unsigned long));
    p->in = malloc(p->size * sizeof(long double));
    p->tot = malloc(p->size * sizeof(long double));

    p->neigh_com_weights = malloc(p->size * sizeof(long double));
    p->neigh_com_pos = malloc(p->size * sizeof(unsigned long));
    p->neigh_com_nb = 0;

    for (size_t i = 0; i < p->size; i++) {
        p->node_to_com[i] = i;
        p->in[i] = selfloop_weightedWeighted(g, i);
        p->tot[i] = degree_weighted(g, i);
        p->neigh_com_weights[i] = -1;
        p->neigh_com_pos[i] = 0;
    }

    return p;
}

void louvain_partit_destroy(louvain_partition_t* p) {
    free(p->in);
    free(p->tot);
    free(p->neigh_com_weights);
    free(p->neigh_com_pos);
    free(p->node_to_com);
    free(p);
}

inline void louvain_part_remove_node(louvain_partition_t* p, louvain_graph_t* g, unsigned long node,
        unsigned long comm, long double dnodecomm) {
    p->in[comm] -= 2.0L * dnodecomm + selfloop_weighted(g, node);
    p->tot[comm] -= degree_weighted(g, node);
}

inline void louvain_part_insert_node(louvain_partition_t *p, louvain_graph_t *g, unsigned long node,
        unsigned long comm, long double dnodecomm) {
    p->in[comm] += 2.0L * dnodecomm + selfloop_weighted(g, node);
    p->tot[comm] += degree_weighted(g, node);
    p->node_to_com[node] = comm;
}

inline long double gain(louvain_partition_t* p, louvain_graph_t* g, unsigned long comm,
        long double dnodecomm, long double d_node) { // degc ? long double???
    long double totc = p->tot[comm];
    long double m2 = g->totalWeight;

    return (dnc - totc * degc / m2);
}

long double compute_modularity(louvain_partition_t* p, louvain_graph_t* g) {
    long double q = 0.0L;
    long double m2 = g->totalWeight;

    for (size_t i = 0; i < p->size; i++) {
        if (p->tot[i] > 0.0L)
            q += p->in[i] - (p->tot[i] * p->tot[i]) / m2;
    }

    return q / m2;
}

void init_neighbouring_communities(louvain_partition_t* p) {
    for (size_t i = 0; i < p->neigh_com_nb; i++) {
        p->neigh_com_weights[p->neigh_com_pos[i]] = -1;
    }
    p->neigh_com_nb = 0;
}

long double louvain_one_level(louvain_partition_t* p, louvain_graph_t* g) {
    unsigned long rand_pos;
    unsigned long tmp;
    unsigned long nb_moves;
    long double start_modularity = modularity(p, g);
    long double new_modularity = start_modularity;
    long double cur_modularity;
    unsigned long i, j, node;
    unsigned long old_com, new_com, best_com;
    long double weighted_deg, best_com_w, best_gain, new_gain;

    // generate a random order for nodes' movements
    unsigned long *rand_ord =
        (unsigned long *)malloc(p->size * sizeof(unsigned long));

    for (unsigned long i = 0; i < p->size; i++) {
        rand_ord[i] = i;
    }
    for (unsigned long i = 0; i < p->size - 1; i++) {
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
        for (i = 0; i < dict_ul_node_size(g->graph->cache_nodes); i++) {
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
            best_com_w = 0.0L;
            best_gain = 0.0L;
            for (j = 0; j < p->neigh_com_nb; j++) {
                new_com = p->neigh_com_pos[j];
                new_gain = gain(p, g, new_com, p->neigh_com_weights[new_com], weighted_deg);

                if (new_gain > best_gain) {
                    best_com = new_com;
                    best_comm_w = p->neigh_com_weights[new_com];
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

    free(random_order);

    return new_modularity - start_modularity;
}

unsigned long* louvain(in_memory_file_t db) {
    unsigned long original_size = dict_ul_node_size(db->cache_nodes);
    unsigned long* partition = calloc(original_size, sizeof(unsigned long));
    louvain_graph_t* g = louvain_graph_init(db);
    louvain_graph_t* original = init;
    louvain_graph_t *g2;
    unsigned long n;
    long double improvement;

    // Initialize partition with trivial communities
    for (size_t i = 0; i < g->n; i++) {
        partition[i] = i;
    }

    // Execution of Louvain method
    while (1) {
        louvain_partition_t* gp = create_louvain_partition(g);

        improvement = louvain_one_level(gp, g);

        n = update_partition(gp, partition, originalSize);

        if (improvement < MIN_IMPROVEMENT) {
            louvain_part_destroy(gp);
            break;
        }

        g2 = louvain_partition_to_graph(gp, g);

        // free all graphs except the original one
        if (dict_ul_node_size(g->graph->cache_nodes) < originalSize) {
            louvain_graph_destroy(g);
        }
        freeLouvainPartition(gp);
        g = g2;
    }

    if (dict_ul_node_size(g->graph->cache_nodes) < originalSize) {
        louvain_graph_destroy(g);
    }

    free(original->map);
    free(original);
    return n;
}

louvain_graph_t* louvain_graph_init(db) {
    louvain_graph_t* result = malloc(sizeof(*result));
    result->graph = db;
    list_relationship_t* rels = in_memory_get_relationships(g->graph);
    relationship_t* rel = NULL;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        result->total_weight += rel->weight;
    }
    // Double the weight as we assume undirected edges for now.
    result->total_weight << 1;
    // reference C impl did this, check against cpp impl.
    result->map == NULL;

    return result;
}

void louvain_graph_destroy(louvain_graph_t* g) {
    in_memory_file_destroy(g->graph);
    free(g->map);
    free(g);
}

