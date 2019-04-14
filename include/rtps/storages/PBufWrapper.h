/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PBUFWRAPPER_H
#define RTPS_PBUFWRAPPER_H

#include "lwip/pbuf.h"
#include "rtps/common/types.h"

/**
 * This Wrapper handles the lifetime of a pbuf element. It either allocates it
 * when constructed or wraps an existing one. In both cases it is freed when the wrapper is destructed.
 */

namespace rtps {

    struct PBufWrapper {

        pbuf *firstElement = nullptr;

        PBufWrapper() = default;
        explicit PBufWrapper(pbuf* bufferToWrap);
        explicit PBufWrapper(DataSize_t length);

        // Shallow Copy. No copying of the underlying pbuf. Just another reference like a shared pointer.
        PBufWrapper(const PBufWrapper& other);
        PBufWrapper& operator=(const PBufWrapper& other);

        PBufWrapper(PBufWrapper&& other) noexcept;
        PBufWrapper& operator=(PBufWrapper&& other) noexcept;

        ~PBufWrapper();

        PBufWrapper deepCopy() const;

        bool isValid() const;

        bool append(const uint8_t* data, DataSize_t length);


        /// Note that unused reserved memory is now part of the wrapper. New calls to append(uint8_t*[...]) will
        /// continue behind the appended wrapper
        void append(PBufWrapper&& other);

        bool reserve(DataSize_t length);

        /// After calling this function, data is added starting from the beginning again. It does not revert reserve.
        void reset();

        DataSize_t spaceLeft() const;
        DataSize_t spaceUsed() const;

    private:

        constexpr static pbuf_layer m_layer = PBUF_TRANSPORT;
        constexpr static pbuf_type m_type = PBUF_POOL;

        DataSize_t m_freeSpace = 0;

        bool increaseSizeBy(uint16_t length);

        void copySimpleMembersAndResetBuffer(const PBufWrapper& other);
    };

}

#endif //RTPS_PBUFWRAPPER_H
