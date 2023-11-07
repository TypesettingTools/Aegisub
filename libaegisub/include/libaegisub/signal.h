// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#pragma once

#include <boost/config.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace agi { namespace signal {
class Connection;

/// Implementation details; nothing outside this file should directly touch
/// anything in the detail namespace
namespace detail {
	class SignalBase;
	class ConnectionToken {
		friend class agi::signal::Connection;
		friend class SignalBase;

		SignalBase *signal;
		bool blocked = false;
		bool claimed = false;

		ConnectionToken(SignalBase *signal) : signal(signal) { }
		inline void Disconnect();
	public:
		~ConnectionToken() { Disconnect(); }
	};
}

/// A connection which is not automatically closed
///
/// Connections initially start out owned by the signal. If a slot knows that it
/// will outlive a signal and does not need to be able to block a connection, it
/// can simply ignore the return value of Connect.
///
/// If a slot needs to be able to disconnect from a signal, it should store the
/// returned connection in a Connection, which transfers ownership of the
/// connection to the slot. If there is any chance that the signal will outlive
/// the slot, this must be done.
class UnscopedConnection {
	friend class Connection;
	detail::ConnectionToken *token;
public:
	UnscopedConnection(detail::ConnectionToken *token) : token(token) { }
};

/// Object representing a connection to a signal
class Connection {
	std::unique_ptr<detail::ConnectionToken> token;
public:
	Connection() = default;
	Connection(UnscopedConnection src) BOOST_NOEXCEPT : token(src.token) { token->claimed = true; }
	Connection(Connection&& that) BOOST_NOEXCEPT : token(std::move(that.token)) { }
	Connection(detail::ConnectionToken *token) BOOST_NOEXCEPT : token(token) { token->claimed = true; }
	Connection& operator=(Connection&& that) BOOST_NOEXCEPT { token = std::move(that.token); return *this; }

	/// @brief End this connection
	///
	/// This normally does not need to be manually called, as a connection is
	/// automatically closed when all Connection objects referring to it are
	/// gone. To temporarily enable or disable a connection, use Block/Unblock
	/// instead
	void Disconnect() { if (token) token->Disconnect(); }

	/// @brief Disable this connection until Unblock is called
	void Block() { if (token) token->blocked = true; }

	/// @brief Reenable this connection after it was disabled by Block
	void Unblock() { if (token) token->blocked = false; }
};

namespace detail {
	/// Polymorphic base class for slots
	///
	/// This class has two purposes: to avoid having to make Connection
	/// templated on what type of connection it is controlling, and to avoid
	/// some messiness with templated friend classes
	class SignalBase {
		friend class ConnectionToken;
		/// @brief Disconnect the passed slot from the signal
		/// @param tok Token to disconnect
		virtual void Disconnect(ConnectionToken *tok)=0;

		/// Signals can't be copied
		SignalBase(SignalBase const&) = delete;
		SignalBase& operator=(SignalBase const&) = delete;
	protected:
		SignalBase() = default;
		virtual ~SignalBase() {};
		/// @brief Notify a slot that it has been disconnected
		/// @param tok Token to disconnect
		///
		/// Used by the signal when the signal wishes to end a connection (such
		/// as if the signal is being destroyed while slots are still connected
		/// to it)
		void DisconnectToken(ConnectionToken *tok) { tok->signal = nullptr; }

		/// @brief Has a token been claimed by a scoped connection object?
		bool TokenClaimed(ConnectionToken *tok) { return tok->claimed; }

		/// @brief Create a new connection to this slot
		ConnectionToken *MakeToken() { return new ConnectionToken(this); }

		/// @brief Check if a connection currently wants to receive signals
		bool Blocked(ConnectionToken *tok) { return tok->blocked; }
	};

	inline void ConnectionToken::Disconnect() {
		if (signal) signal->Disconnect(this);
		signal = nullptr;
	}
}

template<typename... Args>
class Signal final : private detail::SignalBase {
	using Slot = std::function<void(Args...)>;
	std::vector<std::pair<detail::ConnectionToken*, Slot>> slots; /// Signals currently connected to this slot

	void Disconnect(detail::ConnectionToken *tok) override {
		for (auto it = begin(slots), e = end(slots); it != e; ++it) {
			if (tok == it->first) {
				slots.erase(it);
				return;
			}
		}
	}

	UnscopedConnection DoConnect(Slot sig) {
		auto token = MakeToken();
		slots.emplace_back(token, sig);
		return UnscopedConnection(token);
	}

public:
	~Signal() {
		for (auto& slot : slots) {
			DisconnectToken(slot.first);
			if (!TokenClaimed(slot.first)) delete slot.first;
		}
	}

	/// Trigger this signal
	///
	/// The order in which connected slots are called is undefined and should
	/// not be relied on
	void operator()(Args... args) {
		for (size_t i = slots.size(); i > 0; --i) {
			if (!Blocked(slots[i - 1].first))
				slots[i - 1].second(args...);
		}
	}

	/// @brief Connect a signal to this slot
	/// @param sig Signal to connect
	/// @return The connection object
	UnscopedConnection Connect(Slot sig) {
		return DoConnect(sig);
	}

	// Convenience wrapper for a member function which matches the signal's signature
	template<typename T>
	UnscopedConnection Connect(void (T::*func)(Args...), T* a1) {
		return DoConnect([=,  this](Args... args) { (a1->*func)(args...); });
	}

	// Convenience wrapper for a callable which does not use any signal args
	template<typename Thunk, typename = decltype((*(Thunk *)0)())>
	UnscopedConnection Connect(Thunk&& func) {
		return DoConnect([=,  this](Args... args) mutable { func(); });
	}

	// Convenience wrapper for a member function which does not use any signal
	// args. The match is overly-broad to avoid having two methods with the
	// same signature when the signal has no args.
	template<typename T, typename MemberThunk>
	UnscopedConnection Connect(MemberThunk func, T* obj) {
		return DoConnect([=,  this](Args... args) { (obj->*func)(); });
	}

	// Convenience wrapper for a member function which uses only the first
	// signal arg.
	template<typename T, typename Arg1>
	UnscopedConnection Connect(void (T::*func)(Arg1), T* a1) {
		return DoConnect(std::bind(func, a1, std::placeholders::_1));
	}
};

/// Create a vector of scoped connections from an initializer list
///
/// Required due to that initializer lists copy their input, and trying to pass
/// an initializer list directly to a vector results in a
/// std::initializer_list<Connection>, which can't be copied.
inline std::vector<Connection> make_vector(std::initializer_list<UnscopedConnection> connections) {
	return std::vector<Connection>(std::begin(connections), std::end(connections));
}

} }

/// @brief Define functions which forward their arguments to the connect method
///        of the named signal
/// @param sig Name of the signal
/// @param method Name of the connect method
///
/// When a signal is a member of a class, we typically want other objects to be
/// able to connect to the signal, but not to be able to trigger it. To do this,
/// make this signal private then use this macro to create public subscription
/// methods
///
/// Defines AddSignalNameListener
#define DEFINE_SIGNAL_ADDERS(sig, method) \
	template<typename... Args> \
	agi::signal::UnscopedConnection method(Args&&... args) { \
		return sig.Connect(std::forward<Args>(args)...); \
	}
