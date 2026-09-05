#include <gtest/gtest.h>
#include <Fw/FPrimeBasicTypes.hpp>
#include <limits>

#include "Fw/Types/Serializable.hpp"

namespace ExternalSerializeBufferTest {

using SizeType = Fw::Serializable::SizeType;

constexpr SizeType BUFFER_SIZE = 10;

U8 buffer[BUFFER_SIZE];

void serializeOK(Fw::ExternalSerializeBuffer& esb) {
    const SizeType buffCapacity = esb.getCapacity();
    ASSERT_EQ(esb.getSize(), 0);
    for (SizeType i = 0; i < buffCapacity; i++) {
        const U8 value = static_cast<U8>(i);
        const Fw::SerializeStatus status = esb.serializeFrom(value);
        ASSERT_EQ(status, Fw::FW_SERIALIZE_OK);
    }
    ASSERT_EQ(esb.getSize(), buffCapacity);
}

void serializeFail(Fw::ExternalSerializeBuffer& esb) {
    ASSERT_EQ(esb.getSize(), esb.getCapacity());
    const Fw::SerializeStatus status = esb.serializeFrom(static_cast<U8>(0));
    ASSERT_EQ(status, Fw::FW_SERIALIZE_NO_ROOM_LEFT);
}

void deserializeOK(Fw::ExternalSerializeBuffer& esb) {
    const SizeType buffCapacity = esb.getCapacity();
    ASSERT_EQ(esb.getDeserializeSizeLeft(), buffCapacity);
    for (SizeType i = 0; i < buffCapacity; i++) {
        U8 value = 0;
        const Fw::SerializeStatus status = esb.deserializeTo(value);
        ASSERT_EQ(status, Fw::FW_SERIALIZE_OK);
        ASSERT_EQ(value, static_cast<U8>(i));
    }
    ASSERT_EQ(esb.getDeserializeSizeLeft(), 0);
}

void deserializeFail(Fw::ExternalSerializeBuffer& esb) {
    U8 value = 0;
    ASSERT_EQ(esb.getDeserializeSizeLeft(), 0);
    const Fw::SerializeStatus status = esb.deserializeTo(value);
    ASSERT_EQ(status, Fw::FW_DESERIALIZE_BUFFER_EMPTY);
}

TEST(ExternalSerializeBuffer, Basic) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    // Serialization should succeed
    serializeOK(esb);
    // Serialization should fail
    serializeFail(esb);
    // Deserialization should succeed
    deserializeOK(esb);
    // Deserialization should fail
    deserializeFail(esb);
}

// #5816: the byte-array overload checked m_serLoc + length > m_capacity. With one byte already
// serialized and length at the type maximum the sum wraps to 0, the check passes and a copy of
// length bytes runs past the buffer. The remaining-capacity form cannot wrap.
TEST(ExternalSerializeBuffer, SerializeBytesLengthNearMax) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    const U8 byte = 0x5A;
    ASSERT_EQ(esb.serializeFrom(&byte, 1, Fw::Serialization::OMIT_LENGTH), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.serializeFrom(&byte, std::numeric_limits<SizeType>::max(), Fw::Serialization::OMIT_LENGTH),
              Fw::FW_SERIALIZE_NO_ROOM_LEFT);
    ASSERT_EQ(esb.getSize(), 1);
    // exact fit and one byte over at the boundary
    const U8 fill[BUFFER_SIZE] = {};
    ASSERT_EQ(esb.serializeFrom(fill, BUFFER_SIZE - 1, Fw::Serialization::OMIT_LENGTH), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.getSize(), BUFFER_SIZE);
    ASSERT_EQ(esb.serializeFrom(&byte, 1, Fw::Serialization::OMIT_LENGTH), Fw::FW_SERIALIZE_NO_ROOM_LEFT);
    ASSERT_EQ(esb.getSize(), BUFFER_SIZE);
}

// #5816: the buffer overload checked m_serLoc + size + sizeof(FwSizeStoreType) > m_capacity,
// which wraps the same way when the source buffer reports a size near the type maximum
TEST(ExternalSerializeBuffer, SerializeBufferSizeNearMax) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    U8 backing[BUFFER_SIZE] = {};
    // capacity and length are only bookkeeping here; the fix must reject before any copy
    Fw::ExternalSerializeBuffer big(backing, std::numeric_limits<SizeType>::max());
    ASSERT_EQ(big.setBuffLen(std::numeric_limits<SizeType>::max() - 1), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.serializeFrom(big), Fw::FW_SERIALIZE_NO_ROOM_LEFT);
    ASSERT_EQ(esb.getSize(), 0);
}

TEST(ExternalSerializeBuffer, SerializeBufferExactFit) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    U8 backing[BUFFER_SIZE] = {};
    Fw::ExternalSerializeBuffer val(backing, BUFFER_SIZE);
    // length prefix plus payload exactly fills the destination
    ASSERT_EQ(val.setBuffLen(BUFFER_SIZE - sizeof(FwSizeStoreType)), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.serializeFrom(val), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.getSize(), BUFFER_SIZE);
    // one payload byte more does not fit, and nothing is written
    esb.resetSer();
    ASSERT_EQ(val.setBuffLen(BUFFER_SIZE - sizeof(FwSizeStoreType) + 1), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(esb.serializeFrom(val), Fw::FW_SERIALIZE_NO_ROOM_LEFT);
    ASSERT_EQ(esb.getSize(), 0);
}

TEST(ExternalSerializeBuffer, Clear) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    // Serialization should succeed
    serializeOK(esb);
    // Clear the buffer
    esb.clear();
    // Serialization should fail
    serializeFail(esb);
    // Deserialization should fail
    deserializeFail(esb);
}

TEST(ExternalSerializeBuffer, SetExtBuffer) {
    Fw::ExternalSerializeBuffer esb(buffer, BUFFER_SIZE);
    // Serialization should succeed
    serializeOK(esb);
    // Set the buffer
    // This should also clear the serialization state
    esb.setExtBuffer(buffer, BUFFER_SIZE);
    // Serialization should succeed
    serializeOK(esb);
    // Deserialization should succeed
    deserializeOK(esb);
}

TEST(ExternalSerializeBufferWithMemberCopy, Assign) {
    Fw::ExternalSerializeBufferWithMemberCopy esb1(buffer, BUFFER_SIZE);
    Fw::ExternalSerializeBufferWithMemberCopy esb2(buffer, BUFFER_SIZE);
    // Serialization should succeed
    serializeOK(esb1);
    // Assign esb2 to esb1
    esb1 = esb2;
    // Deserialization should fail
    deserializeFail(esb1);
    // Serialization should succeed
    serializeOK(esb1);
}

}  // namespace ExternalSerializeBufferTest
