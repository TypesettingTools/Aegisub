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
//
// $Id$

/// @file signals.h
/// @brief 
/// @ingroup libaegisub

#pragma once

#if !defined(AGI_PRE) && !defined(LAGI_PRE)
#ifdef _WIN32
#include <functional>
#include <map>
#include <memory>
#else
#include <tr1/functional>
#include <map>
#include <tr1/memory>
#endif
#endif

namespace agi {
	namespace signal {

using namespace std::tr1::placeholders;

class Connection;

/// Implementation details; nothing outside this file should directly touch
/// anything in the detail namespace
namespace detail {
	class SignalBase;
	class ConnectionToken {
		friend class agi::signal::Connection;
		friend class SignalBase;

		SignalBase *signal;
		bool blocked;
		bool claimed;

		ConnectionToken(SignalBase *signal) : signal(signal), blocked(false), claimed(false) { }
		inline void Disconnect();
	public:
		~ConnectionToken() { Disconnect(); }
	};
}

/// @class Connection
/// @brief Object representing a connection to a signal
class Connection {
	std::tr1::shared_ptr<detail::ConnectionToken> token;
public:
	Connection() { }
	Connection(detail::ConnectionToken *token) : token(token) { token->claimed = true; }

	/// @brief End this connection
	///
	/// This normally does not need to be manually called, as a connection is
	/// automatically closed when all Connection objects referring to it are
	/// gone. To temporarily enable or disable a connection, use Block/Unblock
	/// instead
	void Disconnect() { if (token.get()) token->Disconnect(); }

	/// @brief Disable this connection until Unblock is called
	void Block() { if (token.get()) token->blocked = true; }

	/// @brief Reenable this connection after it was disabled by Block
	void Unblock() { if (token.get()) token->blocked = false; }
};

/// @class UnscopedConnection
/// @brief A connection which is not automatically closed
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
	detail::ConnectionToken *token;
public:
	UnscopedConnection(detail::ConnectionToken *token) : token(token) { }
	operator Connection() { return Connection(token); }
};

namespace detail {
	/// @brief Polymorphic base class for slots
	///
	/// This class has two purposes: to avoid having to make Connection
	/// templated on what type of connection it is controlling, and to avoid
	/// some messiness with templated friend classes
	class SignalBase {
		friend class ConnectionToken;
		/// @brief Disconnect the passed slot from the signal
		/// @param tok Token to disconnect
		virtual void Disconnect(ConnectionToken *tok)=0;
	protected:
		/// @brief Notify a slot that it has been disconnected
		/// @param tok Token to disconnect
		///
		/// Used by the signal when the signal wishes to end a connection (such
		/// as if the signal is being destroyed while slots are still connected
		/// to it)
		void DisconnectToken(ConnectionToken *tok) { tok->signal = NULL; }

		/// @brief Has a token been claimed by a scoped connection object?
		bool TokenClaimed(ConnectionToken *tok) { return tok->claimed; }

		/// @brief Create a new connection to this slot
		ConnectionToken *MakeToken() { return new ConnectionToken(this); }

		/// @brief Check if a connection currently wants to receive signals
		bool Blocked(ConnectionToken *tok) { return tok->blocked; }
	};

	inline void ConnectionToken::Disconnect() {
		if (signal) signal->Disconnect(this);
		signal = NULL;
	}

	/// @brief Templated common code for signals
	template<class Slot>
	class SignalBaseImpl : public SignalBase {
	protected:
		typedef std::map<ConnectionToken*, Slot> SlotMap;

		SlotMap slots; /// Signals currently connected to this slot

		void Disconnect(ConnectionToken *tok) {
			slots.erase(tok);
		}

