#include "query.h"

void evaluate_label(block1 p, header1 h, g_id first_gid, int v_rel, 
		std::unordered_map< unsigned int, Label > &label, g_id v_gid);

q_path::q_path()
{
	lru = new LRUBuffer(QUERY::buffer_memory);
	//printf("buffer size: %u\n", QUERY::buffer_memory);
	//printf("page size:   %d\n", GLOBALS::blk_size);
	// one slot per page
	page_queue = new PageQueue(GLOBALS::num_parts);
	for (int page = 0; page < GLOBALS::num_parts; ++page) {
		page_queue->insert(page);
	}
}

q_path::~q_path()
{
	state.clear();
	label.clear();
	stacks.clear();
	delete page_queue;
	delete lru;
}

void q_path::start()
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

	std::stack< g_id >* stack;
	std::unordered_map< unsigned int, std::stack< g_id >* >::iterator it;

	while (!page_queue->is_empty()) {
		// Fill(buffer, DequeueList(page_queue, buffer.size))
		bool status = page_queue->next(page_dequeue, buf_size);
		//assert (status == true);
		lru->fill(page_dequeue, buf_size);
		//printf("Filled buffer.\n");

		// for each page in buffer...
		for (int buf_slot = 0; buf_slot < buf_size; ++buf_slot) {
			page	  = lru->get_page_by_slot(buf_slot);
			page_id	  = get_block_id(blk_get_first_gid(page));
			first_gid = blk_get_first_gid(page);

			b_sc vertices_in_page = blk_get_max_slt(page);

			// for each vertex in page... v_rel = v relative to first_gid
			for (v_rel = 0; v_rel < vertices_in_page; ++v_rel) {
				v_gid = first_gid + v_rel;

				// if vertex undiscovered
				if (state.find(v_gid) == state.end()) {
					// if vertex unlabeled
					if (label.find(v_gid) == label.end()) {
						//printf("evaluating v_gid...\n");
						header = blk_get_header(page, v_rel);
						evaluate_label(page, header, first_gid, v_rel, 
								label, v_gid);

					}
					// assert (label.find(v_gid) != label.end());
					if (label[v_gid] == Source) {
						state[v_gid] = Discovered;
						if (stacks.find(page_id) == stacks.end()) {
							std::stack< g_id > *new_stack = 
									new std::stack< g_id >;
							stacks[page_id] = new_stack;
						}
						// assert (stacks.find(page_id) != stacks.end());
						stack = stacks[page_id];
						stack->push(v_gid);
						//printf("Pushed v %u from p %d\n", v_gid, page_id);
						//printf("stack.size() for %d: %d\n", page_id, stack->size());
						//printf("stacks.size(): %d\n", stacks.size());
					}
				}
				do {
					it = stacks.begin();
					while (it != stacks.end()) {
						//printf("stacks.size(): %d\n", stacks.size());
						page_id = it->first;
						stack = it->second;
						//printf("Stack %d exists.\n", page_id);
						//printf("stack.size() for %d: %d\n", page_id, stack->size());
						if (lru->contains(page_id) && !stack->empty()) {
							//printf("stacks.size(): %d\n", stacks.size());
							break;
						}
						++it;
					}
					if (it == stacks.end())
						break;
					//printf("traversing page %d\n", it->first);

					while (!stack->empty()) {
						v_gid = stack->top(); stack->pop();
						//printf("exploring vertex %u\n", v_gid);
						//blk_print(page);
						//if (stack->empty()) {
						//	delete stack;
						//	stacks.erase(page_id);
						//}
						v_rel = get_slot(v_gid);
						page = lru->get_page_by_pid(page_id);
						first_gid = blk_get_first_gid(page);

						// if vertex unlabeled
						if (label.find(v_gid) == label.end()) {
							header = blk_get_header(page, v_rel);
							evaluate_label(page, header, first_gid, 
									v_rel, label, v_gid);
							if (label[v_gid] == Unfit)
								continue; // try popping next
						}

						if (label[v_gid] == Destination) {
							register_finished();
							printf("Reachability found to vertex with GID %u\n", v_gid);
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
							v_rel = get_ie_adv(str1);
							v_gid = first_gid + v_rel;

							// if vertex undiscovered
							if (state.find(v_gid) == state.end()) {
								state[v_gid] = Discovered;

								if (label.find(v_gid) == label.end()) {
									header = blk_get_header(page, v_rel);
									evaluate_label(page, header, first_gid, 
											v_rel, label, v_gid);
								}
								if (label[v_gid] == Destination) {
									register_finished();
									printf("Reachability found to vertex with GID %u\n", v_gid);
									//page_id = get_block_id(v_gid);
									//page = lru->get_page_by_pid(page_id);
									//blk_print(page);
									//push_on_output("Reachability found.\n");
									//if (QUERY::must_close_output)
									//	print_ln("Reachability found.");
									return;
								} else if (label[v_gid] != Unfit) {
									if (stacks.find(page_id) == stacks.end()) {
										std::stack< g_id >* new_stack = 
												new std::stack< g_id >;
										stacks[page_id] = new_stack;
									}
									stack = stacks[page_id];
									stack->push(v_gid);
									//printf("Pushed v %u from p %d\n", v_gid, page_id);
								}
							}
						}
						str1 = ie_list_end;

						// for each external vertex
						while (str1 < ee_list_end) {
							//printf("exploring external edges...\n");
							v_gid = get_ee_adv(str1);
							v_rel = get_slot(v_gid);

							// if vertex undiscovered
							if (state.find(v_gid) == state.end()) {
								state[v_gid] = Discovered;

								page_id = get_block_id(v_gid); // pageof
								// if label_w = nil AND y in buffer
								if (lru->contains(page_id) 
										&& label.find(v_gid) == label.end()) {
									page = lru->get_page_by_pid(page_id);
									first_gid = blk_get_first_gid(page);
									header = blk_get_header(page, v_rel);
									evaluate_label(page, header, first_gid, 
											v_rel, label, v_gid);
								}
								if (label.find(v_gid) != label.end() 
										&& label[v_gid] == Destination) {
									register_finished();
									printf("Reachability found to vertex with GID %u\n", v_gid);
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
								} else if (label.find(v_gid) == label.end()
										|| label[v_gid] != Unfit) {
									if (stacks.find(page_id) == stacks.end()) {
										std::stack< g_id >* new_stack = 
												new std::stack< g_id >;
										stacks[page_id] = new_stack;
									}
									stack = stacks[page_id];
									stack->push(v_gid);
									//printf("Pushed v %u from p %d\n", v_gid, page_id);
									
									if (!lru->contains(page_id)) {
										page_queue->insert(page_id);
									}
								}
							}
						}
					}
					//++it;
				} while (true);
				//} while (it != stacks.end());
			}
		}
	}

	//delete header;
	delete page_dequeue;

	push_on_output("No path exists.\n");
	if (QUERY::must_close_output)
		print_ln("No path exists.");
	return;
}

void evaluate_label(block1 p, header1 h, g_id first_gid, int v_rel, 
		std::unordered_map< unsigned int, q_path::Label > &label, g_id v_gid) {
	if (QUERY::pred_tree_start_with->evaluate(p, h, first_gid | v_rel, 
				1, 0, false)) {
		label[v_gid] = q_path::Source;
		//printf("set Source\n");
	} else if (QUERY::pred_tree_end_with->evaluate(p, h, first_gid | v_rel)) {
		label[v_gid] = q_path::Destination;
		//printf("set Destination\n");
	} else if (QUERY::pred_tree_through->evaluate(p, h, first_gid | v_rel)) {
		label[v_gid] = q_path::Through;
		//printf("set Through\n");
	} else {
		label[v_gid] = q_path::Unfit;
		//printf("set Unfit\n");
	}
}