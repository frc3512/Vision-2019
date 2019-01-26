// Copyright (c) 2017-2019 FRC Team 3512. All Rights Reserved.

#include "Packet.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstring>

void Packet::append(const void* data, size_t sizeInBytes) {
    if (data && (sizeInBytes > 0)) {
        size_t start = m_packetData.size();
        m_packetData.resize(start + sizeInBytes);
        std::memcpy(&m_packetData[start], data, sizeInBytes);
    }
}

void Packet::clear() { m_packetData.clear(); }

const void* Packet::getData() const {
    if (!m_packetData.empty()) {
        return &m_packetData[0];
    } else {
        return nullptr;
    }
}

size_t Packet::getDataSize() const { return m_packetData.size(); }

Packet& Packet::operator<<(bool data) {
    *this << static_cast<uint8_t>(data);
    return *this;
}

Packet& Packet::operator<<(int8_t data) {
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator<<(uint8_t data) {
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator<<(int16_t data) {
    int16_t toWrite = htons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(uint16_t data) {
    uint16_t toWrite = htons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(int32_t data) {
    int32_t toWrite = htonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(uint32_t data) {
    uint32_t toWrite = htonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(float data) {
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator<<(double data) {
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator<<(const std::string& data) {
    // First insert string length
    uint32_t length = static_cast<uint32_t>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0) {
        append(data.c_str(), length * sizeof(std::string::value_type));
    }

    return *this;
}
