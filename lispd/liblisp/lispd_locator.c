/*
 * lispd_locator.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Send registration messages for each database mapping to
 * configured map-servers.
 *
 * Copyright (C) 2011 Cisco Systems, Inc, 2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    Albert Lopez      <alopez@ac.upc.edu>
 */

//#include "lispd_afi.h"
#include "lispd_lib.h"
#include "lispd_locator.h"
#include "lispd_log.h"


inline lcl_locator_extended_info *new_lcl_locator_extended_info(int *out_socket);
inline rmt_locator_extended_info *new_rmt_locator_extended_info();
inline void free_lcl_locator_extended_info(lcl_locator_extended_info *extended_info);
inline void free_rmt_locator_extended_info(rmt_locator_extended_info *extended_info);

/*
 * Generets a locator element
 */

locator_t   *new_local_locator (
        lisp_addr_t                 *locator_addr,
        uint8_t                     *state,    /* UP , DOWN */
        uint8_t                     priority,
        uint8_t                     weight,
        uint8_t                     mpriority,
        uint8_t                     mweight,
        int                         *out_socket)
{
    locator_t       *locator                = NULL;

    if ((locator = malloc(sizeof(locator_t))) == NULL) {
        lmlog(LWRN, "new_local_locator: Unable to allocate memory for lispd_locator_elt: %s", strerror(errno));
        return(NULL);
    }

    /* Initialize locator */
    locator->addr = locator_addr;
    locator->type = LOCAL_LOCATOR;
    locator->priority = priority;
    locator->weight = weight;
    locator->mpriority = mpriority;
    locator->mweight = mweight;
    locator->data_packets_in = 0;
    locator->data_packets_out = 0;
    locator->state = state;

    locator->extended_info = (void *)new_lcl_locator_extended_info(out_socket);
    if (locator->extended_info == NULL){
        free (locator);
        return (NULL);
    }


    return (locator);
}


/*
 * Generets a "remote" locator element. Remote locators should reserve memory for address and state.
 * Afi information (address) is read from the packet (afi_ptr)
 */

