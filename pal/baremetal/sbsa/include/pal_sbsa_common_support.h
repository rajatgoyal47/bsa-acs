/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version  2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#ifndef __PAL_SBSA_COMMON_SUPPORT_H_
#define __PAL_SBSA_COMMON_SUPPORT_H_


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/**
  @brief  Instance of system pmu info
**/
typedef struct {
    uint8_t  type;                /* The component that this PMU block is associated with*/
    uint64_t primary_instance;    /* Primary node instance, specific to the PMU type*/
    uint32_t secondary_instance;  /* Secondary node instance, specific to the PMU type*/
    uint8_t  dual_page_extension; /* Support of the dual-page mode*/
    uint64_t base0;               /* Base address of Page 0 of the PMU*/
    uint64_t base1;               /* Base address of Page 1 of the PMU,
                                   valid only if dual_page_extension is 1*/
    uint32_t coresight_compliant; /* node coresight compliant or not */
} PMU_INFO_BLOCK;

typedef struct {
    uint32_t  pmu_count;          /* Total number of PMU info blocks*/
    PMU_INFO_BLOCK  info[];     /* PMU info blocks for each PMU nodes*/
} PMU_INFO_TABLE;

/*
 * @brief Mpam Resource Node
 */
typedef struct {
    uint8_t    ris_index;
    uint8_t    locator_type;  /* Identifies location of this resource */
    uint64_t   descriptor1;   /* Primary acpi description of location */
    uint32_t   descriptor2;   /* Secondary acpi description of location */
} MPAM_RESOURCE_NODE;

/*
 * @brief Mpam MSC Node
 */

typedef struct {
    uint8_t     intrf_type;     /* type of interface to this MPAM MSC */
    uint32_t    identifier;    /* unique id to reference the node */
    uint64_t    msc_base_addr; /* base addr of mem-mapped reg space or PCC
                                  subspace ID based on interface type. */
    uint32_t    msc_addr_len;  /* MSC mem map size */
    uint32_t    of_intr;       /* Overflow interrupt GSIV in case of wired interrupts. */
    uint32_t    of_intr_flags; /* Overflow interrupt flags */
    uint32_t    err_intr;      /* Error interrupt GSIV in case of wired interrupts. */
    uint32_t    err_intr_flags;/* Error interrupt flags */
    uint32_t    max_nrdy;      /* max time in microseconds that MSC not ready
                                  after config change */
    uint32_t    rsrc_count;    /* number of resource nodes */
    MPAM_RESOURCE_NODE rsrc_node[];   /* Details of resource node */
} MPAM_MSC_NODE;

/*
 * @brief Mpam info table
 */

#define MPAM_NEXT_MSC(msc_entry) \
        (MPAM_MSC_NODE *)((uint8_t *)(&msc_entry->rsrc_node[0]) \
        + msc_entry->rsrc_count * sizeof(MPAM_RESOURCE_NODE))

typedef struct {
    uint32_t          msc_count;  /* Number of MSC node */
    MPAM_MSC_NODE   msc_node[]; /* Details of MSC node */
} MPAM_INFO_TABLE;

/**
  @brief  SRAT node type
**/

typedef enum {
  SRAT_NODE_MEM_AFF  = 0x01,
  SRAT_NODE_GICC_AFF = 0x03
} SRAT_NODE_TYPE_e;

/**
  @brief  SRAT GICC Affinity Structure
**/

typedef struct {
  uint32_t   prox_domain;      /* Proximity domain*/
  uint32_t   proc_uid;         /* ACPI Processor UID */
  uint32_t   flags;            /* Flags*/
  uint32_t   clk_domain;       /* Clock Domain*/
} SRAT_GICC_AFF_ENTRY;

/**
  @brief  SRAT Memory Affinity Structure
**/

typedef struct {
  uint32_t   prox_domain;     /* Proximity domain */
  uint32_t   flags;           /* flags */
  uint64_t   addr_base;       /* mem range address base */
  uint64_t   addr_len;        /* mem range address len */
} SRAT_MEM_AFF_ENTRY;

