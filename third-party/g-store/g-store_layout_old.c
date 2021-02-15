#include "g-store_layout.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

void refine(graph1* graph, int* v_per_p, int* v_per_p_begin)
{
    static int max_p = 256;
    static int max_v = 256;
    static int pq_size = max_p * max_v / 10;

    int num_v      = graph->num_v;
    int* offset_e  = graph->offset_e;
    int* v_w       = graph->v_w;
    int* e_to      = graph->e_to;
    int* e_w       = graph->e_w;
    int* map_in_c  = graph->map_in_c;
    int* part      = graph->part;
    int  num_p     = graph->num_p;
    int  c_level   = graph->c_level;

    int* p_w = graph->p_w;

    int* idx_v             = malloc1<int>(max_v, "10001", 1);
    int* idx_p             = malloc1<int>(max_p, "10002", 1);
    int* cnt_ps            = malloc1<int>(max_p, "10003", 1);
    void* pq_space         = malloc_b((max_v + 1) * sizeof(int) + max_v * sizeof(idx_p_sc), "10004", 1);
    int* perm              = malloc1<int>(num_p, "10004", 1);
    int* part_map          = malloc1_set<int>(num_p, -1, "10005", 1);

    int* a_cnt_l              = malloc1<int>(max_v, "10006", 1);
    int* a_cnt_r              = malloc1<int>(max_v, "10007", 1);
    int** a_plist             = malloc1<int*>(max_v + 1, "10008", 1);
    int* a_plist_             = malloc1<int>((max_v + 1)* (max_p + 1), "10009", 1);
    idx_scd_w_pair** a_poswsc = malloc1<idx_scd_w_pair*>(max_v, "10010", 1);
    idx_scd_w_pair* a_poswsc_ = malloc1<idx_scd_w_pair>(max_v * max_p, "10011", 1);
    int* a_iw                 = malloc1<int>(max_v, "10012", 1);

    std::vector<std::vector<bool> > p_mat;
    p_mat.resize(num_p, std::vector<bool>(num_p));
    std::vector<bool>* p_mat_p;

    priority_queue2 pq;
    pq.init(pq_space, max_v);

    int8 threshold_w = get_threshold(c_level), alpha_sc;
    int give_up_threshold = (int) (PARAM::give_up_page_multiple * GLOBALS::blk_writable);
    int i, j, k, m, n, p, r, runs, best_p, best_v, best_v_idx, best_p_idx, beta_sc, gamma_sc;
    int cnt_v, cnt_p, cnt_isles, tmp, tmp_sc_p, tmp_p, tmp_p1, tmp_v;
    double best_sc, tmp_sc;
    int* found;
    idx_scd_w_pair* a_poswsc_v;
    int* a_plist_v;

    int sc_sm_m = 1;  //for c_levels >= 8
    int sc_lg_m = 8;
    int8 max_p_w;

    if (c_level < 8)
    {
        runs = PARAM::runs_b;
        sc_sm_m += 2 * (8 - c_level);
        sc_lg_m += 64 * (int) pow(2.0, 8 - c_level);
        max_p_w = threshold_w * 8; //* c_level + 1;
    }
    else
    {
        runs = PARAM::runs_a;
        max_p_w = threshold_w * 8;
    }

    //runs = 1;   //TBD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    max_p_w = threshold_w;

    for (i = 0; i < num_v; i++)
        p_w[part[i]] += v_w[i];      //stored in graph

    //prep-work
    for (i = 0; i < max_v; i++)
    {
        a_plist[i] = a_plist_ + (max_p + 1) * i;
        a_poswsc_v = a_poswsc[i] = a_poswsc_ + max_p * i;
        for (j = 0; j < max_p; j++)
            a_poswsc_v[j].idx = -1;
        a_iw[i] = 0;
        a_cnt_l[i] = 0;
        a_cnt_r[i] = max_p;
    }

    for (i = 0; i < num_p; i++)
    {
        p_mat_p = &p_mat[i];
        for (j = 0; j < num_p; j++)
            (*p_mat_p)[j] = false;
    }

    for (i = 0; i < num_p; i++)
    {
        if ((j = v_per_p_begin[i]) == -1)
            continue;

        p_mat_p = &p_mat[i];

        do
        {
            //assert(part[j] == i);       //todo: delete!
            for (k = offset_e[j]; k < offset_e[j+1]; k++)
                (*p_mat_p)[part[e_to[k]]] = true;
        } while ((j = v_per_p[j]) != -1);
    }

    //start
    for (r = 0; r < runs; r++)
    {
        for (i = 0; i < num_p; i++)
            perm[i] = i;
        permute(perm, num_p);

        for (i = 0; i < num_p; i++)
        {
            p = perm[i];

            if ((j = v_per_p_begin[p]) == -1) //empty
                continue;

            cnt_v = cnt_p = 0;

            do
            {
                idx_v[cnt_v++] = j;

                if (cnt_v == max_v)
                {
                    max_v += 256;
                    idx_v      = realloc1<int>(idx_v, max_v, "10013", 1);
                    free_all(8, pq_space, a_cnt_l, a_cnt_r, a_plist, a_plist_, a_poswsc, a_poswsc_, a_iw);
                    pq_space   = malloc_b((max_v + 1) * sizeof(int) + max_v * sizeof(idx_p_sc), "10014", 1);
                    a_cnt_l    = malloc1<int>(max_v, "10015", 1);
                    a_cnt_r    = malloc1<int>(max_v, "10016", 1);
                    a_plist    = malloc1<int*>(max_v + 1, "10017", 1);
                    a_plist_   = malloc1<int>((max_v + 1)* (max_p + 1), "10018", 1);
                    a_poswsc   = malloc1<idx_scd_w_pair*>(max_v, "10019", 1);
                    a_poswsc_  = malloc1<idx_scd_w_pair>(max_v * max_p, "10020", 1);
                    a_iw       = malloc1<int>(max_v, "10021", 1);
                    pq.init(pq_space, max_v);
                    for (k = 0; k < max_v; k++)
                    {
                        a_plist[k] = a_plist_ + (max_p + 1) * k;
                        a_poswsc_v = a_poswsc[k] = a_poswsc_ + max_p * k;
                        for (m = 0; m < max_p; m++)
                            a_poswsc_v[m].idx = -1;
                        a_iw[k] = 0;
                        a_cnt_l[k] = 0;
                        a_cnt_r[k] = max_p;
                    }
                }

                for (k = offset_e[j]; k < offset_e[j+1]; k++)
                {
                    tmp = part[e_to[k]];
                    if (tmp != p && part_map[tmp] == -1)
                    {
                        idx_p[cnt_p] = tmp;
                        part_map[tmp] = cnt_p++;
                        if (cnt_p == max_p)
                        {
                            max_p += 256;
                            idx_p = realloc1<int>(idx_p, max_p, "10022", 1);
                            free_all(4, cnt_ps, part_map, a_plist_, a_poswsc_);
                            cnt_ps      = malloc1<int>(max_p, "10023", 1);
                            part_map    = malloc1_set<int>(num_p, -1, "10024", 1);
                            a_plist_    = malloc1<int>((max_v + 1)* (max_p+1), "10025", 1);
                            a_poswsc_   = malloc1<idx_scd_w_pair>(max_v * max_p, "10026", 1);
                            for (m = 0; m < max_v; m++)
                            {
                                a_plist[m] = a_plist_ + (max_p + 1) * m;
                                a_poswsc[m] = a_poswsc_ + max_p * m;
                                a_poswsc_v = a_poswsc[m];
                                for (n = 0; n < max_p; n++)
                                    a_poswsc_v[n].idx = -1;
                                a_cnt_r[m] = max_p;
                            }
                        }
                    }
                }
            } while((j = v_per_p[j]) != -1);

            if (cnt_p == 0)
                continue;

            qsort(idx_p, cnt_p, sizeof(int), bs_comp_int<int>);    // (idx_v already sorted)

            for (j = 0; j < cnt_p; j++)
            {
                part_map[idx_p[j]] = j;
                cnt_ps[j] = 0;
            }

            for (j = 0; j < cnt_v; j++)
            {
                for (k = offset_e[idx_v[j]]; k < offset_e[idx_v[j]+1]; k++)
                    if (part[e_to[k]] != p && cnt_ps[part_map[part[e_to[k]]]] >= 0)
                        cnt_ps[part_map[part[e_to[k]]]] = -cnt_ps[part_map[part[e_to[k]]]] - 1;

                for (k = 0; k < cnt_p; k++)
                    if (cnt_ps[k] < 0)
                        cnt_ps[k] = -cnt_ps[k];
            }

            for (j = 0; j < cnt_v; j++)
            {
                //create a_plist, a_poswsc
                a_poswsc_v = a_poswsc[j];
                a_plist_v = a_plist[j];
                for (k = offset_e[idx_v[j]]; k < offset_e[idx_v[j]+1]; k++)
                {
                    tmp_p = part[e_to[k]];
                    if (tmp_p == p)
                        a_iw[j] += e_w[k];
                    else
                    {
                        tmp = part_map[tmp_p];
                        if (a_poswsc_v[tmp].idx == -1)
                        {
                            a_poswsc_v[tmp].w = e_w[k];
                            a_poswsc_v[tmp].idx = a_cnt_l[j];
                            a_plist_v[a_cnt_l[j]] = tmp;
                            a_cnt_l[j]++;
                        }
                        else
                            a_poswsc_v[tmp].w += e_w[k];
                    }
                }

                k = 0;
                tmp_sc_p = -1;
                tmp_sc = MININT;
                while(k < a_cnt_l[j])
                {
                    tmp_p = a_plist_v[k];
                    if (p_w[idx_p[tmp_p]] + v_w[idx_v[j]] > max_p_w)
                    {
                        a_plist_v[a_cnt_r[j]] = tmp_p;
                        a_poswsc_v[tmp_p].idx = a_cnt_r[j];
                        a_poswsc_v[tmp_p].sc = -1.0;
                        a_cnt_r[j]--;
                        a_cnt_l[j]--;
                        if (k == a_cnt_l[j])
                            break; //else:

                        tmp_p1 = a_plist_v[a_cnt_l[j]];
                        a_plist_v[k] = tmp_p1;
                        a_poswsc_v[tmp_p1].idx = k;
                    }
                    else
                    {
                        alpha_sc = std::abs(p - idx_p[tmp_p]) * -a_iw[j];  //p is not in plist, a_poswsc_v[tmp_p].w considered below
                        beta_sc = a_poswsc_v[tmp_p].w - a_iw[j];
                        for (m = 0, gamma_sc = 0; m <= max_p; m++)
                        {
                            if (m == a_cnt_l[j]) { m = a_cnt_r[j] + 1; if (m > max_p) break; }

                            tmp_p1 = a_plist_v[m];
                            alpha_sc += a_poswsc_v[tmp_p1].w * (std::abs(p - idx_p[tmp_p1]) - std::abs(idx_p[tmp_p] - idx_p[tmp_p1]));

                            if (cnt_ps[tmp_p1] == 1)
                                gamma_sc += 1;
                            if (tmp_p1 != tmp_p && !p_mat[idx_p[tmp_p]][idx_p[tmp_p1]])
                                gamma_sc -= 1;
                        }

                        if (r == 0 && (alpha_sc < 0 || beta_sc < 0 || gamma_sc < 0))
                        {
                            a_plist_v[a_cnt_r[j]] = tmp_p;
                            a_poswsc_v[tmp_p].idx = a_cnt_r[j];
                            a_poswsc_v[tmp_p].sc = -1.0;
                            a_cnt_r[j]--;
                            a_cnt_l[j]--;
                            if (k == a_cnt_l[j])
                                break; //else:

                            tmp_p1 = a_plist_v[a_cnt_l[j]];
                            a_plist_v[k] = tmp_p1;
                            a_poswsc_v[tmp_p1].idx = k;
                        }
                        else
                        {
                            a_poswsc_v[tmp_p].sc = PARAM::alpha * alpha_sc + PARAM::beta * beta_sc + PARAM::gamma * gamma_sc +
                                (p_w[idx_p[tmp_p]] < GLOBALS::blk_writable ? sc_sm_m : - sc_lg_m * (int) (p_w[idx_p[tmp_p]] / threshold_w));
                            //consider: p_w + v_w

                            if (a_poswsc_v[tmp_p].sc > tmp_sc)
                            {
                                tmp_sc = a_poswsc_v[tmp_p].sc;
                                tmp_sc_p = tmp_p;
                            }
                            k++;
                        }
                    }
                }
                if (tmp_sc_p != -1)
                    pq.insert(j, tmp_sc_p, tmp_sc);
                else
                    pq.insert_r(j);
            } //next vertex (next j)

            while (!pq.empty())
            {
                pq.pop_max(best_v_idx, best_p_idx, best_sc);

                assert(part[idx_v[best_v_idx]] == p);

                best_p = idx_p[best_p_idx];
                best_v = idx_v[best_v_idx];
                a_poswsc_v = a_poswsc[best_v_idx];
                a_plist_v = a_plist[best_v_idx];

                if (best_sc <= (double) (p_w[p] <= give_up_threshold ? - sc_sm_m : (p_w[p] <= GLOBALS::blk_size ? sc_sm_m : 0)))
                {
                    //clear
                    for (j = 0; j <= max_p; j++)
                    {
                        if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }
                        a_poswsc_v[a_plist_v[j]].idx = -1;
                    }
                    a_cnt_l[best_v_idx] = 0;
                    a_cnt_r[best_v_idx] = max_p;
                    a_iw[best_v_idx] = 0;
                    break;
                }

                if (p_w[best_p] + v_w[best_v] > max_p_w)
                {
                    tmp = a_poswsc_v[best_p_idx].idx;              //tmp = plist position before move
                    a_plist_v[a_cnt_r[best_v_idx]] = best_p_idx;
                    a_poswsc_v[best_p_idx].idx = a_cnt_r[best_v_idx];
                    a_poswsc_v[best_p_idx].sc = -1.0;
                    a_cnt_r[best_v_idx]--;
                    a_cnt_l[best_v_idx]--;
                    if (tmp != a_cnt_l[best_v_idx])
                    {
                        a_plist_v[tmp] = a_plist_v[a_cnt_l[best_v_idx]];
                        a_poswsc_v[a_plist_v[tmp]].idx = tmp;
                    }

                    tmp_sc = MININT;
                    tmp_sc_p = -1;
                    for (j = 0; j < a_cnt_l[best_v_idx]; j++)
                        if (a_poswsc_v[a_plist_v[j]].sc > tmp_sc)
                        {
                            tmp_sc = a_poswsc_v[a_plist_v[j]].sc;
                            tmp_sc_p = a_plist_v[j];
                        }
                    if (tmp_sc_p != -1)
                        pq.insert(best_v_idx, tmp_sc_p, tmp_sc);
                    else
                        pq.insert_r(best_v_idx);

                    continue;
                } //else:

                //print_ln("moving (%d) %d: %d -> %d (%4f; %d-%d->%d)",
                //  r, best_v, p, best_p, best_sc, p_w[p], v_w[best_v], p_w[best_p]);

                part[best_v] = best_p;
                p_w[p] -= v_w[best_v];
                p_w[best_p] += v_w[best_v];

                change_p_lists(best_v, p, best_p, v_per_p, v_per_p_begin);

                for (j = 0; j <= max_p; j++)
                {
                    if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }


                    tmp_p = a_plist_v[j];
                    if (tmp_p == best_p_idx)
                        continue;             //treated after internal

                    // Gamma, Case A - chance to clear link
                    if (--cnt_ps[tmp_p] == 1)
                    {
                        for (k = 0; k < cnt_v; k++)
                            if (a_poswsc[k][tmp_p].idx != -1 && a_poswsc[k][tmp_p].idx < a_cnt_l[k] && k != best_v_idx)
                            {
                                a_poswsc[k][tmp_p].sc += PARAM::gamma;
                                pq.change_if_better(k, tmp_p, a_poswsc[k][tmp_p].sc);
                                break;
                            }
                    }
                    else if (cnt_ps[tmp_p] == 0)
                        p_mat[p][idx_p[tmp_p]] = false;

                    // Gamma, Case B - update links from best_p, maybe take back penalty of move to best_p
                    if (!p_mat[best_p][idx_p[tmp_p]])
                    {
                        p_mat[best_p][idx_p[tmp_p]] = true;
                        for (k = 0; k < cnt_v; k++)
                            if (a_poswsc[k][best_p_idx].idx != -1 && a_poswsc[k][tmp_p].idx != -1 && a_poswsc[k][best_p_idx].idx < a_cnt_l[k] && k != best_v_idx)
                            {
                                a_poswsc[k][best_p_idx].sc += PARAM::gamma;
                                pq.change_if_better(k, best_p_idx, a_poswsc[k][best_p_idx].sc);
                            }
                    }

                    // Score
                    tmp = 0;
                    if (p_w[best_p] - v_w[best_v] < GLOBALS::blk_writable && p_w[best_p] > GLOBALS::blk_writable)
                        tmp = sc_sm_m;
                    else if ((int) ((p_w[best_p] - v_w[best_v]) / threshold_w) < (int)(p_w[best_p] / threshold_w))
                        tmp = sc_lg_m;

                    if (tmp != 0)
                    {
                        for (k = 0; k < cnt_v; k++)
                            if (a_poswsc[k][best_p_idx].idx != -1 && a_poswsc[k][best_p_idx].idx < a_cnt_l[k] && k != best_v_idx)
                            {
                                a_poswsc[k][best_p_idx].sc -= tmp;
                                if (pq.pos[k].p == best_p_idx)
                                {
                                    a_poswsc_v = a_poswsc[k];
                                    a_plist_v = a_plist[k];
                                    tmp_sc = MININT;
                                    tmp_sc_p = -1;
                                    for (m = 0; m < a_cnt_l[k]; m++)
                                        if (a_poswsc_v[a_plist_v[m]].sc > tmp_sc)
                                        {
                                            tmp_sc = a_poswsc_v[a_plist_v[m]].sc;
                                            tmp_sc_p = a_plist_v[m];
                                        }
                                    assert (tmp_sc_p != -1);
                                    pq.change(k, tmp_sc_p, tmp_sc);
                                }
                            }
                    }
                }

                cnt_ps[best_p_idx]--;

                // Alpha, Beta
                for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
                {
                    if (part[e_to[j]] == p)
                    {
                        cnt_ps[best_p_idx]++;

                        found = (int*) bsearch(&e_to[j], idx_v, cnt_v, sizeof(int), bs_comp_int<int>);
                        assert(found != NULL);
                        tmp_v = found - idx_v;
                        a_poswsc_v = a_poswsc[tmp_v];
                        a_plist_v = a_plist[tmp_v];

                        if (a_poswsc_v[best_p_idx].idx == -1)
                        {
                            a_poswsc_v[best_p_idx].w = 0;   //as if best_v was not transfered yet

                            if (p_w[best_p] + v_w[tmp_v] > max_p_w)
                            {
                                a_plist_v[a_cnt_r[tmp_v]] = best_p_idx;
                                a_poswsc_v[best_p_idx].idx = a_cnt_r[tmp_v];
                                a_poswsc_v[best_p_idx].sc = -1.0;
                                a_cnt_r[tmp_v]--;
                            }
                            else
                            {
                                alpha_sc = -std::abs(p - best_p) * a_iw[tmp_v];  //corrected later
                                beta_sc = -a_iw[tmp_v];

                                for (k = 0, gamma_sc = 0; k <= max_p; k++)
                                {
                                    if (k == a_cnt_l[tmp_v]) { k = a_cnt_r[tmp_v] + 1; if (k > max_p) break; }

                                    tmp_p = a_plist_v[k];
                                    alpha_sc += a_poswsc_v[tmp_p].w * (std::abs(p - idx_p[tmp_p]) - std::abs(best_p - idx_p[tmp_p]));

                                    if (best_p_idx != tmp_p)
                                    {
                                        if (cnt_ps[tmp_p] == 1)   //best_p == idx_p[tmp_p] treated below
                                            gamma_sc += 1;
                                        if (!p_mat[best_p][idx_p[tmp_p]])
                                            gamma_sc -= 1;
                                    }
                                }

                                if (r == 0 && (alpha_sc < 0 || beta_sc < 0 || gamma_sc < 0))
                                {
                                    a_plist_v[a_cnt_r[tmp_v]] = best_p_idx;
                                    a_poswsc_v[best_p_idx].idx = a_cnt_r[tmp_v];
                                    a_poswsc_v[best_p_idx].sc = -1.0;
                                    a_cnt_r[tmp_v]--;
                                }
                                else
                                {
                                    if (a_cnt_l[tmp_v] == 0)
                                    {
                                        assert(pq.cnt_elem_r < max_v+1);
                                        pq.remove_r(tmp_v);
                                    }

                                    a_plist_v[a_cnt_l[tmp_v]] = best_p_idx;
                                    a_poswsc_v[best_p_idx].idx = a_cnt_l[tmp_v];
                                    a_poswsc_v[best_p_idx].sc = PARAM::alpha * alpha_sc + PARAM::beta * beta_sc + PARAM::gamma * gamma_sc
                                        + (p_w[best_p] < GLOBALS::blk_writable ? sc_sm_m : - sc_lg_m * (int) (p_w[best_p] / threshold_w));
                                    a_cnt_l[tmp_v]++;

                                    if (a_cnt_l[tmp_v] == 1)
                                        pq.insert(tmp_v, best_p_idx, a_poswsc_v[best_p_idx].sc);

                                }
                            }
                        } //if new end

                        a_iw[tmp_v] -= e_w[j];
                        a_poswsc_v[best_p_idx].w += e_w[j];

                        tmp_sc_p = -1;
                        tmp_sc = MININT;

                        for (k = 0; k < a_cnt_l[tmp_v]; k++)      //adjust score of moving tmp_v to k
                        {
                            tmp_p = a_plist_v[k];
                            a_poswsc_v[tmp_p].sc += PARAM::alpha * std::abs(p - idx_p[tmp_p]) * e_w[tmp_v];
                            a_poswsc_v[tmp_p].sc += PARAM::beta * e_w[tmp_v];
                            // status now: as if edge had not existed

                            if (tmp_p == best_p_idx)
                                a_poswsc_v[tmp_p].sc +=  PARAM::beta * e_w[tmp_v];

                            a_poswsc_v[tmp_p].sc += PARAM::alpha * e_w[tmp_v] * (std::abs(p - best_p) - std::abs(idx_p[tmp_p] - best_p));

                            if (a_poswsc_v[tmp_p].sc > tmp_sc)
                            {
                                tmp_sc = a_poswsc_v[tmp_p].sc;
                                tmp_sc_p = tmp_p;
                            }
                        }
                        if (tmp_sc_p != -1)
                            pq.change(tmp_v, tmp_sc_p, tmp_sc);
                    }
                }

                // Gamma, Case C - only one link left to best_p_idx
                if (cnt_ps[best_p_idx] == 1)
                {
                    for (k = 0; k < cnt_v; k++)
                        if (a_poswsc[k][best_p_idx].idx != -1 && k != best_v_idx)
                        {
                            if (a_poswsc[k][best_p_idx].idx < a_cnt_l[k])
                            {
                                a_poswsc[k][best_p_idx].sc += PARAM::gamma;
                                pq.change_if_better(k, best_p_idx, a_poswsc[k][best_p_idx].sc);
                            }
                            break;
                        }
                }
                else if (cnt_ps[best_p_idx] == 0)
                    p_mat[p][best_p] = false;

                //clear
                a_poswsc_v = a_poswsc[best_v_idx];
                a_plist_v = a_plist[best_v_idx];
                for (j = 0; j <= max_p; j++)
                {
                    if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }
                    a_poswsc_v[a_plist_v[j]].idx = -1;
                }
                a_cnt_l[best_v_idx] = 0;
                a_cnt_r[best_v_idx] = max_p;
                a_iw[best_v_idx] = 0;

            } // end while !pq.empty


            for (j = 1; j <= max_v; j++)
            {
                if (j == pq.cnt_elem + 1) { j = pq.cnt_elem_r; if (j > max_v) break; }
                tmp_v = pq.pq[j];
                a_poswsc_v = a_poswsc[tmp_v];
                a_plist_v = a_plist[tmp_v];
                for (k = 0; k <= max_p; k++)
                {
                    if (k == a_cnt_l[tmp_v]) { k = a_cnt_r[tmp_v] + 1; if (k > max_p) break; }
                    a_poswsc_v[a_plist_v[k]].idx = -1;
                }
                a_cnt_l[tmp_v] = 0;
                a_cnt_r[tmp_v] = max_p;
                a_iw[tmp_v] = 0;
            }
            pq.reset();

            /*for (j = 0; j < max_v; j++)
              for (k = 0; k < max_p; k++)
              if (a_poswsc[j][k].idx != -1)
              print_endln();*/

            for (j = 0; j < cnt_p; j++)
                part_map[idx_p[j]] = -1;

        } // partition done!

        // might have to be moved inside loop to get a correct estimate of distance
        tmp = cnt_isles = n = 0;

        for (i = 0; i < num_p; i++)
        {
            if (v_per_p_begin[i] == -1)
            {
                assert(p_w[i] == 0);
                if (GLOBALS::isle_map[cnt_isles] == i)
                {
                    assert(n==0);
                    n = 1;
                }
                tmp++;
                continue;
            }

            if (i - tmp > 0 && p_w[i-tmp-1] + p_w[i] <= GLOBALS::blk_size && GLOBALS::isle_map[cnt_isles] != i && n == 0)
            {
                tmp++;
                j = v_per_p_begin[i-tmp];
                k = v_per_p_begin[i];

                if (k < j)
                {
                    v_per_p_begin[i-tmp] = m = k;
                    part[k] -= tmp;
                    k = v_per_p[k];
                }
                else
                {
                    m = j;
                    j = v_per_p[j];
                }

                for(;;)
                {
                    if (j == -1)
                    {
                        v_per_p[m] = k;
                        do
                            part[k] -= tmp;
                        while ((k = v_per_p[k]) != -1);
                        break;
                    }

                    if (k == -1)
                    {
                        v_per_p[m] = j;
                        break;
                    }

                    if (j < k)
                    {
                        v_per_p[m] = j;
                        m = j;
                        j = v_per_p[j];
                    }
                    else
                    {
                        v_per_p[m] = k;
                        m = k;
                        part[k] -= tmp;
                        k = v_per_p[k];
                    }
                }

                p_w[i-tmp] += p_w[i];
                continue;
            }

            if (tmp > 0)
            {
                if(GLOBALS::isle_map[cnt_isles] == i || n == 1)
                {
                    GLOBALS::isle_map[cnt_isles++] = i - tmp;
                    n = 0;
                }

                j = v_per_p_begin[i-tmp] = v_per_p_begin[i];

                do
                    part[j] -= tmp;
                while ((j = v_per_p[j]) != -1);

                p_w[i-tmp] = p_w[i];
            }

            if (GLOBALS::isle_map[cnt_isles] == i)
                cnt_isles++;
        }

        num_p -= tmp;
        graph->num_p = num_p;
        GLOBALS::isle_map [cnt_isles] = -1;
    }

    free_all(13, idx_v, idx_p, cnt_ps, pq_space, perm, part_map, a_cnt_l, a_cnt_r, a_plist, a_plist_, a_poswsc, a_poswsc_, a_iw);
}

