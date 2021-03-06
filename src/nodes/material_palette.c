/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

/* $Id$ */

#include <profit/nodes/material_palette.h>
#include <profit/model.h>
#include <profit/state.h>
#include <profit/node.h>
#include <profit/nodeinfo.h>
#include <profit/debug.h>
#include <profit/util/mempool.h>
#include <profit/util/bfile.h>
#include <config.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/**************************************************************************/

static prf_nodeinfo_t prf_material_palette_info = {
    66, PRF_ANCILLARY | PRF_OBSOLETE,
    "Material Palette",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
}; /* struct prf_material_palette_info */

/**************************************************************************/


typedef struct prf_material_palette_data node_data;

#define NODE_DATA_PAD 0

/**************************************************************************/

static
bool_t
prf_material_palette_load_f(
    prf_node_t * node,
    prf_state_t * state,
    bfile_t * bfile )
{
    int pos, count;

    assert( node != NULL && state != NULL && bfile != NULL );

    node->opcode = bf_get_uint16_be( bfile );
    if ( node->opcode != prf_material_palette_info.opcode ) {
        prf_error( 9, "tried material palette load method for node of type %d.",
            node->opcode );
        bf_rewind( bfile, 2 );
        return FALSE;
    }

    node->length = bf_get_uint16_be( bfile );

    if ( node->length > 4 && node->data == NULL ) { /* node preallocated */
        assert( state->model != NULL );
        if ( state->model->mempool_id == 0 )
            node->data = (uint8_t *)malloc( node->length - 4 + NODE_DATA_PAD );
        else
            node->data = (uint8_t *)pool_malloc( state->model->mempool_id,
                node->length - 4 + NODE_DATA_PAD );
        if ( node->data == NULL ) {
            prf_error( 9, "memory allocation problem (returned NULL)" );
            bf_rewind( bfile, 4 );
            return FALSE;
        }
    }

    pos = 4;
    count = 0;
    do {
        int i;
        node_data * data;
        data = (node_data *) (node->data + pos - 4);

        if ( node->length < (pos + 4) ) break;
        data->ambient_red = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->ambient_green = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->ambient_blue = bf_get_float32_be( bfile ); pos += 4;

        if ( node->length < (pos + 4) ) break;
        data->diffuse_red = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->diffuse_green = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->diffuse_blue = bf_get_float32_be( bfile ); pos += 4;

        if ( node->length < (pos + 4) ) break;
        data->specular_red = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->specular_green = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->specular_blue = bf_get_float32_be( bfile ); pos += 4;

        if ( node->length < (pos + 4) ) break;
        data->emissive_red = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->emissive_green = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->emissive_blue = bf_get_float32_be( bfile ); pos += 4;

        if ( node->length < (pos + 4) ) break;
        data->shininess = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->alpha = bf_get_float32_be( bfile ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        data->flags = bf_get_uint32_be( bfile ); pos += 4;

        if ( node->length < (pos + 12) ) break;
        bf_read( bfile, (uint8_t *) data->name, 12 ); pos += 12;
        for ( i = 0; i < 28 && node->length >= (pos + 4); i++ ) {
            data->reserved1[i] = bf_get_uint32_be( bfile ); pos += 4;
        }
        count++;
    } while ( TRUE );

    if ( node->length > pos )
        pos += bf_read( bfile, node->data + pos - 4, node->length - pos );

    return TRUE;
} /* prf_material_palette_load_f() */

/**************************************************************************/

static
bool_t
prf_material_palette_save_f(
    prf_node_t * node,
    prf_state_t * state,
    bfile_t * bfile )
{
    int pos;
    assert( node != NULL && state != NULL && bfile != NULL );

    if ( node->opcode != prf_material_palette_info.opcode ) {
        prf_error( 9, "tried material palette save method on node of type %d.",
            node->opcode );
        return FALSE;
    }

    bf_put_uint16_be( bfile, node->opcode );
    bf_put_uint16_be( bfile, node->length );

    pos = 4;
    do {
        node_data * data;
        int i;
        data = (node_data *) (node->data + pos - 4);
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->ambient_red ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->ambient_green ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->ambient_blue ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->diffuse_red ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->diffuse_green ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->diffuse_blue ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->specular_red ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->specular_green ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->specular_blue ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->emissive_red ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->emissive_green ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->emissive_blue ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->shininess ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_float32_be( bfile, data->alpha ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_put_uint32_be( bfile, data->flags ); pos += 4;
        if ( node->length < (pos + 4) ) break;
        bf_write( bfile, (uint8_t *) data->name, 12 ); pos += 12;
        for ( i = 0; i < 28 && node->length >= (pos + 4); i++ ) {
            bf_put_uint32_be( bfile, data->reserved1[i] ); pos += 4;
        }
    } while ( TRUE );

    if ( node->length > pos )
        pos += bf_write( bfile, node->data + pos - 4 + NODE_DATA_PAD,
            node->length - pos );

    return TRUE;
} /* prf_material_palette_save_f() */
    
/**************************************************************************/

static
void
prf_material_palette_entry_f(
    prf_node_t * node,
    prf_state_t * state )
{
    assert( node != NULL && state != NULL );

    if ( node->opcode != prf_material_palette_info.opcode ) {
        prf_error( 9, "tried material palette entry method on node of type %d.",
            node->opcode );
        return;
    }

    if ( state->material_palette != NULL && state->material_palette != node ) {
        prf_error( 9,
            "material palette entry: already registered a material palette" );
        return;
    }

    state->material_palette = node;
} /* prf_material_palette_entry_f() */

/**************************************************************************/

void
prf_material_palette_init(
    void )
{
    prf_material_palette_info.load_f = prf_material_palette_load_f;
    prf_material_palette_info.save_f = prf_material_palette_save_f;
    prf_material_palette_info.entry_f = prf_material_palette_entry_f;
    prf_nodeinfo_set( &prf_material_palette_info );
} /* prf_material_palette_init() */

/**************************************************************************/