typedef union {
  SRAT_MEM_AFF_ENTRY mem_aff;
  SRAT_GICC_AFF_ENTRY gicc_aff;
} SRAT_NODE_INFO;

typedef struct {
  uint32_t node_type;         /* Node type*/
  SRAT_NODE_INFO node_data;
} SRAT_INFO_ENTRY;

/* SRAT info table to store memory ranges information */

typedef struct {
  uint32_t num_of_srat_entries;
  uint32_t num_of_mem_ranges;
  SRAT_INFO_ENTRY  srat_info[];
} SRAT_INFO_TABLE;

/* Cache info table structures and APIs */

#define CACHE_TYPE_SHARED  0x0
#define CACHE_TYPE_PRIVATE 0x1

/*only the fields and flags required by ACS are parsed from ACPI PPTT table*/
/*Cache flags indicate validity of cache info provided by PPTT Table*/
typedef struct {
  uint32_t size_property_valid;
  uint32_t cache_type_valid;
  uint32_t cache_id_valid;
} CACHE_FLAGS;

/* Since most of platform doesn't support cache id field (ACPI 6.4+), ACS uses PPTT offset as key
   to uniquely identify a cache, In future once platforms align with ACPI 6.4+ my_offset member
   might be removed from cache entry*/
typedef struct {
  CACHE_FLAGS flags;        /* Cache flags */
  uint32_t my_offset;         /* Cache PPTT structure offset */
  uint32_t next_level_index;  /* Index of next level cache entry in CACHE_INFO_TABLE */
  uint32_t size;              /* Size of the cache in bytes */
  uint32_t cache_id;          /* Unique, non-zero identifier for this cache */
  uint32_t is_private;        /* Field indicate whether cache is private */
  uint8_t  cache_type;        /* Cache type */
} CACHE_INFO_ENTRY;

typedef struct {
  uint32_t num_of_cache;  /* Total of number of cache info entries */
  CACHE_INFO_ENTRY cache_info[];  /* Array of cache info entries */
} CACHE_INFO_TABLE;

/* RAS Information */

typedef enum {
  NODE_TYPE_PE = 0x0,
  NODE_TYPE_MC = 0x1,
  NODE_TYPE_SMMU = 0x2,
  NODE_TYPE_VDR = 0x3,
  NODE_TYPE_GIC = 0x4,
  NODE_TYPE_LAST_ENTRY
} RAS_NODE_TYPE_e;

typedef struct {
  uint32_t  processor_id;
  uint32_t  resource_type;
  uint32_t  flags;
  uint64_t  affinity;
  uint64_t  res_specific_data;  /* Resource Specific Data */
} RAS_NODE_PE_DATA;

typedef struct {
  uint32_t  proximity_domain;
} RAS_NODE_MC_DATA;

typedef struct {
  uint32_t  intf_type;           /* Interface Type */
  uint32_t  flags;
  uint64_t  base_addr;
  uint32_t  start_rec_index;     /* Start Record Index */
  uint32_t  num_err_rec;
  uint64_t  err_rec_implement;
  uint64_t  err_status_reporting;
  uint64_t  addressing_mode;
} RAS_INTERFACE_INFO;

typedef struct {
  uint32_t  type;
  uint32_t  flag;
  uint32_t  gsiv;
  uint32_t  its_grp_id;
} RAS_INTERRUPT_INFO;

typedef union {
  RAS_NODE_PE_DATA    pe;
  RAS_NODE_MC_DATA    mc;
} RAS_NODE_DATA;

typedef struct {
  RAS_NODE_TYPE_e     type;             /* Node Type PE/GIC/SMMU */
  uint16_t            length;           /* Length of the Node */
  uint64_t            num_intr_entries; /* Number of Interrupt Entry */
  RAS_NODE_DATA       node_data;        /* Node Specific Data */
  RAS_INTERFACE_INFO  intf_info;        /* Node Interface Info */
  RAS_INTERRUPT_INFO  intr_info[2];     /* Node Interrupt Info */
} RAS_NODE_INFO;

