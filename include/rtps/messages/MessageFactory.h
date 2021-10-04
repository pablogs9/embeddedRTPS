/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#ifndef RTPS_MESSAGEFACTORY_H
#define RTPS_MESSAGEFACTORY_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/utils/sysFunctions.h"

#include <array>
#include <cstdint>

namespace rtps {
namespace MessageFactory {
const std::array<uint8_t, 4> PROTOCOL_TYPE{'R', 'T', 'P', 'S'};
const uint8_t numBytesUntilEndOfLength =
    4; // The first bytes incl. submessagelength don't count

template <class Buffer>
void addHeader(Buffer &buffer, const GuidPrefix_t &guidPrefix) {

  Header header;
  header.protocolName = PROTOCOL_TYPE;
  header.protocolVersion = PROTOCOLVERSION;
  header.vendorId = Config::VENDOR_ID;
  header.guidPrefix = guidPrefix;

  serializeMessage(buffer, header);
}

template <class Buffer>
bool addSubMessageInfoDST(Buffer &buffer, GuidPrefix_t &dst) {
  SubmessageInfoDST msg;
  msg.header.submessageId = SubmessageKind::INFO_DST;

#if IS_LITTLE_ENDIAN
  msg.header.flags = FLAG_LITTLE_ENDIAN;
#else
  msg.header.flags = FLAG_BIG_ENDIAN;
#endif

  msg.header.octetsToNextHeader = sizeof(GuidPrefix_t);
  msg.guidPrefix = dst;

  return serializeMessage(buffer, msg);
}

template <class Buffer>
void addSubMessageTimeStamp(Buffer &buffer, bool setInvalid = false) {
  SubmessageHeader header;
  header.submessageId = SubmessageKind::INFO_TS;

#if IS_LITTLE_ENDIAN
  header.flags = FLAG_LITTLE_ENDIAN;
#else
  header.flags = FLAG_BIG_ENDIAN;
#endif

  if (setInvalid) {
    header.flags |= FLAG_INVALIDATE;
    header.octetsToNextHeader = 0;
  } else {
    header.octetsToNextHeader = sizeof(Time_t);
  }

  serializeMessage(buffer, header);

  if (!setInvalid) {
    buffer.reserve(header.octetsToNextHeader);
    Time_t now = getCurrentTimeStamp();
    buffer.append(reinterpret_cast<uint8_t *>(&now.seconds),
                  sizeof(Time_t::seconds));
    buffer.append(reinterpret_cast<uint8_t *>(&now.fraction),
                  sizeof(Time_t::fraction));
  }
}

template <class Buffer>
void addSubMessageData(Buffer &buffer, const Buffer &filledPayload,
                       bool containsInlineQos, const SequenceNumber_t &SN,
                       const EntityId_t &writerID, const EntityId_t &readerID,
                       const Guid_t relatedWriterGuid = GUID_UNKNOWN,
                       const SequenceNumber_t relatedSequenceNumber = SEQUENCENUMBER_UNKNOWN) {
  SubmessageData msg;
  msg.header.submessageId = SubmessageKind::DATA;
#if IS_LITTLE_ENDIAN
  msg.header.flags = FLAG_LITTLE_ENDIAN;
#else
  msg.header.flags = FLAG_BIG_ENDIAN;
#endif

  msg.header.octetsToNextHeader = SubmessageData::getRawSize() +
                                  filledPayload.spaceUsed() -
                                  numBytesUntilEndOfLength;

  if (containsInlineQos) {
    msg.header.flags |= FLAG_INLINE_QOS;
  }
  if (filledPayload.isValid()) {
    msg.header.flags |= FLAG_DATA_PAYLOAD;
  }

  if(!(relatedWriterGuid == GUID_UNKNOWN) && !(relatedSequenceNumber == SEQUENCENUMBER_UNKNOWN)){
    containsInlineQos = true;
    msg.header.flags |= FLAG_INLINE_QOS;
    msg.header.octetsToNextHeader += 32;
  }

  msg.writerSN = SN;
  msg.extraFlags = 0;
  msg.readerId = readerID;
  msg.writerId = writerID;

  constexpr uint16_t octetsToInlineQoS =
      4 + 4 + 8; // EntityIds + SequenceNumber
  msg.octetsToInlineQos = octetsToInlineQoS;

  serializeMessage(buffer, msg);

  // Add inline QoS

  if (containsInlineQos && !(relatedWriterGuid == GUID_UNKNOWN) && !(relatedSequenceNumber == SEQUENCENUMBER_UNKNOWN)) {
    buffer.reserve(32);
    SMElement::ParameterId id = SMElement::ParameterId::PID_RELATED_SAMPLE_IDENTITY;
    buffer.append(reinterpret_cast<uint8_t *>(&id), sizeof(uint16_t));
    uint16_t length = 24;
    buffer.append(reinterpret_cast<uint8_t *>(&length), sizeof(uint16_t));
    buffer.append(reinterpret_cast<const uint8_t *>(relatedWriterGuid.prefix.id.data()), 12);
    buffer.append(reinterpret_cast<const uint8_t *>(relatedWriterGuid.entityId.entityKey.data()), 3);
    buffer.append(reinterpret_cast<const uint8_t *>(&relatedWriterGuid.entityId.entityKind), 1);
    buffer.append(reinterpret_cast<const uint8_t *>(&relatedSequenceNumber.high), sizeof(uint32_t));
    buffer.append(reinterpret_cast<const uint8_t *>(&relatedSequenceNumber.low), sizeof(uint32_t));
    id = SMElement::ParameterId::PID_SENTINEL;
    buffer.append(reinterpret_cast<uint8_t *>(&id), sizeof(uint16_t));
    length = 0;
    buffer.append(reinterpret_cast<uint8_t *>(&length), sizeof(uint16_t));
  }

  // Add data
  if (filledPayload.isValid()) {
    Buffer shallowCopy = filledPayload;
    buffer.append(std::move(shallowCopy));
  }
}

template <class Buffer>
void addHeartbeat(Buffer &buffer, EntityId_t writerId, EntityId_t readerId,
                  SequenceNumber_t firstSN, SequenceNumber_t lastSN,
                  Count_t count) {
  SubmessageHeartbeat subMsg;
  subMsg.header.submessageId = SubmessageKind::HEARTBEAT;
  subMsg.header.octetsToNextHeader =
      SubmessageHeartbeat::getRawSize() - numBytesUntilEndOfLength;
#if IS_LITTLE_ENDIAN
  subMsg.header.flags = FLAG_LITTLE_ENDIAN;
#else
  subMsg.header.flags = FLAG_BIG_ENDIAN;
#endif
  // Force response by not setting final flag.

  subMsg.writerId = writerId;
  subMsg.readerId = readerId;
  subMsg.firstSN = firstSN;
  subMsg.lastSN = lastSN;
  subMsg.count = count;

  serializeMessage(buffer, subMsg);
}

template <class Buffer>
void addAckNack(Buffer &buffer, EntityId_t writerId, EntityId_t readerId,
                SequenceNumberSet readerSNState, Count_t count,
                bool final_flag) {
  SubmessageAckNack subMsg;
  subMsg.header.submessageId = SubmessageKind::ACKNACK;
#if IS_LITTLE_ENDIAN
  subMsg.header.flags = FLAG_LITTLE_ENDIAN;
#else
  subMsg.header.flags = FLAG_BIG_ENDIAN;
#endif
  if (final_flag) {
    subMsg.header.flags |= FLAG_FINAL; // For now, we don't want any response
  } else {
    subMsg.header.flags &= ~FLAG_FINAL; // For now, we don't want any response
  }
  subMsg.header.octetsToNextHeader =
      SubmessageAckNack::getRawSize(readerSNState) - numBytesUntilEndOfLength;

  subMsg.writerId = writerId;
  subMsg.readerId = readerId;
  subMsg.readerSNState = readerSNState;
  subMsg.count = count;

  serializeMessage(buffer, subMsg);
}
} // namespace MessageFactory
} // namespace rtps

#endif // RTPS_MESSAGEFACTORY_H