//TODO: remove map in coarser early!
int uncoarsen(graph1 *graph)
{
    graph1* c_graph = graph->coarser;

    int num_v     = graph->num_v;
    int* map_in_c = graph->map_in_c;
    int* c__part  = c_graph->part;
    int c__num_p  = c_graph->num_p;

    int i, tmp, max_p1, num_p;
    int *part;

    int* v_per_p = malloc1_set<int>(num_v, -1,"todo12", 2);
    int* v_per_p_begin = malloc1<int>(c__num_p, "todo12", 2);
    int* v_per_p_end = malloc1_set<int>(c__num_p, -1,"todo12", 2);

    std::vector<bool>* part_type = new std::vector<bool>;

    for (i = 0; i < num_v; i++)
    {
        tmp = c__part[map_in_c[i]];

        if (v_per_p_end[tmp] == -1)
            v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
        else
        {
            v_per_p[v_per_p_end[tmp]] = i;
            v_per_p_end[tmp] = i;
        }
    }

    free(v_per_p_end);

    project(graph, v_per_p, v_per_p_begin, part_type, max_p1);

    free_all(8, c_graph->offset_e, c_graph->v_w, c_graph->map_in_c, c_graph->e_to,
            c_graph->e_w, c__part, c_graph->p_w, c_graph);

    graph->coarser = NULL;

    num_p = graph->num_p;
    part = graph->part;

    free(v_per_p_begin);

    arr_set<int>(v_per_p, -1, num_v);

    v_per_p_begin = malloc1<int>(num_p, "todo12", 1);
    v_per_p_end = malloc1_set<int>(num_p, -1,"todo12", 1);

    for (i = 0; i < num_v; i++)
    {
        tmp = part[i];
        if (v_per_p_end[tmp] == -1) {
            v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
        } else {
            v_per_p[v_per_p_end[tmp]] = i;
            v_per_p_end[tmp] = i;
        }
    }

    free(v_per_p_end);
    //not needed until refine, but required for calls of evaluate
    graph->p_w  = malloc1_set<int>(num_p, 0, "todo756");         // todo: might have to be int8?
    reorder(graph, v_per_p, v_per_p_begin, part_type, max_p1);
    delete part_type;
    refine(graph, v_per_p, v_per_p_begin);

    if (graph->c_level == 0) {
        finalize(graph, v_per_p, v_per_p_begin);
        free_all(2, v_per_p, v_per_p_begin);
        return 1;
    }

    free_all(2, v_per_p, v_per_p_begin);
    return 0;
}


