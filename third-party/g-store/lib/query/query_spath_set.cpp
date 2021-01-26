#include "query.h"
#include "fifo.h"

//#define FIFO_QUEUE
//#define PRINT_METRICS

#ifdef FIFO_QUEUE
FIFO *fifo_queue_curr;
FIFO *fifo_queue_next;
FIFO **p_fifo_curr;
FIFO **p_fifo_next;
#endif

q_spath_set::q_spath_set()
{
	lru = new LRUBuffer(QUERY::buffer_memory);
	//printf("buffer size: %u\n", QUERY::buffer_memory);
	//printf("page size:   %d\n", GLOBALS::blk_size);

	// one slot per page
	page_queue_curr = new PageQueue(GLOBALS::num_parts);
	page_queue_next = new PageQueue(GLOBALS::num_parts);

	p_pq_curr = &page_queue_curr;
	p_pq_next = &page_queue_next;

	v_queue_curr = new std::unordered_map< unsigned int, std::queue< int >* >();
	v_queue_next = new std::unordered_map< unsigned int, std::queue< int >* >();
	p_vq_curr = &v_queue_curr;
	p_vq_next = &v_queue_next;

#ifdef FIFO_QUEUE
	fifo_queue_curr = new FIFO();
	fifo_queue_next = new FIFO();
	p_fifo_curr = &fifo_queue_curr;
	p_fifo_next = &fifo_queue_next;
#endif
#ifdef PRINT_METRICS
	tot_reads = 0;
	page_reqs = 0;
	cache_hits = 0;
	mult_pages = 0;
	page_stats = new int[GLOBALS::num_parts];
	for (int i = 0; i < GLOBALS::num_parts; ++i)
		page_stats[i] = 0;
#endif
}

void q_spath_set::print_stats()
{
	printf("Total reads, ");
	printf("Page requests, ");
	printf("Cache hits, ");
	printf("Seq. r. 10, ");
	printf("Seq. r. 50, ");
	printf("Seq.r. 100, ");
	printf("Mult. pages, ");
	printf("Average reads\n");
	double average_reads = 0;
	double scale = (double)1/(double)GLOBALS::num_parts;
	for (int i = 0; i < GLOBALS::num_parts; ++i) {
		average_reads += page_stats[i];
	}
	//printf("Average reads: %.2f\n", (scale*average_reads)); 
	printf("%d, %d, %d, %d, %d, %d, %d, %.2f\n", 
			tot_reads, 
			page_reqs, 
			cache_hits, 
			lru->get_seq_reads(10), 
			lru->get_seq_reads(50), 
			lru->get_seq_reads(100), 
			mult_pages, 
			(scale*average_reads));

	//std::sort(page_stats, page_stats + GLOBALS::num_parts);
	//double avg = 0.0;
	//int RES = 22;

	//for (int i = 0; i < 1001; ++i) {
	//	avg = 0.0;
	//	for (int j = 0; j < RES; ++j) {
	//		avg += (double) page_stats[4+(i*RES)+j];
	//	}
	//	printf("%.2f, ", (avg/RES));
	//}
	//printf("\n");
}

q_spath_set::~q_spath_set()
{
#ifdef PRINT_METRICS
	delete page_stats;
#endif
#ifdef FIFO_QUEUE
	p_fifo_curr = NULL;
	p_fifo_next = NULL;
	delete fifo_queue_curr;
	delete fifo_queue_next;
#endif
	p_vq_curr = NULL;
	p_vq_next = NULL;
	v_queue_next->clear();
	v_queue_curr->clear();
	delete v_queue_next;
	delete v_queue_curr;
	label.clear();
	state.clear();
	p_pq_curr = NULL;
	p_pq_next = NULL;
	delete page_queue_next;
	delete page_queue_curr;
	delete lru;
}

