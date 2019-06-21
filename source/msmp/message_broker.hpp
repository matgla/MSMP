// #pragma once

// #include <eul/container/static_deque.hpp>
// #include <eul/function.hpp>
// #include <eul/logger/logger_factory.hpp>
// #include <eul/logger/logger.hpp>

// #include "msmp/configuration/configuration.hpp"
// #include "msmp/types.hpp"


// namespace msmp
// {

// template <typename Transceiver, typename Configuration = configuration::Configuration, std::size_t QueueSize = 255>
// class MessageBroker
// {
// public:
//     using CallbackType = eul::function<void(), sizeof(void*)>;
//     MessageBroker(eul::logger::logger_factory& logger_factory)
//         : logger_(create_logger(logger_factory))
//         , transceiver_(transceiver)
//     {
//         transceiver_.onData[this](const StreamType& payload){
//             handle_message(payload);
//         });
//     }

//     void register_handler(uint8_t message_id, const auto& callback)
//     {
//         handlers_.push_back(MessageHandler{message_id, callback});
//     }

//     template <typename Message>
//     void publish(const Message& message, const CallbackType& on_success, const CallbackType& on_failure)
//     {
//         const auto serialized_message = message.serialize();

//         transceiver_.send(serialized_message, on_success, on_failure);
//     }

// private:
//     using TransmitterCallbackType = typename Transceiver::TransmitterCallbackType;

//     void handle_message(const StreamType& payload) const
//     {
//         if (payload.size() == 0)
//         {
//             logger_.trace() << "Received empty message";
//             return;
//         }
//         const uint8_t id = payload[0];

//         for (const MessageHandler& handler : handlers_)
//         {
//             if (handler.id == id)
//             {
//                 handler.handle(payload);
//             }
//         }
//     }

//     using QueueType = eul::container::static_deque<MessageHandler, QueueSize>;

//     static auto& create_logger(eul::logger::logger_factory& logger_factory)
//     {
//         static auto logger = logger_factory.create("MessageBroker");
//         return logger;
//     }

//     eul::logger::logger logger_;
//     Transceiver& transceiver_;
//     QueueType handlers_;
// };

// } // namespace msmp
