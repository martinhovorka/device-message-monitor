#include "MessageProcessor.hpp"

std::mutex MessageProcessor::processLock;
std::condition_variable MessageProcessor::processCondition;