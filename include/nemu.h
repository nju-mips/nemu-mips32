#ifndef __NEMU_H__
#define __NEMU_H__

#include "common.h"
#include "cpu/reg.h"
#include "memory.h"

#ifdef __cplusplus

/* arithmetic support for void * */
class VoidPtr {
  void *ptr;
public:
  template<class T>
  VoidPtr(T *ptr) : ptr(static_cast<void *>(ptr)) { }

  VoidPtr(const VoidPtr &) = default;
  VoidPtr(VoidPtr &&) = default;
  VoidPtr &operator=(const VoidPtr &) = default;
  VoidPtr &operator=(VoidPtr &&) = default;

  VoidPtr operator+(size_t inc) {
	void *nptr = static_cast<void *>(&static_cast<uint8_t *>(this->ptr)[inc]);
	return VoidPtr { nptr };
  }

  VoidPtr &operator++(int inc)
  { return *this = this->operator+(inc); }

  VoidPtr &operator+=(size_t inc)
  { return *this = this->operator+(inc); }

  VoidPtr operator-(int dec) {
	return this->operator+(-dec);
  }

  VoidPtr &operator--(int dec)
  { return *this = this->operator-(dec); }

  VoidPtr &operator-=(size_t dec)
  { return *this = this->operator-(dec); }

  template<class T>
  operator T*()
  { return static_cast<T*>(ptr); }

  bool operator !() const { return ptr != NULL; }
};

#else

typedef void * VoidPtr;

#endif

#endif