typedef struct {
  uint32_t  num_nodes;      /* Number of total RAS Nodes */
  uint32_t  num_pe_node;    /* Number of PE RAS Nodes */
  uint32_t  num_mc_node;    /* Number of Memory Controller Nodes */
  RAS_NODE_INFO  node[];  /* Array of RAS nodes */
} RAS_INFO_TABLE;

typedef enum {
    ERR_UC = 0x1,         /* Uncorrectable Error */
    ERR_DE,               /* Deferred Error */
    ERR_CE,               /* Correctable Error */
    ERR_CRITICAL          /* Critical Error */
} RAS_ERROR_TYPE;

typedef struct {
   RAS_ERROR_TYPE ras_error_type;   /* Error Type */
   uint64_t error_pa;                 /* Error Phy Address */
   uint32_t rec_index;                /* Error Record Index */
   uint32_t node_index;               /* Error Node Index in Info table */
   uint8_t is_pfg_check;              /* Pseudo Fault Check or not */
} RAS_ERR_IN_t;

typedef struct {
   uint32_t intr_id;        /* Interrupt ID */
   uint32_t error_record;   /* Error Record Number */
} RAS_ERR_OUT_t;

void pal_ras_create_info_table(RAS_INFO_TABLE *ras_info_table);
uint32_t pal_ras_setup_error(RAS_ERR_IN_t in_param, RAS_ERR_OUT_t *out_param);
uint32_t pal_ras_inject_error(RAS_ERR_IN_t in_param, RAS_ERR_OUT_t *out_param);
uint32_t pal_ras_check_plat_poison_support(void);

typedef enum {
  RAS2_TYPE_MEMORY = 0   /* RAS2 memory feature type*/
} RAS2_FEAT_TYPE;

typedef struct {
  uint32_t  proximity_domain;      /* Proximity domain of the memory */
  uint32_t  patrol_scrub_support;  /* Patrol srub support flag */
} RAS2_MEM_INFO;

typedef union {
  RAS2_MEM_INFO mem_feat_info;   /* Memory feature specific info */
} RAS2_BLOCK_INFO;

typedef struct {
  RAS2_FEAT_TYPE type;                     /* RAS2 feature type*/
  RAS2_BLOCK_INFO block_info;     /* RAS2 block info */
} RAS2_BLOCK;

typedef struct {
  uint32_t num_all_block;        /* Number of RAS2 feature blocks */
  uint32_t num_of_mem_block;     /* Number of memory feature blocks */
  RAS2_BLOCK blocks[];
} RAS2_INFO_TABLE;

void pal_ras2_create_info_table(RAS2_INFO_TABLE *ras2_info_table);

/* HMAT info table structures and APIs*/

#define HMAT_MEM_HIERARCHY_MEMORY   0x00
#define HMAT_DATA_TYPE_ACCESS_BW    0x03
#define HMAT_DATA_TYPE_READ_BW      0x04
#define HMAT_DATA_TYPE_WRITE_BW     0x05
#define HMAT_BW_ENTRY_UNREACHABLE   0xFFFF
#define HMAT_BASE_UNIT_48BIT        0xFFFFFFFFFFFFULL
typedef struct {
  uint32_t mem_prox_domain;             /* Proximity domain of the memory region*/
  uint64_t write_bw;                    /* Maximum write bandwidth */
  uint64_t read_bw;                     /* Maximum read bandwidth */
} HMAT_BW_ENTRY;

typedef struct {
  uint32_t num_of_mem_prox_domain;      /* Number of Memory Proximity Domains */
  HMAT_BW_ENTRY bw_info[];            /* Array of bandwidth info based on proximity domain */
} HMAT_INFO_TABLE;

void pal_hmat_create_info_table(HMAT_INFO_TABLE *HmatTable);

