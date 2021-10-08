#!/usr/bin/env python3
import networkx as nx

def main():
    G = nx.read_edgelist("/home/someusername/workspace_local/email_eu.txt", create_using=nx.DiGraph)

    communities = nx.algorithms.community.greedy_modularity_communities(G)

    i = 0
    seq = []
    for comm in communities:
        for node in comm:
            seq.append(node)

    with open("/home/someusername/workspace_local/node_seq.txt", 'w') as wf:
        for elem in seq:
           wf.write(str(elem) + "\n") 

if __name__ == "__main__":
    main()
