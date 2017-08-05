#pragma once

#include <utility>
#include <functional>
#include <map>

template<class T>
class FSM {
public:
	using Transaction = std::pair<T, T>;
	using Handler = std::function<void(T &)>;

	FSM(T initialState) : state(initialState), previousState(initialState) {}

	~FSM() = default;

	void Update() {
		auto &handler = handlers[Transaction{previousState, state}];
		if (handler)
			handler(state);
		previousState = state;
	}

	void RegisterHandler(T state, Handler handler) {
		handlers[Transaction{state, state}] = handler;
	}

	void RegisterTransactionHandler(Transaction transaction, Handler handler) {
		handlers[transaction] = handler;
	}

private:
	T previousState;
	T state;
	std::map<Transaction, Handler> handlers;
};