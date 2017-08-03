#pragma once

#include <utility>
#include <functional>
#include <map>

template<class T>
class FSM {
	T previousState;
	T &state;
	std::map<Transaction, std::function<void()>> handlers;
public:
	using Transaction = std::pair<T, T>;

	FSM(T &value) : state(value), previousState(value) {}

	~FSM() = default;

	void Update() {
		auto handler = handlers[Transaction{previousState, state}];
		if (handler)
			handler();
	}

	void RegisterHandler(T state, std::function<void()> handler) {
		handlers[Transaction{state, state}] = handler;
	}

	void RegisterTransactionHandler(Transaction transaction, std::function<void()> handler) {
		handlers[transaction] = handler;
	}
};