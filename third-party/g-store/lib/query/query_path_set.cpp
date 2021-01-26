#include "query.h"
#include "fifo.h"

//#define FIFO_QUEUE
//#define PRINT_METRICS

#ifdef FIFO_QUEUE
FIFO* fifo;
#endif

q_path_set::q_path_set()
{
	lru = new LRUBuffer(QUERY::buffer_memory);
	//printf("buffer size: %u\n", QUERY::buffer_memory);
	//printf("page size:   %d\n", GLOBALS::blk_size);
	//printf("num pages:   %d\n", GLOBALS::num_parts);

	page_queue = new PageQueue(GLOBALS::num_parts);
#ifdef FIFO_QUEUE
	fifo = new FIFO();
#endif
	stacks = new std::unordered_map< unsigned int, std::stack< g_id >* >();

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

void q_path_set::print_stats()
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

q_path_set::~q_path_set()
{
#ifdef PRINT_METRICS
	delete page_stats;
#endif
#ifdef FIFO_QUEUE
	delete fifo;
#endif
	state.clear();
	label.clear();
	stacks->clear();
	delete stacks;
	delete page_queue;
	delete lru;
}

void q_path_set::start()
{
	int buf_size = lru->get_num_slots();
	int* page_dequeue = new int[buf_size]; // dequeue as many as fit in buffer

	char*   page;
	header1 header;
	int     page_id;
	g_id    first_gid;
	g_id    v_gid;
	int     v_rel;

	std::stack< g_id >* stack;
	std::unordered_map< unsigned int, std::stack< g_id >* >::iterator it;

#ifdef FIFO_QUEUE
	while (label.size() < (unsigned int) GLOBALS::num_vertices || !fifo->is_empty()) {
#else
	while (label.size() < (unsigned int) GLOBALS::num_vertices || !page_queue->is_empty()) {
#endif

		int count = 0;
#ifdef FIFO_QUEUE
		if (fifo->is_empty()) {
#else
		if (page_queue->is_empty()) {
#endif
			count = page_queue->next(page_dequeue, buf_size, true);
			//printf("Fill with empty pq: %d\n", count);
		} else {
#ifdef FIFO_QUEUE
			count = fifo->next(page_dequeue, buf_size, false);
#else
			count = page_queue->next(page_dequeue, buf_size, false);
#endif
#ifdef PRINT_METRICS
			page_reqs += count;
#endif
		}
#ifdef PRINT_METRICS
		// stats...
		for (int i = 0; i < count; ++i) {
			int j = page_dequeue[i];
			if (page_stats[j] == 1)
				++mult_pages;
			if (!lru->contains(j))
				++page_stats[j];
		}

		tot_reads += count;
#endif

		lru->fill(page_dequeue, count);

		/*
		 * SCAN
		 */
		for (int buf_slot = 0; buf_slot < buf_size; ++buf_slot) {

			page	  = lru->get_page_by_slot(buf_slot);
			page_id	  = get_block_id(blk_get_first_gid(page));
			first_gid = blk_get_first_gid(page);

			b_sc vertices_in_page = blk_get_max_slt(page);

			for (v_rel = 0; v_rel < vertices_in_page; ++v_rel) {
				v_gid = first_gid + v_rel;

				// if vertex undiscovered
				if (state.find(v_gid) == state.end()) {

					// if vertex unlabeled
					if (label.find(v_gid) == label.end()) {
						header = blk_get_header(page, v_rel);

						if (get_header(header) == 0)      // invalid header
							continue;
						evaluate_label(page, header, first_gid, v_rel, label, v_gid);
					}

					if (label[v_gid] == Source) {

						state[v_gid] = Discovered;
						
						if (stacks->find(page_id) == stacks->end()) {
							std::stack< g_id > *new_stack = new std::stack< g_id >;
							(*stacks)[page_id] = new_stack;
						}
						(*stacks)[page_id]->push(v_gid);

					} else if (label[v_gid] == Unfit) {
						state[v_gid] = Discovered;
					}
				}
			} // end scan page
		}

		/*
		 * TRAVERSE
		 */
		bool exploreBuffer;

#ifdef PRINT_METRICS		
		int* page_hits = new int[GLOBALS::num_parts];
		for (int i = 0; i < GLOBALS::num_parts; ++i) 
			page_hits[i] = 0;
#endif
		do {
			it = stacks->begin();
			exploreBuffer = false;

			while (it != stacks->end()) {

				page_id = it->first;
				stack = it->second;

				if (lru->contains(page_id) && !stack->empty()) {
#ifdef PRINT_METRICS
					if (page_hits[page_id] == 0) {
						++page_hits[page_id];
						++cache_hits;
					}
#endif

					while (!stack->empty()) {
						v_gid = stack->top(); stack->pop();

						v_rel = get_slot(v_gid);
						page = lru->get_page_by_pid(page_id);
						first_gid = blk_get_first_gid(page);

						// if vertex unlabeled
						if (label.find(v_gid) == label.end()) {
							header = blk_get_header(page, v_rel);
							evaluate_label(page, header, first_gid, v_rel, label, v_gid);
							if (label[v_gid] == Unfit) {
								state[v_gid] = Discovered;
								continue; // try popping next
							}
						}

						if (label[v_gid] == Destination) {
							register_finished();
							printf("Reachability found to vertex with GID %u\n", v_gid);
#ifdef PRINT_METRICS
							print_stats();
#endif
							//page_id = get_block_id(v_gid);
							//page = lru->get_page_by_pid(page_id);
							//blk_print(page);
							//push_on_output("Reachability found.\n");
							//if (QUERY::must_close_output)
							//	print_ln("Reachability found.");
							return;
						}

						header = blk_get_header(page, v_rel);
						// for cycling through internal/external edge lists
						char* str1 = page + get_header(header + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off;
						// end of internal vertex list
						char* ie_list_end = page + get_header(header + GLOBALS::ee_in_h);
						// end of external vertex list
						char* ee_list_end = page + get_header(header + GLOBALS::ee_end_in_h);

						// for each internal vertex
						while (str1 < ie_list_end) {
							v_rel = get_ie_adv(str1);
							v_gid = first_gid + v_rel;

							// if vertex undiscovered
							if (state.find(v_gid) == state.end()) {

								state[v_gid] = Discovered;

								if (label.find(v_gid) == label.end()) {
									header = blk_get_header(page, v_rel);
									evaluate_label(page, header, first_gid, v_rel, label, v_gid);
								}

								if (label[v_gid] == Destination) {
									register_finished();
									printf("Reachability found to vertex with GID %u\n", v_gid);
#ifdef PRINT_METRICS
									print_stats();
#endif
									//page_id = get_block_id(v_gid);
									//page = lru->get_page_by_pid(page_id);
									//blk_print(page);
									//push_on_output("Reachability found.\n");
									//if (QUERY::must_close_output)
									//	print_ln("Reachability found.");
									return;
								} else if (label[v_gid] != Unfit) {

									stack->push(v_gid);
								}
							}
						}
						str1 = ie_list_end;

						// for each external vertex
						while (str1 < ee_list_end) {

							v_gid = get_ee_adv(str1);
							v_rel = get_slot(v_gid);

							// if vertex undiscovered
							if (state.find(v_gid) == state.end()) {

								state[v_gid] = Discovered;

								int page_id_2 = get_block_id(v_gid); // pageof	

								// if label_w = nil AND y in buffer
								if (lru->contains(page_id_2) && label.find(v_gid) == label.end()) {
#ifdef PRINT_METRICS
									if (page_hits[page_id_2] == 0) {
										++page_hits[page_id_2];
										++cache_hits;
									}
#endif
									page = lru->get_page_by_pid(page_id_2);
									first_gid = blk_get_first_gid(page);
									header = blk_get_header(page, v_rel);
									evaluate_label(page, header, first_gid, v_rel, label, v_gid);
								}

								if (label.find(v_gid) != label.end() && label[v_gid] == Destination) {
									register_finished();
									printf("Reachability found to vertex with GID %u\n", v_gid);
#ifdef PRINT_METRICS
									print_stats();
#endif
									//page_id = get_block_id(v_gid);
									//if (!lru->contains(page_id)) {
									//	lru->fill(&page_id, 1);
									//}
									//page = lru->get_page_by_pid(page_id);
									//blk_print(page);
									//push_on_output("Reachability found.\n");
									//if (QUERY::must_close_output)
									//	print_ln("Reachability found.");
									return;
								} else if (label.find(v_gid) == label.end() || label[v_gid] != Unfit) {

									if (stacks->find(page_id_2) == stacks->end()) {
										std::stack< g_id >* new_stack = new std::stack< g_id >;
										(*stacks)[page_id_2] = new_stack;
									}
									(*stacks)[page_id_2]->push(v_gid);

									if (!lru->contains(page_id_2))
#ifdef FIFO_QUEUE
										fifo->insert(page_id_2);
#else
										page_queue->insert(page_id_2);
#endif

								}
							}
						}
					}
					exploreBuffer = true;
				}

				if (it->second->empty()) {
#ifdef FIFO_QUEUE
					fifo->erase(it->first);
#else
					page_queue->erase(it->first);
#endif
					it = stacks->erase(it);
				} else {
					++it;
				}
			}
		} while (exploreBuffer);
#ifdef PRINT_METRICS
		delete page_hits;
#endif
	}

	stack = NULL;
	delete page_dequeue;

	register_finished();
#ifdef PRINT_METRICS
	print_stats();
#endif
	push_on_output("No path exists.\n");
	if (QUERY::must_close_output)
		print_ln("No path exists.");
	return;
}
