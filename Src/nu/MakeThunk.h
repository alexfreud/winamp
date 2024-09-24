#ifndef NULLSOFT_MAKETHUNKH
#define NULLSOFT_MAKETHUNKH

#include <vector>


class ThunkHolder
{
private:
#pragma pack(push,1)
	class ThisThunk
	{
	private:
		unsigned __int8 mov_eax_imm32;
		unsigned __int32 save_ebx;
		unsigned __int16 mov_reax_ebx;
		unsigned __int8 pop_ebx;
		unsigned __int8 push_imm32;
		unsigned __int32 m_this;
		unsigned __int8 m_call_rel32;
		unsigned __int32 m_rel_proc;
		unsigned __int8 m_pop_eax;
		unsigned __int8 m_push_ebx;
		unsigned __int8 m_mov_ecx_imm32_2;
		unsigned __int32 m_restore_ebx;
		unsigned __int16 m_mov_ebx_recx;
		unsigned __int8 m_ret;
		unsigned __int32 m_ebx;
	public:
		template <class class_t, class proc_t>
		ThisThunk(class_t *pThis, proc_t proc)
		{
			__int32 procAdr = *(__int32 *) & proc;


			/* first, save ebx to memory,
			effectively: save_ebx = ebx;
			*/
			mov_eax_imm32 = 0xB8;
			save_ebx = (__int32) & m_ebx;
			mov_reax_ebx = 0x1889;
			pop_ebx = 0x5B;
			push_imm32 = 0x68;
			m_this = (__int32)pThis;
			m_call_rel32 = 0xE8;
			m_rel_proc = procAdr - (__int32) & m_pop_eax;
			m_pop_eax = 0x59;
			m_push_ebx = 0x53;
			m_mov_ecx_imm32_2 = 0xB9;
			m_restore_ebx = (__int32) & m_ebx;
			m_mov_ebx_recx = 0x198B;
			m_ret = 0xC3;
		}

		/*
		mov eax, &save_ebx
		mov [eax], ebx
		pop ebx
		push pThis
		call rel32 m_relproc
		pop ecx
		push ebx
		mov ecx, &save_ebx
		mov ebx, [ecx]
		ret

		*/

	};
#pragma pack(pop)
public:

	template <class class_t, class proc_t, class this_proc_t>
	void operator ()(class_t *pThis, proc_t &proc, this_proc_t thisProc)
	{
		ThisThunk *newThunk = new ThisThunk(pThis, thisProc);
		thunks.push_back(newThunk);
		proc = (proc_t)newThunk;
	}

	~ThunkHolder()
	{
		std::vector<ThisThunk *>::iterator itr;
		for (itr = thunks.begin();itr != thunks.end();itr++)
		{
			delete (*itr);
			*itr = 0;
		}
		thunks.clear();
	}

	std::vector<ThisThunk *> thunks;
};

#endif
