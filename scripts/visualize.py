import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os

BASE_PATH = "/home/someusername/workspace_local"
DATASET_NAMES = ["c_elegans", "email_research_eu", "dblp", "amazon", "youtube"]
METHOD_NAMES = ["natural", "random", "louvain", "g-store", "icbl"]
SP = ["", "sil_"]
QUERY_NAMES = ["bfs", "dfs", "dijkstra", "a-star", "alt"]
SUFFIXES = ["_blocks.txt", "_pages.txt"]
IO_TYPES = ["block", "page"]

def io_comparison_figure(data: dict):
    labels = ['BFS', 'DFS', 'Dijkstra', 'A*', 'ALT']

    nodes_natural = data[0]['n']
    nodes_random = data[1]['n']
    nodes_louvain = data[2]['n']
    nodes_gstore = [3]['n']
    nodes_icbl = [4]['n']

    rels_natural = data[0]['r']
    rels_random = data[1]['r']
    rels_louvain = data[2]['r']
    rels_gstore = data[3]['r']
    rels_icbl = data[4]['r']

    x = np.arange(len(labels))  # the label locations
    width = 5  # the width of the bars

    fig, ax = plt.subplots()
    natural_rels = ax.bar(x - 2 * width / 5, rels_natural, width, label='Natural Order, Edges')
    natural_nodes = ax.bar(x - 2 * width / 5, nodes__natural, width, bottom=rels__natural, label='Natural Order, Nodes')

    random_rels = ax.bar(x - width / 5, rels_random, width, label='Random Order, Edges')
    random_nodes = ax.bar(x - width / 5, nodes_random, width, bottom=rels_random, label='Random Order, Nodes')

    louvain_rels = ax.bar(x, rels_louvain, width, label='Louvain Order, Edges')
    louvain_nodes = ax.bar(x, nodes_louvain, width, bottom=rels_louvain, label='Louvain Order, Nodes')

    gstore_rels = ax.bar(x + width / 5, rels_gstore, width, label='G-Store Order, Edges')
    gstre_nodes = ax.bar(x + width / 5, nodes_gstore, width, bottom=rels_gstore, label='G-Store Order, Nodes')

    icbl_rels = ax.bar(x + 2 * width / 5, rels_icbl, width, label='ICBL Order, Edges')
    icbl_nodes = ax.bar(x + 2 * width / 5, nodes_icbl, width, bottom=rels_icbl, label='ICBL Order, Nodes')

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel('Amount of accessed blocks')
    ax.set_title('Block IOs by query, layout and record type for ' + dataset)
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()

    # ax.bar_label(natural_rels, padding=3)
    # ax.bar_label(natural_nodes, padding=3)
    fig.tight_layout()

    plt.show()
    #TODO
    plt.savefig(path.join(p_path, dataset_str + "noise_" + str(noise) + "_clusters.pdf"))
    plt.close('all')

def import_data(dataset: str) -> dict:
    # always return a dict with method nodes & method rels as key, access nos as values.
    for method in METHOD_NAMES:
        for prefix in SP:
            for query in QUERY_NAMES:
                for suffix in SUFFIXES:
                    with open(os.path.join(BASE_PATH, dataset, method, prefix + query + suffix) , "r") as read_file: 
                        for line in read_file:
                            tokens = line.split(" ")
                            if (tokens[0] == 'N'):
                                result[method][query][prefix][suffix]['n'].append(tokens[1])
                            if (tokens[0] == 'R'):
                                result[method][query][prefix][suffix]['r'].append(tokens[1])
    return result


def sequential_access_figure(dataset):


def preprocess_ios(data):
    prev = -1
    for (i, method) in enumerate(METHOD_NAMES):
        for suffix in SUFFIXES:
            for (j, query) in enumerate(QUERY_NAMES):
                for t in ['n', 'r']:
                    io_stats[IO_TYPES[0]][i][t][j] = 0
                    io_stats[IO_TYPES[1]][i][t][j] = 0

                    if (suffix == SUFFIXES[0]):
                        for value in data[method][query][SP[0]][suffix][t]:
                            if (prev != value):
                                io_stats[IO_TYPES[0]][i][t][j] += 1
                            prev = value
                    if (suffix == SUFFIXES[1]):
                        for value in data[method][query][SP[0]][suffix][t]:
                            if (prev != value):
                                io_stats[IO_TYPES[1]][i][t][j] += 1
                            prev = value



def main():
    for dataset in DATASET_NAMES:
        import_data(dataset)
        
    #import accessed blocks per dataset and plot layout block io comparison
        io_stats = preprocess_ios(data)
        io_comparison_figure(io_stats[IO_TYPES[0]], dataset)
        io_comparison_figure(io_stats[IO_TYPES[1]], dataset)

    #import relationship block accesses per dataset with and without sil & plot as time series
        sequential_access_figure(data, dataset)



if __name__ == '__main__':
    main() 
