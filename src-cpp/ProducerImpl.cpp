/*
 * librdkafka - Apache Kafka C/C++ library
 *
 * Copyright (c) 2014 Magnus Edenhill
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <string>
#include <list>
#include <cerrno>

#include "rdkafkacpp_int.h"


RdKafka::Producer::~Producer () {

};

static void dr_msg_cb_trampoline (rd_kafka_t *rk,
                                  const rd_kafka_message_t *
                                  rkmessage,
                                  void *opaque) {
  RdKafka::HandleImpl *handle = static_cast<RdKafka::HandleImpl *>(opaque);
  RdKafka::MessageImpl message(NULL, rkmessage);
  handle->dr_cb_->dr_cb(message);
}



RdKafka::Producer *RdKafka::Producer::create (RdKafka::Conf *conf,
                                              std::string &errstr) {
  char errbuf[512];
  RdKafka::ConfImpl *confimpl = dynamic_cast<RdKafka::ConfImpl *>(conf);
  RdKafka::ProducerImpl *rkp = new RdKafka::ProducerImpl();
  rd_kafka_conf_t *rk_conf = NULL;

  if (confimpl) {
    if (!confimpl->rk_conf_) {
      errstr = "Requires RdKafka::Conf::CONF_GLOBAL object";
      delete rkp;
      return NULL;
    }

    rkp->set_common_config(confimpl);

    rk_conf = rd_kafka_conf_dup(confimpl->rk_conf_);

    if (confimpl->dr_cb_) {
      rd_kafka_conf_set_dr_msg_cb(rk_conf, dr_msg_cb_trampoline);
      rkp->dr_cb_ = confimpl->dr_cb_;
    }
  }


  rd_kafka_t *rk;
  if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, rk_conf,
                          errbuf, sizeof(errbuf)))) {
    errstr = errbuf;
    delete rkp;
    return NULL;
  }

  rkp->rk_ = rk;

  return rkp;
}


RdKafka::ErrorCode RdKafka::ProducerImpl::produce (RdKafka::Topic *topic,
                                                   int32_t partition,
                                                   int msgflags,
                                                   void *payload, size_t len,
                                                   const std::string *key,
                                                   void *msg_opaque) {
  RdKafka::TopicImpl *topicimpl = dynamic_cast<RdKafka::TopicImpl *>(topic);

  if (rd_kafka_produce(topicimpl->rkt_, partition, msgflags,
                       payload, len,
                       key ? key->c_str() : NULL, key ? key->size() : 0,
                       msg_opaque) == -1)
    return static_cast<RdKafka::ErrorCode>(rd_kafka_errno2err(errno));

  return RdKafka::ERR_NO_ERROR;
}


RdKafka::ErrorCode RdKafka::ProducerImpl::produce (RdKafka::Topic *topic,
                                                   int32_t partition,
                                                   int msgflags,
                                                   void *payload, size_t len,
                                                   const void *key,
                                                   size_t key_len,
                                                   void *msg_opaque) {
  RdKafka::TopicImpl *topicimpl = dynamic_cast<RdKafka::TopicImpl *>(topic);

  if (rd_kafka_produce(topicimpl->rkt_, partition, msgflags,
                       payload, len, key, key_len,
                       msg_opaque) == -1)
    return static_cast<RdKafka::ErrorCode>(rd_kafka_errno2err(errno));

  return RdKafka::ERR_NO_ERROR;
}
