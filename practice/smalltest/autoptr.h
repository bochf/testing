/**
 * Auto pointer
 */

#ifndef __AUTOPTR_H__
#define __AUTOPTR_H__

#include <cstdlib>

template<class T>
class AutoPtr
{
	public:
		explicit AutoPtr(T* p = NULL) : m_ptr(p) {};
		virtual ~AutoPtr()
		{
			if (m_ptr)
				delete m_ptr;
			m_ptr = NULL;
		};

		T* get() const
		{
			return m_ptr;
		}

		void set(T* p)
		{
			if (m_ptr == p)
				return;

			if (m_ptr)
				delete m_ptr;

			m_ptr = p;
		};

		T* release()
		{
			T * old_ptr = m_ptr;
			m_ptr = NULL;
			return old_ptr;
		};

		// operators
		T& operator*() const
		{
			return *m_ptr;
		}

		T* operator->() const
		{
			return m_ptr;
		}

		AutoPtr<T>& operator=(AutoPtr<T>& right)
		{
			if (this != &right)
			{
				set(right.release());
			}

			return *this;
		}

	private:
		T * m_ptr;
};

#endif
