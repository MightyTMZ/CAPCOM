#include "capcom/collectors/NetworkCollector.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>

#include <stdexcept>
#include <utility>

namespace {

std::pair<uint64_t, uint64_t> getNetworkByteTotals() {
    MIB_IF_TABLE2* table = nullptr;
    if (GetIfTable2(&table) != NO_ERROR) {
        throw std::runtime_error("GetIfTable2 failed");
    }

    uint64_t receivedBytes = 0;
    uint64_t transmittedBytes = 0;

    for (ULONG index = 0; index < table->NumEntries; ++index) {
        const MIB_IF_ROW2& row = table->Table[index];
        const bool isActive = row.OperStatus == IfOperStatusUp;
        const bool isLoopback = row.Type == IF_TYPE_SOFTWARE_LOOPBACK;

        if (isActive && !isLoopback) {
            receivedBytes += row.InOctets;
            transmittedBytes += row.OutOctets;
        }
    }

    FreeMibTable(table);
    return {receivedBytes, transmittedBytes};
}

}

namespace capcom {

void NetworkCollector::initialize() {
    const auto [receivedBytes, transmittedBytes] = getNetworkByteTotals();
    previous_rx_ = receivedBytes;
    previous_tx_ = transmittedBytes;
    previous_time_ = std::chrono::steady_clock::now();
    initialized_ = true;
}

void NetworkCollector::sample(Telemetry& data) {
    const auto now = std::chrono::steady_clock::now();
    const auto [receivedBytes, transmittedBytes] = getNetworkByteTotals();

    if (initialized_) {
        const double elapsedSeconds =
            std::chrono::duration<double>(now - previous_time_).count();

        if (elapsedSeconds > 0.0) {
            data.network_rx_bytes_per_sec =
                static_cast<double>(receivedBytes - previous_rx_) /
                elapsedSeconds;
            data.network_tx_bytes_per_sec =
                static_cast<double>(transmittedBytes - previous_tx_) /
                elapsedSeconds;
        }
    }

    previous_rx_ = receivedBytes;
    previous_tx_ = transmittedBytes;
    previous_time_ = now;
    initialized_ = true;
}

}