void q_spath_set::start()
{	
	//printf("Entered start()...\n"); fflush(stdout);
	int buf_size = lru->get_num_slots();
	int* page_dequeue = new int[buf_size]; // dequeue as many as fit in buffer

	char*   page;
	header1 header;
	int     page_id;
	g_id    first_gid;
	g_id    v_gid;
	int     v_rel;

	int DEPTH = 0;

	std::queue< int >* queue;

	// fill page buffer with all pages of graph
	for (int page = 0; page < GLOBALS::num_parts; ++page) {
		(*p_pq_curr)->insert(page);
	}

	//printf("total # pages: %d\n", GLOBALS::num_parts);

	// Scan: evaluate all vertices looking for source set
	while (!(*p_pq_curr)->is_empty()) {
		// read without read-ahead
		int count = (*p_pq_curr)->next(page_dequeue, buf_size, false);
#ifdef PRINT_METRICS
		tot_reads += count;

		// stats...
		for (int i = 0; i < count; ++i) {
			int j = page_dequeue[i];
			if (page_stats[j] == 1)
				++mult_pages;
			++page_stats[j];
		}
#endif

		lru->fill(page_dequeue, count);

		// for each page in buffer...
		for (int buf_slot = 0; buf_slot < buf_size; ++buf_slot) {
			page	  = lru->get_page_by_slot(buf_slot);
			page_id	  = get_block_id(blk_get_first_gid(page));
			first_gid = blk_get_first_gid(page);

			b_sc vertices_in_page = blk_get_max_slt(page);

			// for each vertex in page... v_rel = relative to first_gid
			for (v_rel = 0; v_rel < vertices_in_page; ++v_rel) {
				v_gid = first_gid + v_rel;
				header = blk_get_header(page, v_rel);
				evaluate_label(page, header, first_gid, v_rel, label, v_gid);

				if (label[v_gid] == Source) {

					state[v_gid] = Discovered;
#ifdef FIFO_QUEUE	
					(*p_fifo_next)->insert(page_id);
#else
					(*p_pq_next)->insert(page_id);
#endif
					//printf("source: %d\n", page_id);
					if ((*p_vq_next)->find(page_id) == (*p_vq_next)->end()) {
						//std::queue< int > *new_queue = new std::queue< int >;
						(*(*p_vq_next))[page_id] = new std::queue< int >;//new_queue;
					}
					(*(*p_vq_next))[page_id]->push(v_rel);
				}
			}
		}
	} // end scan

	//printf("Finished scan.\n");fflush(stdout);

	// Traverse: find shortest path
	do {
#ifdef FIFO_QUEUE
		std::swap(p_fifo_next, p_fifo_curr);
#else		
		std::swap(p_pq_next, p_pq_curr);
#endif
		std::swap(p_vq_next, p_vq_curr);

		// traverse one layer in BFS tree
#ifdef FIFO_QUEUE
		while (!(*p_fifo_curr)->is_empty()) {
			int count = (*p_fifo_curr)->next(page_dequeue, buf_size, false);
#else
		while (!(*p_pq_curr)->is_empty()) {
			int count = (*p_pq_curr)->next(page_dequeue, buf_size, false);
#endif
#ifdef PRINT_METRICS
			page_reqs += count;
			tot_reads += count;

			// stats...
			for (int i = 0; i < count; ++i) {
				int j = page_dequeue[i];
				if (page_stats[j] == 1)
					++mult_pages;

				if (lru->contains(j))
					++cache_hits;
				else
					++page_stats[j];
			}
#endif

			lru->fill(page_dequeue, count);

			// for each page in buffer...
			for (int buf_slot = 0; buf_slot < buf_size; ++buf_slot) {
				page	  = lru->get_page_by_slot(buf_slot);
				page_id	  = get_block_id(blk_get_first_gid(page));
				first_gid = blk_get_first_gid(page);
				if ((*p_vq_curr)->find(page_id) == (*p_vq_curr)->end())
					continue; // skip current block if no vertex queue
				queue     = (*(*p_vq_curr))[page_id]; // queue for this page

				// traverse vertices of interest for page
				while (!queue->empty()) {
					v_rel = queue->front();
					queue->pop();
					v_gid = first_gid + v_rel;

					if (label[v_gid] == Destination) {
						register_finished();
						printf("Shortest path found to vertex with GID %u\n", v_gid);
						printf("Depth: %d\n", DEPTH);
#ifdef PRINT_METRICS
						print_stats();
#endif
						return;
					} else if (label[v_gid] != Unfit) {
						// iterate over all adjacent

						header = blk_get_header(page, v_rel);
						// for cycling through internal/external edge lists
						char* str1 = page + 
								get_header(header + GLOBALS::ie_in_h)
								+ GLOBALS::ie_fix_off;
						// end of internal vertex list
						char* ie_list_end = page + 
								get_header(header + GLOBALS::ee_in_h);
						// end of external vertex list
						char* ee_list_end = page + 
							get_header(header + GLOBALS::ee_end_in_h);

						// for each internal vertex
						while (str1 < ie_list_end) {
							//printf("exploring internal edges...\n");
							int u_rel  = get_ie_adv(str1);
							g_id u_gid = first_gid + u_rel;

							// if vertex undiscovered
							if (state.find(u_gid) == state.end()) {
								state[u_gid] = Discovered;
								parent[u_gid] = v_gid;

								if (label[u_gid] == Destination) {
									register_finished();
									printf("Shortest path found to vertex with GID %u\n", u_gid);
									printf("Depth: %d\n", DEPTH+1);
#ifdef PRINT_METRICS
									print_stats();
#endif
									return;
								}
#ifdef FIFO_QUEUE
								(*p_fifo_next)->insert(page_id);
#else
								(*p_pq_next)->insert(page_id);
#endif

								if ((*p_vq_next)->find(page_id) == (*p_vq_next)->end()) {
									//std::queue< int > *new_queue = new std::queue< int >;
									(*(*p_vq_next))[page_id] = new std::queue< int >;//new_queue;
								}
								(*(*p_vq_next))[page_id]->push(u_rel);
							}
						}

						str1 = ie_list_end;

						// for each external vertex
						while (str1 < ee_list_end) {
							//printf("exploring external edges...\n");
							g_id u_gid = get_ee_adv(str1);
							int u_rel  = get_slot(u_gid);

							// if vertex undiscovered
							if (state.find(u_gid) == state.end()) {
								state[u_gid] = Discovered;
								parent[u_gid] = v_gid;

								if (label[u_gid] == Destination) {
									register_finished();
									printf("Shortest path found to vertex with GID %u\n", u_gid);
									printf("Depth: %d\n", DEPTH+1);
#ifdef PRINT_METRICS
									print_stats();
#endif
									return;
								}
								
								int page_id_2 = get_block_id(u_gid); // pageof
#ifdef FIFO_QUEUE
								(*p_fifo_next)->insert(page_id_2);
#else
								(*p_pq_next)->insert(page_id_2);
#endif

								if ((*p_vq_next)->find(page_id_2) == (*p_vq_next)->end()) {
									//std::queue< int > *new_queue = new std::queue< int >;
									(*(*p_vq_next))[page_id_2] = new std::queue< int >;//new_queue;
								}
								(*(*p_vq_next))[page_id_2]->push(u_rel);
							}
						}
					}
				} // end traverse page
			} // end traverse buffer
		} // end traverse layer
		++DEPTH;
#ifdef FIFO_QUEUE
	} while (!(*p_fifo_next)->is_empty());
#else
	} while (!(*p_pq_next)->is_empty()); // end traverse
#endif
	queue = NULL;
	delete page_dequeue;

	register_finished();
	push_on_output("No path exists.\n");
#ifdef PRINT_METRICS
	print_stats();
#endif
	if (QUERY::must_close_output)
		print_ln("No path exists.");
	return;
}