/* Platform Communication Channel (PCC) info table */
#ifndef GAS_STRUCT
#define GAS_STRUCT
typedef struct {
  uint8_t   addr_space_id;
  uint8_t   reg_bit_width;
  uint8_t   reg_bit_offset;
  uint8_t   access_size;
  uint64_t  addr;
} GENERIC_ADDRESS_STRUCTURE;
#endif

typedef struct {
  uint64_t                         base_addr;               /* base addr of shared mem-region */
  GENERIC_ADDRESS_STRUCTURE        doorbell_reg;            /* doorbell register */
  uint64_t                         doorbell_preserve;       /* doorbell register preserve mask */
  uint64_t                         doorbell_write;          /* doorbell register set mask */
  uint32_t                         min_req_turnaround_usec; /* minimum request turnaround time */
  GENERIC_ADDRESS_STRUCTURE        cmd_complete_chk_reg;    /* command complete check register */
  uint64_t                         cmd_complete_chk_mask;   /* command complete check mask */
  GENERIC_ADDRESS_STRUCTURE        cmd_complete_update_reg; /* command complete update register */
  uint64_t                         cmd_complete_update_preserve;
                                                            /* command complete update preserve */
  uint64_t                         cmd_complete_update_set; /* command complete update set mask */
} PCC_SUBSPACE_TYPE_3;

typedef union {
  PCC_SUBSPACE_TYPE_3 pcc_ss_type_3;
} PCC_TYPE_SPECIFIC_INFO;

typedef struct {
  uint32_t                 subspace_idx;    /* PCC subspace index in PCCT ACPI table */
  uint32_t                 subspace_type;   /* type of PCC subspace */
  PCC_TYPE_SPECIFIC_INFO   type_spec_info;  /* PCC subspace type specific info */
} PCC_INFO;

typedef struct {
  uint32_t  subspace_cnt; /* number of PCC subspace info stored */
  PCC_INFO  pcc_info[];   /* array of PCC info blocks */
} PCC_INFO_TABLE;


typedef struct {
  uint32_t reserved : 4;        /* Bits [31:28] Reserved must be zero */
  uint32_t token : 10;          /* Bits [27:18] Token Caller-defined value */
  uint32_t protocol_id : 8;     /* Bits [17:10] Protocol ID */
  uint32_t message_type : 2;    /* Bits [09:08] Message Type */
  uint32_t message_id : 8;      /* Bits [07:00] Message ID */
} SCMI_PROTOCOL_MESSAGE_HEADER;

typedef struct {
  uint32_t msc_id;            /* Identifier of the MSC */
  uint32_t flags;             /* Reserved, must be zero */
  uint32_t offset;            /* MPAM register offset to read from */
} PCC_MPAM_MSC_READ_CMD_PARA;

typedef struct {
  int32_t  status;             /* command response status code */
  uint32_t val;                /* value read from the register */
} PCC_MPAM_MSC_READ_RESP_PARA;

typedef struct {
  uint32_t msc_id;            /* Identifier of the MSC */
  uint32_t flags;             /* Reserved, must be zero */
  uint32_t val;               /* value to be written to the register */
  uint32_t offset;            /* MPAM register offset to write */
} PCC_MPAM_MSC_WRITE_CMD_PARA;

typedef struct {
  int32_t  status;             /* command response status code */
} PCC_MPAM_MSC_WRITE_RESP_PARA;

#define MPAM_FB_PROTOCOL_ID    0x1A
#define MPAM_MSG_TYPE_CMD      0x0
#define MPAM_MSC_READ_CMD_ID   0x4
#define MPAM_MSC_WRITE_CMD_ID  0x5
#define MPAM_PCC_CMD_SUCCESS   0x0
#define MPAM_PCC_SAFE_RETURN   0x0
#define RETURN_FAILURE         0xFFFFFFFF
#define PCC_TY3_CMD_OFFSET     12
#define PCC_TY3_COMM_SPACE     16
#define PCCT_SUBSPACE_TYPE_3_EXTENDED_PCC 0x03

void pal_pcc_create_info_table(PCC_INFO_TABLE *PccInfoTable);

#endif
