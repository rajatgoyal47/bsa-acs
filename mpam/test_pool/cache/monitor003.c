/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"
#include "val/common/include/acs_mpam.h"
#include "val/common/include/acs_mpam_reg.h"
#include "val/mpam/include/mpam_val_interface.h"


#define TEST_NUM   ACS_MPAM_CACHE_TEST_NUM_BASE + 6
#define TEST_RULE  ""
#define TEST_DESC  "Check PARTID Storage by CPOR Nodes    "

#define PARTITION_PERCENTAGE 75
#define CACHE_PERCENTAGE 50

static uint64_t mpam2_el2_temp;

static void payload(void)
{
    uint32_t msc_node_cnt = val_mpam_get_msc_count();
    uint32_t rsrc_node_cnt;
    uint32_t msc_index;
    uint32_t rsrc_index;
    uint32_t llc_index;
    uint64_t cache_identifier;
    uint32_t cache_maxsize;
    uint32_t cpor_nodes = 0;
    uint32_t csumon_count = 0;
    uint8_t  partid1;
    uint8_t  partid2;
    void *src_buf = 0;
    void *dest_buf = 0;
    uint64_t buf_size;
    uint64_t mpam2_el2 = 0;
    uint64_t nrdy_timeout;
    uint32_t storage_value1;
    uint32_t storage_value2;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint32_t test_partid = 9; //Selecting distinct partid

   /* Get the Index for LLC */
    llc_index = val_cache_get_llc_index();
    if (llc_index == CACHE_TABLE_EMPTY) {
        val_print(ACS_PRINT_ERR, "\n       Cache info table empty", 0);
        val_set_status(index, RESULT_SKIP(TEST_NUM, 01));
        return;
    }

    /* Get the cache identifier for LLC */
    cache_identifier = val_cache_get_info(CACHE_ID, llc_index);
    if (cache_identifier == INVALID_CACHE_INFO) {
        val_print(ACS_PRINT_ERR, "\n       LLC invalid in PPTT", 0);
        val_set_status(index, RESULT_SKIP(TEST_NUM, 02));
        return;
    }

    /* Get MPAM related information for LLC */
    for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {

      rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
      for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

        if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index) != MPAM_RSRC_TYPE_PE_CACHE)
          continue;

        if (val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index) != cache_identifier)
          continue;

        /* Select resource instance if RIS feature implemented */
        if (val_mpam_msc_supports_ris(msc_index))
          val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

        if (val_mpam_supports_cpor(msc_index)) {
          cache_maxsize = GET_MAX_VALUE(cache_maxsize,
                              val_cache_get_info(CACHE_SIZE, llc_index));

          if (val_mpam_supports_csumon(msc_index))
              csumon_count = val_mpam_get_csumon_count(msc_index);

          cpor_nodes++;
        }
      }
    }

    val_print(ACS_PRINT_DEBUG, "\n       CPOR Nodes = %d", cpor_nodes);
    val_print(ACS_PRINT_DEBUG, "\n       Cache Max Size = 0x%x", cache_maxsize);
    val_print(ACS_PRINT_DEBUG, "\n       Number of CSU Monitors = %d", csumon_count);

    /* Skip the test if CSU monitors/ nodes supporting Cache Portion Partitoning are 0
    */
    if (csumon_count == 0 || cpor_nodes == 0) {
        val_set_status(index, RESULT_SKIP(TEST_NUM, 03));
        return;
    }

    /* Configure CPOR settings for nodes supporting CPOR */
    for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {

      rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
      for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

        if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index) != MPAM_RSRC_TYPE_PE_CACHE)
          continue;

        if (val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index) != cache_identifier)
          continue;

        /* Select resource instance if RIS feature implemented */
        if (val_mpam_msc_supports_ris(msc_index))
          val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

        if (val_mpam_supports_cpor(msc_index))
          val_mpam_configure_cpor(msc_index, test_partid, PARTITION_PERCENTAGE);

      }
    }

    /* Create two PARTID groups for PE traffic */
    partid1 = test_partid;
    partid2 = test_partid + 1;

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);
    mpam2_el2_temp = mpam2_el2;

    /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PMG_D_SHIFT+7, MPAMn_ELx_PMG_D_SHIFT);

    /* Write partid1 & DEFAULT_PMG to mpam2_el2 to generate PE traffic */
    mpam2_el2 |= (((uint64_t)DEFAULT_PMG << MPAMn_ELx_PMG_D_SHIFT) |
                  ((uint64_t)partid1 << MPAMn_ELx_PARTID_D_SHIFT));

    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

     /* visit each MSC node and check for Cache resources */
    for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {
      rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);

      for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

        /* Select resource instance if RIS feature implemented */
        if (val_mpam_msc_supports_ris(msc_index))
          val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

        /* Check whether resource node is a PE Cache */
        if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index) !=
                                                    MPAM_RSRC_TYPE_PE_CACHE)
          continue;

        /*Check if the PE Cache ID matches LLC ID */
        if (val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index) != cache_identifier)
          continue;

        buf_size = cache_maxsize * CACHE_PERCENTAGE / 100 ;

        /*Allocate memory for source and destination buffers */
        src_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, buf_size);
        dest_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, buf_size);

        val_print(ACS_PRINT_DEBUG, "\n       buf_size            = 0x%x", buf_size);

        if ((src_buf == NULL) || (dest_buf == NULL)) {
            val_print(ACS_PRINT_ERR, "\n       Mem allocation failed", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 01));
        }

        /* Configure CSU monitors with partid1 */
        if (val_mpam_supports_csumon(msc_index))
            val_mpam_configure_csu_mon(msc_index, partid1, DEFAULT_PMG, 0);

        storage_value1 = val_mpam_read_csumon(msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Initial Value 1 = 0x%x", storage_value1);

        /* Enable CSU monitoring */
        val_mpam_csumon_enable(msc_index);

        /* wait for MAX_NRDY_USEC after msc config change */
        nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
        while (nrdy_timeout) {
            --nrdy_timeout;
        };

        /*Perform first memory transaction */
        val_memcpy(src_buf, dest_buf, buf_size);

        /* Read Cache storage value */
        storage_value1 = val_mpam_read_csumon(msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Storage Value 1 = 0x%x", storage_value1);

        /* Configure CSU monitors with partid2 */
        if (val_mpam_supports_csumon(msc_index))
            val_mpam_configure_csu_mon(msc_index, partid2, DEFAULT_PMG, 0);

        /* Read Cache storage value */
        storage_value2 = val_mpam_read_csumon(msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Initial Value 2 = 0x%x", storage_value2);

        /* Enable CSU monitoring */
        val_mpam_csumon_enable(msc_index);

        /* wait for MAX_NRDY_USEC after msc config change */
        nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
        while (nrdy_timeout) {
            --nrdy_timeout;
        };

        /*Perform second memory transaction */
        val_memcpy(src_buf, dest_buf, buf_size);

        /* Read Cache storage value for PMG2 */
        storage_value2 = val_mpam_read_csumon(msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Storage Value 2 = 0x%x", storage_value2);

        /* Disable the monitor */
        val_mpam_csumon_disable(msc_index);

        /* Test fails if storage_value1 is zero or storage_value2 is non zero */
        if (!storage_value1 || storage_value2) {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 02));

            /*Restore MPAM2_EL2 settings */
            val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);

            /*Free the buffers */
            val_memory_free_aligned(src_buf);
            val_memory_free_aligned(dest_buf);

            return;
        }

        /*Restore MPAM2_EL2 settings */
        val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);

        /*Free the buffers */
        val_memory_free_aligned(src_buf);
        val_memory_free_aligned(dest_buf);
      }
    }

    val_set_status(index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t monitor003_entry(void)
{
    uint32_t status = ACS_STATUS_FAIL;
    uint32_t num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    /* This check is when user is forcing us to skip this test */
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, ACS_END(TEST_NUM), TEST_RULE);

    return status;
}