void finalize(graph1* graph, int* &v_per_p, int* &v_per_p_begin)
{

    int num_v = graph->num_v;
    int num_e = graph->num_e;
    int num_p = graph->num_p;
    int* e_w = graph->e_w;
    int* v_w = graph->v_w;
    int* offset_e = graph->offset_e;
    int* e_to = graph->e_to;
    int* part = graph->part;
    int* p_w = graph->p_w;
    int* org_size = malloc1<int>(num_v,"todo");
    int* p_num_v = malloc1<int>(num_p,"todo");
    int* v_map = malloc1<int>(num_v, "todo");

    int i, j, k, m, n, tmp, tmp2, max_num_rec = 0;

    int my_p_w, l_p_w, r_p_w;
    int my_num_v, l_num_v, r_num_v;
    bool l_go, r_go, success;
    int new_my_p_w, new_lr_p_w, cnt_new_p, best_v;
    int cnt_v, new_p, p_add = 0;
    int ee_ie_diff;

    idx_sc_pair<int> max_to_l;
    idx_sc_pair<int> max_to_r;

    fread(org_size, sizeof(int), num_v, GLOBALS::fp_org_size);
    fclose(GLOBALS::fp_org_size);

    tmp = 0;
    for (i = 0; i < num_p; i++)
    {
        cnt_v = 0;

        j = v_per_p_begin[i];
        do
            cnt_v++;
        while ((j = v_per_p[j]) != -1);

        p_num_v[i] = cnt_v;
        if (cnt_v >= 256)
            tmp++;

        if (cnt_v > max_num_rec)
            max_num_rec = cnt_v;
    }

    // the parameters will be set based on the first run of vertices
    if (FIRST_RUN)
    {
        if (tmp > 0.005 * num_p)
        {
            GLOBALS::ie_size = 2;
            GLOBALS::blk_max_num_rec = UINT2_MAX + 1;
        }
        else
        {
            GLOBALS::ie_size = 1;
            GLOBALS::blk_max_num_rec = UINT1_MAX + 1;
        }

        GLOBALS::max_rec_p_page = GLOBALS::blk_max_num_rec;

        ee_ie_diff = EE_SIZE - GLOBALS::ie_size;

        GLOBALS::blk_writable = GLOBALS::blk_size - (4 + GLOBALS::header_slt_len + GLOBALS::ie_size * 2);
    }

    for (i = 0; i < num_p; i++)
    {
        tmp = 0;
        j = v_per_p_begin[i];

        do
        {
            tmp2 = 0;
            for (k = offset_e[j]; k < offset_e[j] + org_size[j]; k++)
                if (part[e_to[k]] == i)
                    tmp2 += ee_ie_diff;

            tmp += tmp2;
        } while ((j = v_per_p[j]) != -1);

        p_w[i] -= tmp;
    }


    int* scmv_ = malloc1<int>(5 * max_num_rec, "todo");

    int* move_lr = scmv_;
    int* idx_v = scmv_ + max_num_rec;
    int* sc_to_l = scmv_ + max_num_rec*2;
    int* sc_to_r = scmv_ + max_num_rec*3;
    int* chg_lr_p_w = scmv_ + max_num_rec*4;
    int* chg_my_p_w = sc_to_r;

    void* pq_space = malloc_b((sizeof(idx_sc_pair<int>) + sizeof(int)) * (max_num_rec + 1), "todo324");
    priority_queue1 pq(pq_space, max_num_rec);

    max_num_rec = 0;

    for (i = 0; i < num_p; i++)
    {
        if (p_w[i] <= GLOBALS::blk_writable && p_num_v[i] <= GLOBALS::max_rec_p_page)
        {
            if (p_num_v[i] > max_num_rec)
                max_num_rec = p_num_v[i];
            continue;
        }

        //need to act
        my_p_w = p_w[i];
        my_num_v = p_num_v[i];

        if (i == 0)
        {
            l_p_w = GLOBALS::blk_writable + 1;
            l_num_v = GLOBALS::max_rec_p_page + 1;
        }
        else
        {
            l_p_w = p_w[i-1];
            l_num_v = p_num_v[i-1];
        }

        if (i == num_p - 1)
        {
            r_p_w = GLOBALS::blk_writable + 1;
            r_num_v = GLOBALS::max_rec_p_page + 1;
        }
        else
        {
            r_p_w = p_w[i+1];
            r_num_v = p_num_v[i+1];
        }

        if (     (my_p_w <= GLOBALS::blk_writable || 3*GLOBALS::blk_writable - my_p_w - l_p_w - r_p_w >= 0)
                && (my_num_v <= GLOBALS::max_rec_p_page || 3*GLOBALS::max_rec_p_page - my_num_v - l_num_v - r_num_v >= 0) )
        {
            // potential to give vertices to neighbors
            arr_set(move_lr, 0, my_num_v);
            arr_set(sc_to_l, 0, my_num_v);
            arr_set(sc_to_r, 0, my_num_v);

            j = v_per_p_begin[i];
            k = 0;
            do
            {
                idx_v[k] = j;
                v_map[j] = k++;
            }
            while ((j = v_per_p[j]) != -1);

            cnt_v = p_num_v[i];

            for(;;)
            {
                max_to_l.set(-1, INT_MIN);
                max_to_r.set(-1, INT_MIN);

                for(j = 0; j < cnt_v; j++)
                {
                    if (move_lr[j] == 0)
                    {
                        for (m = offset_e[idx_v[j]]; m < offset_e[idx_v[j]+1]; m++)
                        {
                            tmp = part[e_to[m]];
                            if (tmp == i + p_add)
                            {
                                sc_to_l[j] -= e_w[m];
                                sc_to_r[j] -= e_w[m];
                            }
                            else if (tmp == i + p_add - 1)
                                sc_to_l[j] += e_w[m];
                            else if (tmp == i + p_add + 1)
                                sc_to_r[j] += e_w[m];
                        }

                        max_to_l.if_gr_set(j, sc_to_l[j]);
                        max_to_r.if_gr_set(j, sc_to_r[j]);
                    }
                }

                if (max_to_l.idx != -1 && l_p_w < GLOBALS::blk_writable && l_num_v < GLOBALS::max_rec_p_page)
                {
                    //potential to move to left (in general move left is preferable to move right, because not needed again)
                    best_v = idx_v[max_to_l.idx];
                    new_my_p_w = new_lr_p_w = v_w[best_v];

                    for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
                    {
                        if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] != -1)
                            new_my_p_w -= e_w[j] * ee_ie_diff;
                        else if (part[e_to[j]] == i + p_add - 1 || (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == -1))
                            new_lr_p_w -= e_w[j] * ee_ie_diff;
                    }

                    if (l_p_w + new_lr_p_w <= GLOBALS::blk_writable)
                    {
                        my_num_v--;
                        my_p_w -= new_my_p_w;
                        l_num_v++;
                        l_p_w += new_lr_p_w;
                        move_lr[max_to_l.idx] = -1;
                        chg_lr_p_w[max_to_l.idx] = new_lr_p_w;
                        l_go = true;
                    }
                    else
                        l_go = false;
                }
                else
                    l_go = false;

                if (my_p_w <= GLOBALS::blk_writable && my_num_v <= GLOBALS::max_rec_p_page)
                {
                    success = true;
                    break;
                }

                if (max_to_r.idx != -1 && r_p_w < GLOBALS::blk_writable
                        && r_num_v < GLOBALS::max_rec_p_page && move_lr[max_to_r.idx] == 0)
                {
                    best_v = idx_v[max_to_r.idx];
                    new_my_p_w = new_lr_p_w = v_w[best_v];

                    for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
                    {
                        if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] != 1)
                            new_my_p_w -= e_w[j] * ee_ie_diff;
                        else if (part[e_to[j]] == i + p_add + 1 || (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == 1))
                            new_lr_p_w -= e_w[j] * ee_ie_diff;
                    }

                    if (r_p_w + new_lr_p_w <= GLOBALS::blk_writable)
                    {
                        my_num_v--;
                        my_p_w -= new_my_p_w;
                        r_num_v++;
                        r_p_w += new_lr_p_w;
                        move_lr[max_to_r.idx] = 1;
                        chg_lr_p_w[max_to_r.idx] = new_lr_p_w;
                        r_go = true;
                    }
                    else
                        r_go = false;
                }
                else
                    r_go = false;

                if (my_p_w <= GLOBALS::blk_writable && my_num_v <= GLOBALS::max_rec_p_page)
                {
                    success = true;
                    break;
                }
                else if (!l_go && !r_go)
                {
                    success = false;
                    break;
                }
            }

            ///////////////////////////////////////////

            if (success)
            {
                for (j = 0; j < cnt_v; j++)
                    if (move_lr[j] != 0)
                    {
                        best_v = idx_v[j];
                        new_p = i + move_lr[j];
                        //do not need to change

                        part[best_v] = new_p + p_add;
                        p_num_v[new_p]++;
                        p_num_v[i]--;
                    }

                p_num_v[i] = my_num_v;
                p_w[i] = my_p_w;

                /*if (i != 0) // not needed
                  {
                  p_num_v[i-1] = l_num_v;
                  p_w[i-1] = l_p_w;
                  }*/
                if (i != num_p-1)
                {
                    p_num_v[i+1] = r_num_v;
                    p_w[i+1] = r_p_w;
                }

                if_gr_set(my_num_v, max_num_rec);
                if_gr_set(l_num_v, max_num_rec);
                if_gr_set(r_num_v, max_num_rec);
            }

        } //large if loop
        else
            success = false;

        //////////////////////////////////////////////////////////////////////////

        if (!success)
        {
            // otherwise: split partition
            j = v_per_p_begin[i];
            k = cnt_new_p = 0;
            my_p_w = p_w[i];
            my_num_v = cnt_v = p_num_v[i];
            max_to_l.set(-1, INT_MAX);
            arr_set(move_lr, -1, my_num_v);
            arr_set(sc_to_l, 0, my_num_v);

            do
            {
                assert(part[j] == i + p_add);

                tmp = 0;
                for (m = offset_e[j]; m < offset_e[j+1]; m++)
                    tmp += e_w[j] * (part[e_to[m]] - (i + p_add));

                max_to_l.if_sm_set(k, tmp);
                idx_v[k] = j;
                v_map[j] = k++;

            } while ((j = v_per_p[j]) != -1);

            best_v = idx_v[max_to_l.idx];
            move_lr[max_to_l.idx] = cnt_new_p;

            l_p_w = v_w[best_v];
            my_p_w -= v_w[best_v];
            l_num_v = 1;
            my_num_v -= 1;

            for (j = offset_e[best_v]; j < offset_e[best_v + 1]; j++)
                if (part[e_to[j]] == i + p_add)
                {
                    sc_to_l[v_map[e_to[j]]] += e_w[j];
                    my_p_w += ee_ie_diff * e_w[j];
                }


            for(;;)
            {
                pq.reset();

                for (j = 0; j < cnt_v; j++)
                    if (move_lr[j] == -1)
                    {
                        tmp = idx_v[j];
                        new_lr_p_w = new_my_p_w = v_w[tmp];

                        for (k = offset_e[tmp]; k < offset_e[tmp + 1]; k++)
                            if (part[e_to[k]] == i + p_add)
                                if (move_lr[v_map[e_to[k]]] == cnt_new_p)
                                    new_lr_p_w -= ee_ie_diff * e_w[k];
                                else if (move_lr[v_map[e_to[k]]] == -1)
                                    new_my_p_w -= ee_ie_diff * e_w[k];

                        chg_lr_p_w[j] = new_lr_p_w;
                        chg_my_p_w[j] = new_my_p_w;

                        pq.insert(j, sc_to_l[j]);
                    }

                if (pq.empty()) //todo: maybe stop with new_my_p_w==0
                    break;

                l_go = false;
                do
                {
                    max_to_l = pq.pop_max();

                    if (l_p_w + chg_lr_p_w[max_to_l.idx] <= GLOBALS::blk_writable)
                    {
                        l_go = true;
                        break;
                    }
                } while (!pq.empty());

                if (l_go)
                {
                    move_lr[max_to_l.idx] = cnt_new_p;
                    l_p_w += chg_lr_p_w[max_to_l.idx];
                    my_p_w -= chg_my_p_w[max_to_l.idx];
                    my_num_v--;
                    l_num_v++;

                    best_v = idx_v[max_to_l.idx];
                    for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
                        if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == -1)
                            sc_to_l[v_map[e_to[j]]] += e_w[j];

                    if (l_num_v >= GLOBALS::max_rec_p_page)
                    {
                        if_gr_set(l_num_v, max_num_rec);
                        cnt_new_p++;
                        l_num_v = 0;
                        l_p_w = 0;
                    }
                }
                else
                {
                    if_gr_set(l_num_v, max_num_rec);
                    cnt_new_p++;
                    l_num_v = 0;
                    l_p_w = 0;
                }
            }

            //assert (cnt_new_p >= 1);
            if (cnt_new_p == 0)
                print_endln();
            assert (my_p_w == 0);

            for (j = 0; j < num_v; j++)
                if (part[j] > i + p_add)
                    part[j] += cnt_new_p;

            for (j = 0; j < cnt_v; j++)
            {
                assert(move_lr[j] != -1);

                part[idx_v[j]] = i + p_add + move_lr[j];
                p_w[i] = l_p_w;         // from last run
                p_num_v[i] = l_num_v;
                if_gr_set(l_num_v, max_num_rec);
            }
            p_add += cnt_new_p;
        }
    } // for each p

    if (FIRST_RUN)
    {
        max_num_rec = int(max_num_rec * 1.05);

        if (max_num_rec == GLOBALS::blk_max_num_rec)
            GLOBALS::max_rec_p_page = GLOBALS::blk_max_num_rec;
        else
        {
            tmp = 0;
            while (max_num_rec >>= 1) ++tmp;
            tmp = 1 << tmp;
            if (tmp != max_num_rec)
                tmp <<= 1;
            GLOBALS::max_rec_p_page = tmp;
            if_sm_set(GLOBALS::blk_max_num_rec, GLOBALS::max_rec_p_page);
        }

        blk_init_fkts();
        FIRST_RUN = FALSE;
        GLOBALS::blk_var_start = 4 + GLOBALS::header_slt_len + 2* GLOBALS::ie_size;
    }

    num_p += p_add;
    graph->num_p = num_p;

    //required for printing statistics
    free_all(6, p_num_v, v_map, v_per_p, v_per_p_begin, scmv_, p_w);

    v_per_p = malloc1_set<int>(num_v, -1, "todo12");          //freed outside
    v_per_p_begin = malloc1<int>(num_p, "todo12");            //freed outside
    int* v_per_p_end = malloc1_set<int>(num_p, -1,"todo12");
    p_w = graph->p_w = malloc1_set<int>(num_p, 0,"todo12");   //part of graph - freed outside
    p_num_v = malloc1_set<int>(num_p, 0, "todo");

    for (i = 0; i < num_v; i++)
    {
        tmp = part[i];
        p_w[tmp] += v_w[i];
        p_num_v[tmp]++;

        if (v_per_p_end[tmp] == -1)
            v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
        else
        {
            v_per_p[v_per_p_end[tmp]] = i;
            v_per_p_end[tmp] = i;
        }
    }

    free(v_per_p_end);

    /*
       for (i = 0; i < num_p; i++)
       {
       tmp = 0;
       j = v_per_p_begin[i];

       do
       {
       tmp2 = 0;
       for (k = offset_e[j]; k < offset_e[j] + org_size[j]; k++)
       if (part[e_to[k]] == i)
       tmp2 += ee_ie_diff;

       tmp += tmp2;
       } while ((j = v_per_p[j]) != -1);

       p_w[i] -= tmp;
       }
       */
    //light version, if not printing statistics
    //  free_all(3, p_num_v, v_map, scmv_, p_w);
    //  graph->p_w = NULL;

    // Combine partitions next to each other
    new_p = 0;
    for (i = 0; i < num_p; i++)
    {
        l_p_w = p_w[i];
        for (j = i + 1; j < num_p; j++)
        {
            my_p_w = my_num_v = tmp = 0;
            for (k = i; k <= j; k++)
            {
                my_p_w += p_w[k];
                my_num_v += p_num_v[k];
                m = v_per_p_begin[k];
                do
                {
                    tmp2 = 0;
                    for (n = offset_e[m]; n < offset_e[m] + org_size[m]; n++)
                        if (part[e_to[n]] >= i && part[e_to[n]] <= j)
                            tmp2 += ee_ie_diff;

                    tmp += tmp2;
                } while ((m = v_per_p[m]) != -1);
            }

            if (my_p_w - tmp > GLOBALS::blk_writable || my_num_v > GLOBALS::max_rec_p_page)
                break;
            l_p_w = my_p_w - tmp;
        }
        combine_p_lists(i, j-1, new_p, v_per_p, v_per_p_begin);
        p_w[new_p] = l_p_w;

        for (k = i; k < j; k++)
        {
            m = v_per_p_begin[k];
            do
                part[m] = new_p;
            while ((m = v_per_p[m]) != -1);
        }
        new_p++;
        i = j - 1;
        if (new_p == 996)
            print_endln();
    }

    num_p = new_p;
    graph->num_p = num_p;

    free(p_num_v);
    v_per_p_begin = realloc1<int>(v_per_p_begin, num_p, "todo12", 1);
    p_w = graph->p_w = realloc1<int>(p_w, num_p, "todo12", 1);
}

