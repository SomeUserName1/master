# include "in_memory_file.h"

in_memory_file_t* create_in_memory_file() {
    in_memory_file_t* file = (in_memory_file_t*) malloc(sizeof(*file));

    file->cache_nodes = create_dict_ul_node();
    file->cache_rels = create_dict_ul_rel();

    return file;
}

void in_memory_file_destroy(in_memory_file_t* db) {
    dict_ul_node_destroy(db->cache_nodes);
    dict_ul_rel_destroy(db->cache_rels);
    free(db);
}

node_t* in_memory_get_node(in_memory_file_t* db, unsigned long id) {
    return  dict_ul_node_get_direct(db->cache_nodes, id);
}

list_node_t* in_memory_get_nodes(in_memory_file_t* db) {
    list_node_t* nodes = create_list_node(LIST_NONE);
    node_t* node = NULL;
    dict_ul_node_iterator_t* it = create_dict_ul_node_iterator(db->cache_nodes);

    while(dict_ul_node_iterator_next(it, NULL, &node) > -1) {
           list_node_append(nodes, node);
    }
    return nodes;
}

relationship_t* in_memory_get_relationship(in_memory_file_t* db, unsigned long id) {
    return dict_ul_rel_get_direct(db->cache_rels, id);
}

list_relationship_t* in_memory_get_relationships(in_memory_file_t* db) {
    list_relationship_t* rels = create_list_relationship(LIST_NONE);
    relationship_t* rel = NULL;
    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(db->cache_rels);

    while(dict_ul_rel_iterator_next(it, NULL, &rel) > -1) {
           list_relationship_append(rels, rel);
    }
    return rels;
}

