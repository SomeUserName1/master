import matplotlib.pyplot as plt
import numpy as np
import os
import random

BASE_PATH = "/home/someusername/workspace_local"
BASE_OUT_PATH = "/home/someusername/bwsync/workspace/uni/master/doc/plots"
BASE_OUT_PATH_T = "/home/someusername/bwsync/workspace/uni/master/doc/data"
DATASET_NAMES = ["c_elegans", "email_research_eu", "dblp", "amazon", "youtube"]
DATASET_ALL = [True, True, False, False, False]
METHOD_NAMES = ["natural", "random", "louvain", "g-store", "icbl"]
MNT = ["Natural", "Random", "Louvain", "G-Store", "ICBL"]
SP = ["", "sil_"]
SP_TEXT = ["unsorted", "sorted"]
QUERY_NAMES = ["bfs", "dfs", "dijkstra", "a-star", "alt"]
SUFFIXES = ["_blocks.txt", "_pages.txt"]
IO_TYPES = ["Block", "Page"]
RECORD_TYPE = ["N", "R"]


def io_comparison_figure(data: list, dataset: str, data_all: bool):
    labels = ['BFS', 'DFS', 'Dijkstra', 'A*', 'ALT']
    for (sil, _) in enumerate(SP):
        for (io, io_t) in enumerate(IO_TYPES):
            nodes_natural = data[0][io][0][0]
            nodes_random = data[0][io][1][0]
            nodes_louvain = data[0][io][2][0]
            nodes_gstore = data[0][io][3][0]

            rels_natural = data[0][io][0][1]
            rels_random = data[0][io][1][1]
            rels_louvain = data[0][io][2][1]
            rels_gstore = data[0][io][3][1]

            x = np.arange(len(labels))  # the label locations
            fig, ax = plt.subplots()

            if data_all:
                nodes_icbl = data[0][io][4][0]
                rels_icbl = data[0][io][4][1]

                width = 0.14  # the width of the bars
                ax.bar(x - 2 * width, rels_natural, width, color=(0, 0, 1), label='Nat')
                ax.bar(x - 2 * width, nodes_natural, width, bottom=rels_natural, color=(0, 0, 0.4))
                ax.bar(x - width, rels_random, width, color=(0, 1, 0), label='Rand')
                ax.bar(x - width, nodes_random, width, bottom=rels_random, color=(0, 0.4, 0))
                ax.bar(x, rels_louvain, width, color=(1, 0, 0), label='Lou')
                ax.bar(x, nodes_louvain, width, bottom=rels_louvain, color=(0.4, 0, 0))
                ax.bar(x + width, rels_gstore, width, color=(1, 0.5, 0), label='G-Sto')
                ax.bar(x + width, nodes_gstore, width, bottom=rels_gstore, color=(0.5, .25, 0))
                ax.bar(x + 2 * width, rels_icbl, width, color=(0, 1, 1), label='ICBL')
                ax.bar(x + 2 * width, nodes_icbl, width, bottom=rels_icbl, color=(0, .5, .5))
            else:
                width = 0.175
                ax.bar(x - 1.5 * width, rels_natural, width, color=(0, 0, 1), label='Nat, R')
                ax.bar(x - 1.5 * width, nodes_natural, width, bottom=rels_natural, color=(0, 0, 0.4), label='Nat, N')
                ax.bar(x - 0.5 * width, rels_random, width, color=(0, 1, 0), label='Rand, R')
                ax.bar(x - 0.5 * width, nodes_random, width, bottom=rels_random, color=(0, 0.4, 0), label='Rand, N')
                ax.bar(x + 0.5 * width, rels_louvain, width, color=(1, 0, 0), label='Lou, R')
                ax.bar(x + 0.5 * width, nodes_louvain, width, bottom=rels_louvain, color=(0.4, 0, 0), label='Lou, N')
                ax.bar(x + 1.5 * width, rels_gstore, width, color=(1, 0.5, 0), label='G-Sto, R')
                ax.bar(x + 1.5 * width, nodes_gstore, width, bottom=rels_gstore, color=(0.5, .25, 0), label='G-Sto, N')

            # Add some text for labels, title and custom x-axis tick labels, etc.
            ax.set_ylabel('Amount of accessed blocks')

            stri = ""
            if sil == 1:
                stri = "with sorted incidence list"

            ax.set_title(io_t + ' IOs for ' + dataset + stri)
            ax.set_xticks(x)
            ax.set_xticklabels(labels)
            plt.tight_layout()
            ax.legend(bbox_to_anchor=(1.04, 1), loc="upper left", borderaxespad=0.)

            # ax.bar_label(natural_rels, padding=3)
            # ax.bar_label(natural_nodes, padding=3)
            plt.savefig(os.path.join(BASE_OUT_PATH, dataset + "_" + io_t + "_" + SP_TEXT[0] + "_"
                                     + "io_comparison.pdf"), bbox_inches="tight")
            plt.close('all')


def import_data(dataset: str, all_data: bool) -> list:
    result = []
    # always return a dict with method nodes & method rels as key, access nos as values.
    for (m, method) in enumerate(METHOD_NAMES):
        if not all_data and method == "icbl":
            continue

        result.append([])
        for (q, query) in enumerate(QUERY_NAMES):
            result[m].append([])
            for (p, prefix) in enumerate(SP):
                result[m][q].append([])
                for (s, suffix) in enumerate(SUFFIXES):
                    result[m][q][p].append([])
                    for (t, _) in enumerate(RECORD_TYPE):
                        result[m][q][p][s].append([])

                    with open(os.path.join(BASE_PATH, dataset, method, prefix + query + suffix), "r") as read_file:
                        for line in read_file:
                            tokens = line.split(" ")
                            if tokens[0] == RECORD_TYPE[0]:
                                result[m][q][p][s][0].append(int(tokens[1]))
                            if tokens[0] == RECORD_TYPE[1]:
                                result[m][q][p][s][1].append(int(tokens[1]))
    return result


