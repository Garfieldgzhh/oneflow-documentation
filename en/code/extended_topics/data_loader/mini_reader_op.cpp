/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/core/framework/framework.h"

namespace oneflow {

REGISTER_CPU_ONLY_USER_OP("MiniReader")
    .Output("out")
    .Attr("data_dir", UserOpAttrType::kAtString)
    .Attr("data_part_num", UserOpAttrType::kAtInt32)
    .Attr<std::string>("part_name_prefix", UserOpAttrType::kAtString, "part-")
    .Attr<int32_t>("part_name_suffix_length", UserOpAttrType::kAtInt32, -1)
    .Attr("batch_size", UserOpAttrType::kAtInt32)
    .Attr<bool>("random_shuffle", UserOpAttrType::kAtBool, false)
    .Attr<bool>("shuffle_after_epoch", UserOpAttrType::kAtBool, false)
    .Attr<int64_t>("seed", UserOpAttrType::kAtInt64, -1)
    .Attr<int32_t>("shuffle_buffer_size", UserOpAttrType::kAtInt32, 1024)
    .SetTensorDescInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      user_op::TensorDesc* out_tensor = ctx->TensorDesc4ArgNameAndIndex("out", 0);
      int32_t local_batch_size = ctx->Attr<int32_t>("batch_size");
      const SbpParallel& sbp = ctx->SbpParallel4ArgNameAndIndex("out", 0);
      int64_t parallel_num = ctx->parallel_ctx().parallel_num();
      if (sbp.has_split_parallel() && parallel_num > 1) {
        CHECK_EQ_OR_RETURN(local_batch_size % parallel_num, 0);
        local_batch_size /= parallel_num;
      }
      *out_tensor->mut_shape() = Shape({local_batch_size, 2});
      *out_tensor->mut_data_type() = DataType::kDouble;
      return Maybe<void>::Ok();
    })
    .SetGetSbpFn([](user_op::SbpContext* ctx) -> Maybe<void> {
      ctx->NewBuilder().Split(ctx->outputs(), 0).Build();
      return Maybe<void>::Ok();
    });

}  // namespace oneflow