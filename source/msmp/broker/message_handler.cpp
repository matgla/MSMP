#include "msmp/broker/message_handler.hpp"

namespace msmp
{
namespace broker
{

MessageHandler::MessageHandler()
    : observing_node_(this)
{

}

MessageHandler::ObservingNodeType& MessageHandler::getObservingNode()
{
    return observing_node_;
}

} // namespace broker
} // namespace msmp
