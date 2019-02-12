// #pragma once

// #include <gsl/span>

// #include <eul/mpl/mixin/access.hpp>

// #include "msmp/default_configuration.hpp"
// #include "msmp/payload_receiver.hpp"
// #include "msmp/payload_transmitter.hpp"


// namespace msmp
// {

// template <typename T>
// struct is_receiver : std::false_type
// {
// };

// template <typename T>
// struct is_transmitter : std::false_type
// {
// };

// template <typename BaseType>
// class PayloadTransceiver
// {
//     PayloadTransceiver();

//     bool write(const gsl::span<const uint8_t>& payload);

// private:
//     eul::mpl::mixin::access<BaseType> data_;
// };

// template <typename BaseType>
// PayloadTransceiver<BaseType>::PayloadTransceiver() : data_(this)
// {
// }

// template <typename BaseType>
// bool PayloadTransceiver<BaseType>::write(const gsl::span<const uint8_t>& payload)
// {
//     auto& transmitter = data_[eul::mpl::mixin::ability<is_transmitter>{}];
//     transmitter.send()
// }

// } // namespace msmp