locator_t   *new_rmt_locator (
        uint8_t                     **afi_ptr,
        uint8_t                     state,    /* UP , DOWN */
        uint8_t                     priority,
        uint8_t                     weight,
        uint8_t                     mpriority,
        uint8_t                     mweight)
{
    locator_t       *locator                = NULL;
    int                     len                     = 0;

    if ((locator = malloc(sizeof(locator_t))) == NULL) {
        lmlog(LWRN, "new_rmt_locator: Unable to allocate memory for lispd_locator_elt: %s", strerror(errno));
        return(NULL);
    }

    if((locator->addr = lisp_addr_new()) == NULL){
        lmlog(LWRN,"new_rmt_locator: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
        free (locator);
        return (NULL);
    }

    if((locator->state = malloc(sizeof(uint8_t))) == NULL){
        lmlog(LWRN,"new_rmt_locator: Unable to allocate memory for uint8_t: %s", strerror(errno));
        lisp_addr_del(locator->addr);
        free (locator);
        return (NULL);
    }

    if ((len = lisp_addr_parse(*afi_ptr, locator->addr)) <= 0)
        return(NULL);
    *afi_ptr = CO(*afi_ptr, len);

    locator->extended_info = (void *)new_rmt_locator_extended_info();
    if (locator->extended_info == NULL){
        lisp_addr_del(locator->addr);
        free (locator->state);
        free (locator);
        return (NULL);
    }

    *(locator->state) = state;
    locator->type = DYNAMIC_LOCATOR;
    locator->priority = priority;
    locator->weight = weight;
    locator->mpriority = mpriority;
    locator->mweight = mweight;
    locator->data_packets_in = 0;
    locator->data_packets_out = 0;

    return (locator);
}

locator_t   *new_static_rmt_locator (
        lisp_addr_t                 *rloc_addr,
        uint8_t                     state,    /* UP , DOWN */
        uint8_t                     priority,
        uint8_t                     weight,
        uint8_t                     mpriority,
        uint8_t                     mweight)
{
    locator_t       *locator                = NULL;

    if ((locator = malloc(sizeof(locator_t))) == NULL) {
        lmlog(LWRN, "new_static_rmt_locator: Unable to allocate memory for lispd_locator_elt: %s", strerror(errno));
        return(NULL);
    }

//    if((locator->locator_addr = lisp_addr_new()) == NULL){
//        lispd_log_msg(LWRN,"new_static_rmt_locator: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
//        free (locator);
//        return (NULL);
//    }

    if((locator->state = malloc(sizeof(uint8_t))) == NULL){
        lmlog(LWRN,"new_static_rmt_locator: Unable to allocate memory for uint8_t: %s", strerror(errno));
        free (locator->addr);
        free (locator);
        return (NULL);
    }

    locator->extended_info = (void *)new_rmt_locator_extended_info();
    if (locator->extended_info == NULL){
        lisp_addr_del(locator->addr);
        free (locator->state);
        free (locator);
        return (NULL);
    }

    locator->addr = lisp_addr_clone(rloc_addr);
    *(locator->state) = state;
    locator->type = STATIC_LOCATOR;
    locator->priority = priority;
    locator->weight = weight;
    locator->mpriority = mpriority;
    locator->mweight = mweight;
    locator->data_packets_in = 0;
    locator->data_packets_out = 0;

    return (locator);
}

inline lcl_locator_extended_info *new_lcl_locator_extended_info(int *out_socket)
{
    lcl_locator_extended_info *lcl_loc_ext_inf;
    if ((lcl_loc_ext_inf = (lcl_locator_extended_info *)malloc(sizeof(lcl_locator_extended_info))) == NULL) {
        lmlog(LWRN, "lcl_locator_extended_info: Unable to allocate memory for rmt_locator_extended_info: %s", strerror(errno));
        return(NULL);
    }
    lcl_loc_ext_inf->out_socket = out_socket;
    lcl_loc_ext_inf->rtr_locators_list = NULL;

    return lcl_loc_ext_inf;
}


inline rmt_locator_extended_info *new_rmt_locator_extended_info()
{
    rmt_locator_extended_info *rmt_loc_ext_inf;
    if ((rmt_loc_ext_inf = (rmt_locator_extended_info *)malloc(sizeof(rmt_locator_extended_info))) == NULL) {
        lmlog(LWRN, "new_rmt_locator_extended_info: Unable to allocate memory for rmt_locator_extended_info: %s", strerror(errno));
        return(NULL);
    }
    rmt_loc_ext_inf->rloc_probing_nonces = NULL;
    rmt_loc_ext_inf->probe_timer = NULL;

    return rmt_loc_ext_inf;
}


lispd_rtr_locator *new_rtr_locator(lisp_addr_t address)
{
    lispd_rtr_locator       *rtr_locator            = NULL;

    rtr_locator = (lispd_rtr_locator *)malloc(sizeof(lispd_rtr_locator));
    if (rtr_locator == NULL){
        lmlog(LWRN, "new_rtr_locator: Unable to allocate memory for lispd_rtr_locator: %s", strerror(errno));
        return (NULL);
    }
    rtr_locator->address = address;
    rtr_locator->latency = 0;
    rtr_locator->state = UP;

    return (rtr_locator);
}

/*
 * Leave in the list, rtr with afi equal to the afi passed as a parameter
 */

void remove_rtr_locators_with_afi_different_to(lispd_rtr_locators_list **rtr_list, int afi)
{
    lispd_rtr_locators_list *rtr_list_elt           = *rtr_list;
    lispd_rtr_locators_list *prev_rtr_list_elt      = NULL;
    lispd_rtr_locators_list *aux_rtr_list_elt       = NULL;

    while (rtr_list_elt != NULL){
        if (rtr_list_elt->locator->address.afi == afi){
            if (prev_rtr_list_elt == NULL){
                prev_rtr_list_elt = rtr_list_elt;
                if(rtr_list_elt != *rtr_list){
                    *rtr_list = rtr_list_elt;
                }
            }else{
                prev_rtr_list_elt->next = rtr_list_elt;
                prev_rtr_list_elt = prev_rtr_list_elt->next;
            }
            rtr_list_elt = rtr_list_elt->next;
        }else{
            aux_rtr_list_elt = rtr_list_elt;
            rtr_list_elt = rtr_list_elt->next;
            free (aux_rtr_list_elt->locator);
            free (aux_rtr_list_elt);
        }
    }
    /* Put the next element of the last rtr_locators_list found with afi X to NULL*/
    if (prev_rtr_list_elt != NULL){
        prev_rtr_list_elt->next = NULL;
    }else{
        *rtr_list = NULL;
    }
}


/*
 * Free memory of lispd_locator.
 */

void locator_del(locator_t *locator) {
    if (!locator)
        return;
    lisp_addr_del(locator->addr);
    if (locator->type != LOCAL_LOCATOR) {
        if (locator->extended_info)
            free_rmt_locator_extended_info(
                    (rmt_locator_extended_info*) locator->extended_info);
        free(locator->state);
    } else {
        if (locator->extended_info)
            free_lcl_locator_extended_info(
                    (lcl_locator_extended_info *) locator->extended_info);
    }
    free(locator);
}

inline void free_lcl_locator_extended_info(lcl_locator_extended_info *extended_info)
{
    free_rtr_list(extended_info->rtr_locators_list);
    free (extended_info);
}

void free_rtr_list(lispd_rtr_locators_list *rtr_list_elt)
{
    lispd_rtr_locators_list *aux_rtr_list_elt   = NULL;

    while (rtr_list_elt != NULL){
        aux_rtr_list_elt = rtr_list_elt->next;
        free(rtr_list_elt->locator);
        free(rtr_list_elt);
        rtr_list_elt = aux_rtr_list_elt;
    }
}

inline void free_rmt_locator_extended_info(rmt_locator_extended_info *extended_info)
{
    if (extended_info->probe_timer != NULL){
        stop_timer(extended_info->probe_timer);
        extended_info->probe_timer = NULL;
    }
    if (extended_info->rloc_probing_nonces != NULL){
        free (extended_info->rloc_probing_nonces);
    }
    free (extended_info);
}

void dump_locator (
        locator_t   *locator,
        int                 log_level)
{
    char locator_str [2000];
    if (is_loggable(log_level)){
        sprintf(locator_str, "| %39s |", lisp_addr_to_char(locator->addr));
        sprintf(locator_str + strlen(locator_str), "  %5s ", locator->state ? "Up" : "Down");
        sprintf(locator_str + strlen(locator_str), "|     %3d/%-3d     |", locator->priority, locator->weight);
        lmlog(log_level,"%s",locator_str);
    }
}

/**********************************  LOCATORS LISTS FUNCTIONS ******************************************/

/*
 * Add a locator to a locators list
 */
int add_locator_to_list(locators_list_t **list, locator_t *loc) {
    locators_list_t *loc_list = NULL, *aux_llist_prev = NULL,
            *aux_llist_next = NULL;
    int cmp = 0;

    if ((loc_list = malloc(sizeof(locators_list_t))) == NULL) {
        lmlog(LWRN, "add_locator_to_list: Unable to allocate memory for "
                "lispd_locator_list: %s", strerror(errno));
        return (ERR_MALLOC);
    }

    loc_list->next = NULL;
    loc_list->locator = loc;

    if (loc->type == LOCAL_LOCATOR
        && lisp_addr_ip_afi(locator_addr(loc)) != AF_UNSPEC) {
        /* If it's a local initialized locator, we should store it in order*/

        if (*list == NULL) {
            *list = loc_list;
        } else {
            aux_llist_prev = NULL;
            aux_llist_next = *list;
            while (aux_llist_next != NULL) {
                cmp = lisp_addr_cmp(loc->addr,
                        aux_llist_next->locator->addr);
                if (cmp < 0) {
                    break;
                } else if (cmp == 0) {
                    lmlog(DBG_3,"add_locator_to_list: The locator %s already exists.",
                            get_char_from_lisp_addr_t(*(loc->addr)));
                    free(loc_list);
                    return (ERR_EXIST);
                }
                aux_llist_prev = aux_llist_next;
                aux_llist_next = aux_llist_next->next;
            }
            if (aux_llist_prev == NULL) {
                loc_list->next = aux_llist_next;
                *list = loc_list;
            } else {
                aux_llist_prev->next = loc_list;
                loc_list->next = aux_llist_next;
            }
        }
    } else { /* Remote locators and not initialized local locators */
        if (*list == NULL) {
            *list = loc_list;
        } else {
            aux_llist_prev = *list;
            while (aux_llist_prev->next != NULL) {
                aux_llist_prev = aux_llist_prev->next;
            }
            aux_llist_prev->next = loc_list;
        }
    }

    return (GOOD);
}


int add_rtr_locator_to_list(
        lispd_rtr_locators_list **rtr_list,
        lispd_rtr_locator       *rtr_locator)
{
    lispd_rtr_locators_list *rtr_locator_list_elt   = NULL;
    lispd_rtr_locators_list *rtr_locator_list       = *rtr_list;

    rtr_locator_list_elt = (lispd_rtr_locators_list *)malloc(sizeof(lispd_rtr_locators_list));
    if (rtr_locator_list_elt == NULL){
        lmlog(LWRN, "new_rtr_locator_list_elt: Unable to allocate memory for lispd_rtr_locators_list: %s", strerror(errno));
        return (BAD);
    }
    rtr_locator_list_elt->locator = rtr_locator;
    rtr_locator_list_elt->next = NULL;
    if (rtr_locator_list != NULL){
        while (rtr_locator_list->next != NULL){
            rtr_locator_list = rtr_locator_list->next;
        }
        rtr_locator_list->next = rtr_locator_list_elt;
    }else{
        *rtr_list = rtr_locator_list_elt;
    }

    return (GOOD);
}

/*
 * Extract the locator from a locators list that match with the address.
 * The locator is removed from the list
 */
locator_t *extract_locator_from_list(
        locators_list_t     **head_locator_list,
        lisp_addr_t             addr)
{
    locator_t       *locator                = NULL;
    locators_list_t     *locator_list           = NULL;
    locators_list_t     *prev_locator_list_elt  = NULL;

    locator_list = *head_locator_list;
    while (locator_list != NULL){
        if (compare_lisp_addr_t(locator_list->locator->addr,&addr)==0){
            locator = locator_list->locator;
            /* Extract the locator from the list */
            if (prev_locator_list_elt != NULL){
                prev_locator_list_elt->next = locator_list->next;
            }else{
                *head_locator_list = locator_list->next;
            }
            free (locator_list);
            break;
        }
        prev_locator_list_elt = locator_list;
        locator_list = locator_list->next;
    }
    return (locator);
}

/*
 * Return the locator from the list that contains the address passed as a parameter
 */

locator_t *get_locator_from_list(
        locators_list_t    *locator_list,
        lisp_addr_t             *addr)
{
    locator_t       *locator                = NULL;
    int                     cmp                     = 0;

    while (locator_list != NULL){
        cmp = lisp_addr_cmp(locator_list->locator->addr,addr);
        if (cmp == 0){
            locator = locator_list->locator;
            break;
        }else if (cmp == 1){
            break;
        }
        locator_list = locator_list->next;
    }
    return (locator);
}

/*
 * Free memory of lispd_locator_list.
 */

void locator_list_del(locators_list_t     *locator_list)
{
    locators_list_t  * aux_locator_list     = NULL;
    /*
     * Free the locators
     */
    while (locator_list)
    {
        aux_locator_list = locator_list->next;
        locator_del(locator_list->locator);
        free (locator_list);
        locator_list = aux_locator_list;
    }
}

void locator_list_free_container(locators_list_t *locator_list, uint8_t free_locators_flag) {
    locators_list_t  * aux_locator_list     = NULL;
    /*
     * Free the locators
     */
    while (locator_list){
        aux_locator_list = locator_list->next;
        if (free_locators_flag)
            locator_del(locator_list->locator);
        free(locator_list);
        locator_list = aux_locator_list;
    }
}

inline locator_t *locator_new() {
    return(calloc(1, sizeof(locator_t)));
}


char *locator_to_char(locator_t *locator)
{
    static char locator_str[5][2000];
    static int i;

    /* hack to allow more than one locator per line */
    i++; i = i%5;

    sprintf(locator_str[i], "%s, ", lisp_addr_to_char(locator->addr));
    sprintf(locator_str[i] + strlen(locator_str[i]), "%s, ", locator->state ? "Up" : "Down");
    sprintf(locator_str[i] + strlen(locator_str[i]), "%d/%-d", locator->priority, locator->weight);
    return(locator_str[i]);
}


locator_t *locator_init_from_field(locator_field *lf) {
    locator_t   *loc    = NULL;
    uint8_t             status  = UP;

    if(!(loc = locator_new()))
        return(NULL);

    /*
     * We only consider the reachable bit if the information comes from the owner of the locator (local)
     */
    if (locator_field_hdr(lf)->reachable == DOWN && locator_field_hdr(lf)->local == UP){
        status = DOWN;
    }

    loc->addr = lisp_addr_init_from_field(locator_field_addr(lf));
    if (!loc->addr)
        return(NULL);
    loc->extended_info = (void *)new_rmt_locator_extended_info();
    if (!loc->extended_info){
        free(loc);
        return(NULL);
    }

    loc->state = calloc(1, sizeof(uint8_t));
    *(loc->state) = status;
    loc->type = DYNAMIC_LOCATOR;
    loc->priority = locator_field_hdr(lf)->priority;
    loc->weight = locator_field_hdr(lf)->weight;
    loc->mpriority = locator_field_hdr(lf)->mpriority;
    loc->mweight = locator_field_hdr(lf)->mweight;
    loc->data_packets_in = 0;
    loc->data_packets_out = 0;

    return(loc);
}

int
locator_parse(uint8_t *ptr, locator_t *loc) {
    locator_hdr_t *hdr;
    lisp_addr_t *addr;
    uint8_t status  = UP;
    int len;

    hdr = ptr;
    if (!LOC_REACHABLE(hdr) && LOC_LOCAL(hdr)) {
        status = DOWN;
    }
    if (!loc->addr) {
        loc->addr = lisp_addr_new();
    }

    len = lisp_addr_parse(LOC_ADDR(hdr), loc->addr);
    if (len <=0) {
        return(BAD);
    }

    loc->state = malloc(1, sizeof(uint8_t));
    *(loc->state) = status;
    loc->type = DYNAMIC_LOCATOR;
    loc->priority = LOC_PRIORITY(hdr);
    loc->weight = LOC_WEIGHT(hdr);
    loc->mpriority = LOC_MPRIORITY(hdr);
    loc->mweight = LOC_MWEIGHT(hdr);

    /* TODO: should we remove these? */
    loc->data_packets_in = 0;
    loc->data_packets_out = 0;

    return(sizeof(locator_hdr_t) + len);
}



int locator_write_to_field(locator_t *locator, locator_field *lfield) {
    lcl_locator_extended_info   *lct_extended_info  = NULL;
    lisp_addr_t                 *itr_address        = NULL;

    /* If the locator is DOWN, set the priority to 255 -> Locator should not be used */
    locator_field_hdr(lfield)->priority     = (*(locator->state) == UP) ? locator->priority : UNUSED_RLOC_PRIORITY;
    locator_field_hdr(lfield)->weight       = locator->weight;
    locator_field_hdr(lfield)->mpriority    = locator->mpriority;
    locator_field_hdr(lfield)->mweight      = locator->mweight;
    locator_field_hdr(lfield)->local        = 1;
    locator_field_hdr(lfield)->reachable    = *(locator->state);

    lct_extended_info = (lcl_locator_extended_info *)(locator->extended_info);
    itr_address = (lct_extended_info->rtr_locators_list != NULL) ?
            &(lct_extended_info->rtr_locators_list->locator->address): locator->addr;

    if (lisp_addr_write_to_field(itr_address, locator_field_addr(lfield)) <= 0) {
        lmlog(DBG_3, "locator_write_to_field: copy_addr failed for locator: %s",
                lisp_addr_to_char(locator_addr(locator)));
        return(BAD);
    }
    return(GOOD);
}

int locator_get_size_in_field(locator_t *loc) {
    return(lisp_addr_get_size_in_field(locator_addr(loc))+sizeof(locator_hdr_t));
}

/*
 *  Compute the sum of the lengths of the locators
 *  so we can allocate  memory for the packet....
 */

int locator_list_get_size_in_field(locators_list_t *locators_list)
{
    int sum = 0;
    while (locators_list) {
        sum += locator_get_size_in_field(locators_list->locator);
        locators_list = locators_list->next;
    }
    return(sum);
}

int locator_cmp(locator_t *l1, locator_t *l2) {
    int ret = 0;
    if ((ret = lisp_addr_cmp(locator_addr(l1), locator_addr(l2))) != 0)
        return(1);
    if (l1->priority != l2->priority)   return(1);
    if (l1->weight != l2->weight)   return(1);
    if (l1->mpriority != l2->mpriority)   return(1);
    if (l1->mweight != l2->mweight)   return(1);
    return(0);
}

/* FC: ugly stuff! Need to unify locators properly support extended info .. */
locator_t *locator_clone_remote(locator_t *locator) {
    locator_t *copy = locator_new();
    copy->addr = lisp_addr_clone(locator->addr);
    copy->state = calloc(1, sizeof(uint8_t));
    *(copy->state) = *(locator->state);

    copy->type = locator->type;
    copy->priority = locator->priority;
    copy->weight = locator->weight;
    copy->mpriority = locator->mpriority;
    copy->mweight = locator->mweight;
    copy->data_packets_in = locator->data_packets_in;
    copy->data_packets_out = locator->data_packets_out;
    copy->extended_info = new_rmt_locator_extended_info();
    ((lcl_locator_extended_info *)copy->extended_info)->out_socket = ((lcl_locator_extended_info *)locator->extended_info)->out_socket;
    if (((lcl_locator_extended_info *)copy->extended_info)->rtr_locators_list) {
        lmlog(LWRN, "locator_clone_remote: clone of rtr locators list NOT IMPLEMENTED!");
    }
    return(copy);
}

locators_list_t *locators_list_clone_remote(locators_list_t *lst) {
    locators_list_t *copy   = NULL;
    locators_list_t *it     = NULL;
    copy = calloc(1, sizeof(locators_list_t));

    it = lst;
    while(it) {
        add_locator_to_list(&copy, locator_clone_remote(it->locator));
        it = it->next;
    }
    return(copy);
}

locator_t *locator_init_remote(lisp_addr_t *addr) {
    locator_t *locator = locator_new();
    locator->addr = addr;

    if((locator->state = malloc(sizeof(uint8_t))) == NULL){
        locator_del(locator);
        return(NULL);
    }

    locator->extended_info = (void *)new_rmt_locator_extended_info();
    if (!locator->extended_info){
        locator_del(locator);
        return (NULL);
    }

    locator->type = DYNAMIC_LOCATOR;

    return(locator);
}

locator_t *locator_init_remote_full(lisp_addr_t *addr, uint8_t state, uint8_t priority, uint8_t weight,
        uint8_t mpriority, uint8_t mweight) {
    locator_t *locator = locator_init_remote(addr);
    *(locator->state) = state;
    locator->priority = priority;
    locator->weight = weight;
    locator->mpriority = mpriority;
    locator->mweight = mweight;
    return(locator);
}

locator_t *locator_init_local(lisp_addr_t *addr) {
    locator_t *locator = locator_new();
    /* Initialize locator */
    locator->addr = addr;
    locator->type = LOCAL_LOCATOR;

    return(locator);
}

locator_t *locator_init_local_full(lisp_addr_t *addr, uint8_t *state, uint8_t priority, uint8_t weight,
        uint8_t mpriority, uint8_t mweight) {
    locator_t *locator = locator_init_local(addr);
    /* Initialize locator */
    locator->priority = priority;
    locator->weight = weight;
    locator->mpriority = mpriority;
    locator->mweight = mweight;
    locator->data_packets_in = 0;
    locator->data_packets_out = 0;
    locator->state = state;
    return(locator);
}


