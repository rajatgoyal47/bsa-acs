/** @file
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

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
#ifndef __GIC_V3_H__
#define __GIC_V3_H__

void v3_Init(void);
void v3_EnableInterruptSource(uint32_t);
void v3_DisableInterruptSource(uint32_t);
uint32_t v3_AcknowledgeInterrupt(void);
void v3_EndofInterrupt(uint32_t int_id);

#endif /*__GIC_V3_H__ */