		/// Protected destructor so that we don't need a virtual destructor
		~SignalBaseImpl() {
			for (typename SlotMap::iterator cur = slots.begin(); cur != slots.end(); ++cur) {
				DisconnectToken(cur->first);
				if (!TokenClaimed(cur->first)) delete cur->first;
			}
		}
	public:
		/// @brief Connect a signal to this slot
		/// @param sig Signal to connect
		/// @return The connection object
		UnscopedConnection Connect(Slot sig) {
			ConnectionToken *token = MakeToken();
			slots.insert(std::make_pair(token, sig));
			return UnscopedConnection(token);
		}
		template<class F, class Arg1>
		UnscopedConnection Connect(F func, Arg1 a1) {
			return Connect(std::tr1::bind(func, a1));
		}
		template<class F, class Arg1, class Arg2>
		UnscopedConnection Connect(F func, Arg1 a1, Arg2 a2) {
			return Connect(std::tr1::bind(func, a1, a2));
		}
		template<class F, class Arg1, class Arg2, class Arg3>
		UnscopedConnection Connect(F func, Arg1 a1, Arg2 a2, Arg3 a3) {
			return Connect(std::tr1::bind(func, a1, a2, a3));
		}
	};
}

#define SIGNALS_H_FOR_EACH_SIGNAL(...) \
	for (typename super::SlotMap::iterator cur = slots.begin(); cur != slots.end(); ++cur) { \
		if (!Blocked(cur->first)) cur->second(__VA_ARGS__); \
	}

/// @class Signal
/// @brief Two-argument signal
/// @param Arg1 Type of first argument to pass to slots
/// @param Arg2 Type of second argument to pass to slots
template<class Arg1 = void, class Arg2 = void>
class Signal : public detail::SignalBaseImpl<std::tr1::function<void (Arg1, Arg2)> > {
	typedef detail::SignalBaseImpl<std::tr1::function<void (Arg1, Arg2)> > super;
	using super::Blocked;
	using super::slots;
public:
	Signal() { }

	/// @brief Trigger this signal
	/// @param a1 First argument to the signal
	/// @param a2 Second argument to the signal
	///
	/// The order in which connected slots are called is undefined and should
	/// not be relied on
	void operator()(Arg1 a1, Arg2 a2) { SIGNALS_H_FOR_EACH_SIGNAL(a1, a2) }

	// Don't hide the base overloads
	using super::Connect;

	/// @brief Connect a member function with the correct signature to this signal
	/// @param func Function to connect
	/// @param a1   Object
	///
	/// This overload is purely for convenience so that classes can do
	/// sig.Connect(&Class::Foo, this) rather than
	/// sig.Connect(&Class::Foo, this, _1, _2)
	template<class T>
	UnscopedConnection Connect(void (T::*func)(Arg1, Arg2), T* a1) {
		return Connect(std::tr1::bind(func, a1, _1, _2));
	}
};

/// @class Signal
/// @brief One-argument signal
/// @param Arg1 Type of the argument to pass to slots
template<class Arg1>
class Signal<Arg1, void> : public detail::SignalBaseImpl<std::tr1::function<void (Arg1)> > {
	typedef detail::SignalBaseImpl<std::tr1::function<void (Arg1)> > super;
	using super::Blocked;
	using super::slots;
public:
	Signal() { }

	/// @brief Trigger this signal
	/// @param a1 The argument to the signal
	///
	/// The order in which connected slots are called is undefined and should
	/// not be relied on
	void operator()(Arg1 a1) { SIGNALS_H_FOR_EACH_SIGNAL(a1) }

	// Don't hide the base overloads
	using super::Connect;

	/// @brief Connect a member function with the correct signature to this signal
	/// @param func Function to connect
	/// @param a1   Object
	///
	/// This overload is purely for convenience so that classes can do
	/// sig.Connect(&Class::Foo, this) rather than sig.Connect(&Class::Foo, this, _1)
	template<class T>
	UnscopedConnection Connect(void (T::*func)(Arg1), T* a1) {
		return Connect(std::tr1::bind(func, a1, _1));
	}
};

/// @class Signal
/// @brief Zero-argument signal
template<>
class Signal<void> : public detail::SignalBaseImpl<std::tr1::function<void ()> > {
	typedef detail::SignalBaseImpl<std::tr1::function<void ()> > super;
	using super::Blocked;
	using super::slots;
public:
	Signal() { }

#if defined(_WIN32) || defined(__FreeBSD__)
// MSVC incorrectly considers this not a template context due to it being fully
// specified, making typename invalid here
#define typename
#endif

	/// @brief Trigger this signal
	///
	/// The order in which connected slots are called is undefined and should
	/// not be relied on
	void operator()() { SIGNALS_H_FOR_EACH_SIGNAL() }
#undef typename
};

#undef SIGNALS_H_FOR_EACH_SIGNAL
	}
}

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
	template<class A>                         agi::signal::UnscopedConnection method(A a)             { return sig.Connect(a);       } \
	template<class A,class B>                 agi::signal::UnscopedConnection method(A a,B b)         { return sig.Connect(a,b);     } \
	template<class A,class B,class C>         agi::signal::UnscopedConnection method(A a,B b,C c)     { return sig.Connect(a,b,c);   } \
	template<class A,class B,class C,class D> agi::signal::UnscopedConnection method(A a,B b,C c,D d) { return sig.Connect(a,b,c,d); }
