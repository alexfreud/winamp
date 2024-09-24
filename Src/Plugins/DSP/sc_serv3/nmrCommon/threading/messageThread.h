#ifndef messageThread_H_
#define messageThread_H_

#include "thread.h"
#include "MT_stl.h"
#include <deque>
#ifdef _WIN32
#include <crtdbg.h>
#else
#include <assert.h>
#endif

#ifndef _WIN32
#define _ASSERTE assert
#endif

/////////////////////////////////////////////////////////////////
//* templates that allow you to build a message driven thread
//*
//*
//* define a class that is your handler. It must have a type
//* message_t which is the message type, and a method
//* bool operator()(const message_t &m) which is the message handler.
//* this method should return false when all processing is done and
//* the thread should shutdown
//*
/* example

class mh
{
	string m_string;

public:
	struct message
	{
		int m_v;
		message(int v):m_v(v){}
	};
	typedef message message_t;

protected:

	bool operator()(const message_t &mm)
	{
		if (!mm.m_v) return false; // if value is zero, then we are done
		::MessageBox(0,tos(mm.m_v).c_str(),m_string.c_str(),MB_OK);
		return true;
	}

	mh(const string &s):m_string(s){}
};

main()
{
messageThread<mh> m("this is a test");
m.start();
m.postMessage(m::message(1));
m.postMessage(m::message(2));
m.postMessage(m::message(3));
m.postMessage(m::message(4));
m.postMessage(m::message(0));
m.join();
}

*/

template<class Handler,class Queue>
class messageHandler: public Handler
{
private:
	Queue	m_queue;

protected:
	unsigned operator()() throw()
	{
		unsigned result = (unsigned)-1;
		try
		{
			while (true)
			{
				typename Queue::value_type m = m_queue.get();
				if (!Handler::operator()(m))
					break;
			}
			result = 0;
		}
		catch(...){}

		return result;
	}
public:
	inline void postMessage(const typename Queue::message_t &m)
	{
		m_queue.push(m);
	}

	inline typename Queue::size_type pendingMessages() const throw() { return m_queue.size(); }
	inline void clearMessageQueue() throw() { m_queue.clear(); }

	messageHandler(){}
	template <class P1> messageHandler(const P1 &p1):Handler(p1){}
	template <class P1,class P2> messageHandler(const P1 &p1,const P2 &p2):Handler(p1,p2){}
	template <class P1,class P2,class P3> messageHandler(const P1 &p1,const P2 &p2,const P3 &p3):Handler(p1,p2,p3){}
	template <class P1,class P2,class P3,class P4> messageHandler(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4):Handler(p1,p2,p3,p4){}
	template <class P1,class P2,class P3,class P4,class P5> messageHandler(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5):Handler(p1,p2,p3,p4,p5){}
	template <class P1,class P2,class P3,class P4,class P5,class P6> messageHandler(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6):Handler(p1,p2,p3,p4,p5,p6){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> messageHandler(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6,const P7 &p7):Handler(p1,p2,p3,p4,p5,p6,p7){}
	template <class P1> messageHandler(P1 &p1):Handler(p1){}
};

template<class Handler,class Message = typename Handler::message_t>
class messageThread: public Tthread<messageHandler<Handler,MT_queue<Message> > >
{
public:
	messageThread(){}
	template <class P1> messageThread(const P1 &p1):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1){}
	template <class P1,class P2> messageThread(const P1 &p1,const P2 &p2):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2){}
	template <class P1,class P2,class P3> messageThread(const P1 &p1,const P2 &p2,const P3 &p3):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2,p3){}
	template <class P1,class P2,class P3,class P4> messageThread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2,p3,p4){}
	template <class P1,class P2,class P3,class P4,class P5> messageThread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2,p3,p4,p5){}
	template <class P1,class P2,class P3,class P4,class P5,class P6> messageThread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2,p3,p4,p5,p6){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> messageThread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6,const P7 &p7):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1,p2,p3,p4,p5,p6,p7){}
	template <class P1> messageThread(P1 &p1):Tthread<messageHandler<Handler,MT_queue<Message> > >(p1){}
};

//////////////// Finite state machine
template <class Handler,typename RETVAL = bool>
class fsm: public Handler {

public:
	typedef RETVAL (Handler::*state)(const typename Handler::message_t &m);// throw(); // should not throw an exception

	// this is broken in VC 7, so trans must be made public	
	//friend class Handler;
public:
	RETVAL operator()(const typename Handler::message_t &m) throw() { _ASSERTE(m_State);return (this->*m_State)(m); }

	void trans(state target) throw()
	{
		if (target == m_State) return;
		_ASSERTE(m_State);
#ifndef NDEBUG
		state tmp = m_State;
#endif
		(this->*m_State)(Handler::message_t::exitStateMessage());
#ifndef NDEBUG
		// sanity check - don't change states in exit_sig
		_ASSERTE(tmp == m_State);
#endif
		m_State = target;
		(this->*m_State)(Handler::message_t::enterStateMessage());
		_ASSERTE(m_State == target); // don't change states in enter_sig
	}
public:

	fsm():m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1> fsm(const P1 &p1):
		Handler(p1),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1> fsm(P1 &p1):
		Handler(p1),m_State(&fsm<Handler,RETVAL>::initialState){}

	template <class P1,class P2> fsm(const P1 &p1,const P2 &p2):
		Handler(p1,p2),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2> fsm(const P1 &p1,P2 &p2):
		Handler(p1,p2),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2> fsm(P1 &p1,P2 &p2):
		Handler(p1,p2),m_State(&fsm<Handler,RETVAL>::initialState){}

	template <class P1,class P2,class P3> fsm(const P1 &p1,const P2 &p2,const P3 &p3):
		Handler(p1,p2,p3),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2,class P3,class P4> fsm(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4):
		Handler(p1,p2,p3,p4),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2,class P3,class P4,class P5> fsm(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5):
		Handler(p1,p2,p3,p4,p5),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2,class P3,class P4,class P5,class P6> fsm(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6):
		Handler(p1,p2,p3,p4,p5,p6),m_State(&fsm<Handler,RETVAL>::initialState){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> fsm(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6,const P7 &p7):
		Handler(p1,p2,p3,p4,p5,p6,p7),m_State(&fsm<Handler,RETVAL>::initialState){}

private:
	state	m_State;
};

#endif
