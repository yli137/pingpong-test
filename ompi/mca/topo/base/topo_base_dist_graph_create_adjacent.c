/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2008      The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2009      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2011-2013 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2011-2013 Inria.  All rights reserved.
 * Copyright (c) 2011-2013 Université Bordeaux 1
 * Copyright (c) 2014-2015 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2017      IBM Corp.  All rights reserved.
 * Copyright (c) 2018      Triad National Security, LLC. All rights
 *                         reserved.
 */

#include "ompi_config.h"

#include "ompi/communicator/communicator.h"
#include "ompi/info/info.h"
#include "ompi/mca/topo/base/base.h"


static int _mca_topo_base_dist_graph_create_adjacent (mca_topo_base_module_t* module, int indegree,
                                                      const int sources[], const int sourceweights[],
                                                      int outdegree, const int destinations[],
                                                      const int destweights[], int reorder,
                                                      ompi_communicator_t **newcomm)
{
    mca_topo_base_comm_dist_graph_2_2_0_t *topo = NULL;
    int err;

    err = OMPI_ERR_OUT_OF_RESOURCE;  /* suppose by default something bad will happens */

    assert( NULL == (*newcomm)->c_topo );

    topo = OBJ_NEW(mca_topo_base_comm_dist_graph_2_2_0_t);
    if (NULL == topo) {
        ompi_comm_free (newcomm);
        return OMPI_ERR_OUT_OF_RESOURCE;
    }
    topo->in = topo->inw = NULL;
    topo->out = topo->outw = NULL;
    topo->indegree = indegree;
    topo->outdegree = outdegree;
    topo->weighted = !((MPI_UNWEIGHTED == sourceweights) && (MPI_UNWEIGHTED == destweights));

    if (topo->indegree > 0) {
        topo->in = (int*)malloc(sizeof(int) * topo->indegree);
        if (NULL == topo->in) {
            goto bail_out;
        }
        memcpy(topo->in, sources, sizeof(int) * topo->indegree);
        if (MPI_UNWEIGHTED != sourceweights) {
            topo->inw = (int*)malloc(sizeof(int) * topo->indegree);
            if( NULL == topo->inw ) {
                goto bail_out;
            }
            memcpy( topo->inw, sourceweights, sizeof(int) * topo->indegree );
        }
    }

    if (topo->outdegree > 0) {
        topo->out = (int*)malloc(sizeof(int) * topo->outdegree);
        if (NULL == topo->out) {
            goto bail_out;
        }
        memcpy(topo->out, destinations, sizeof(int) * topo->outdegree);
        topo->outw = NULL;
        if (MPI_UNWEIGHTED != destweights) {
            if (topo->outdegree > 0) {
                topo->outw = (int*)malloc(sizeof(int) * topo->outdegree);
                if (NULL == topo->outw) {
                    goto bail_out;
                }
                memcpy(topo->outw, destweights, sizeof(int) * topo->outdegree);
            }
        }
    }

    (*newcomm)->c_topo                 = module;
    (*newcomm)->c_topo->mtc.dist_graph = topo;
    (*newcomm)->c_topo->reorder        = reorder;
    (*newcomm)->c_flags               |= OMPI_COMM_DIST_GRAPH;

    return OMPI_SUCCESS;

 bail_out:
    if (NULL != topo) {
        OBJ_RELEASE(topo);
    }

    ompi_comm_free(newcomm);
    return err;
}

int mca_topo_base_dist_graph_create_adjacent(mca_topo_base_module_t* module,
                                             ompi_communicator_t *comm_old,
                                             int indegree, const int sources[],
                                             const int sourceweights[],
                                             int outdegree,
                                             const int destinations[],
                                             const int destweights[],
                                             opal_info_t *info, int reorder,
                                             ompi_communicator_t **newcomm)
{
    int err;

    if (OMPI_SUCCESS != (err = ompi_comm_dup_with_info (comm_old, info, newcomm))) {
        return err;
    }

    return _mca_topo_base_dist_graph_create_adjacent (module, indegree, sources, sourceweights, outdegree,
                                                      destinations, destweights, reorder, newcomm);
}