def print_data(data: list, dataset: str, all_data: bool):
    for (m, method) in enumerate(METHOD_NAMES):
        if not all_data and method == "icbl":
            continue

        for (q, query) in enumerate(QUERY_NAMES):
            for (p, prefix) in enumerate(SP):
                for (s, suffix) in enumerate(IO_TYPES):
                    for (t, ty) in enumerate(RECORD_TYPE):
                        for i in range(0, 10):
                            print(dataset + " " + method + " " + query + " " + suffix + " " + ty + ": " + str(
                                data[m][q][p][s][t][i]))


def sequential_access_figure(data, dataset, all_data):
    for (io, io_t) in enumerate(IO_TYPES):
        for (q, query) in enumerate(QUERY_NAMES):
            for (m, method) in enumerate(METHOD_NAMES):
                if not all_data and method == "icbl":
                    continue

                s1 = data[m][q][0][io][1]
                s2 = data[m][q][1][io][1]
                idx = random.randint(0, min(len(s1), len(s2)) - 50)

                s1 = s1[idx: idx + 50]
                s2 = s2[idx: idx + 50]

                max_len = max(len(s1), len(s2))
                max_y = max(max(s1), max(s2))

                x = np.arange(0, int(max_len))
                y = np.arange(0, int(max_y))
                t1 = np.arange(0, len(s1))
                t2 = np.arange(0, len(s2))
                fig, axs = plt.subplots(1, 1)
                axs.plot(t1, s1, label='Non-Sorted', zorder=-1)
                axs.plot(t2, s2, label='Sorted', zorder=1)
                axs.set_xlim(0, max_len)
                axs.set_ylim(0, max_y)
                axs.set_ylabel(y)
                axs.set_xlabel(x)
                axs.set_xlabel('Accesses')
                axs.set_ylabel('Address')
                axs.set_title(io_t + ' accesses of ' + query + ', with layout ' + method)
                plt.tight_layout()
                plt.legend(bbox_to_anchor=(1.04, 1), loc="upper left", borderaxespad=0.)
                plt.savefig(os.path.join(BASE_OUT_PATH, dataset + "_" + method + "_" + query + "_" + io_t + "_"
                                         + "sil_access_seq.png"), bbox_inches="tight")
                plt.close('all')


def preprocess_ios(data, all_data: bool) -> list:
    prev = -1
    io_stats = []
    for (sil, _) in enumerate(SP):
        io_stats.append([])
        for (s, _) in enumerate(SUFFIXES):
            io_stats[sil].append([])
            for (m, method) in enumerate(METHOD_NAMES):
                if not all_data and method == "icbl":
                    continue

                io_stats[sil][s].append([])
                for (t, _) in enumerate(RECORD_TYPE):
                    io_stats[sil][s][m].append([])
                    for (q, _) in enumerate(QUERY_NAMES):
                        io_stats[sil][s][m][t].append(0)

                        for value in data[m][q][sil][s][t]:
                            if prev != value:
                                io_stats[sil][s][m][t][q] += 1
                            prev = value
    return io_stats


def print_io_stats(io_stats: list, all_data: bool, dataset):
    for (sil, sort) in enumerate(SP):
        for (s, _) in enumerate(SUFFIXES):
            with open(os.path.join(BASE_OUT_PATH_T, dataset + "_" + IO_TYPES[s] + "_" + SP_TEXT[sil] + "_"
                                                    + "io_comparison.tex"), "w") as write_file:
                write_file.write("\\begin{table}\n\t\\begin{center}\n\t\t \\begin{tabular}[c]{c c c c c c} \\toprule\n"
                                 + "\t\t\t  & BFS & DFS & Dijkstra & $A^*$  & ALT \\\\ \\midrule \n \t\t\t")

                for (m, method) in enumerate(METHOD_NAMES):
                    if not all_data and method == "icbl":
                        continue
                    write_file.write("\\multirow{2}{*}{" + MNT[m] + "} ")

                    for (t, rec) in enumerate(RECORD_TYPE):
                        for (q, query) in enumerate(QUERY_NAMES):
                            write_file.write(" & " + str(io_stats[sil][s][m][t][q]))
                        write_file.write(" \\\\ \n \t\t\t\t")
                    write_file.write("&&&&& \\\\[-0.5em]\n \t\t\t")

                write_file.write("\t\t\\end{tabular}  \n  \t \\end{center}\n"
                                 + "\t \\caption{ Insert Caption here}\n"
                                   "\\end{table}")


def main():
    random.seed(os.getpid())
    for (d, dataset) in enumerate(DATASET_NAMES):
        data = import_data(dataset, DATASET_ALL[d])
        # print_data(data, dataset, DATASET_ALL[d])
        io_stats = preprocess_ios(data, DATASET_ALL[d])
        print_io_stats(io_stats, DATASET_ALL[d], dataset)
        io_comparison_figure(io_stats, dataset, DATASET_ALL[d])

        # sequential_access_figure(data, dataset, DATASET_ALL[d])
        del data


if __name__ == '__main__':
    main()
