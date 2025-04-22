#pragma once

#include <functional>
#include <vector>
#include <map>
#include <unordered_set>
#include <memory>


template <class EventType,class ReturnType, typename... ArgTypes> class Event;


template <class EventType, class ReturnType, typename... ArgTypes>
class Event<EventType, ReturnType(ArgTypes...)> {
public:
	void Invoke(EventType type, ArgTypes... args) {
		if (m_listeners.find(type) == m_listeners.end()) {
			return;
		}

		for (auto& func : m_listeners[type]) {
			func.get()->operator()(args...);
		}
	}
	void operator()(EventType type, ArgTypes... args) {
		Invoke(type, args...);
	}

	struct EventHandler {
		EventType Type;
		std::shared_ptr<std::function<ReturnType(ArgTypes...)>> Function;
	};


	EventHandler AddListener(EventType type, std::function<ReturnType(ArgTypes...)> function) {
		std::shared_ptr<std::function<ReturnType(ArgTypes...)>> func = std::make_shared<std::function<ReturnType(ArgTypes...)>>(function);

		EventHandler handler = {};
		handler.Type = type;
		handler.Function = func;

		m_listeners[type].insert(func);

		return handler;
	}
	void RemoveListener(EventHandler handler) {
		if (m_listeners.find(handler.Type) == m_listeners.end()) {

			return;
		}

		m_listeners[handler.Type].erase(handler.Function);
	}

	std::vector<EventType> GetEventTypes() const {
		std::vector<EventType> output = {};

		for (const auto& elem : m_listeners) {
			output.push_back(elem.first);
		}

		return output;
	}

protected:
	std::map<EventType, std::unordered_set<std::shared_ptr<std::function<ReturnType(ArgTypes...)>>>> m_listeners;
};
