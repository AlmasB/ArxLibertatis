/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_AUDIO_AUDIORESOURCE_H
#define ARX_AUDIO_AUDIORESOURCE_H

#include "AudioTypes.h"
#include <stdlib.h>

#include "platform/Platform.h"

struct PakFileHandle;

namespace audio {
	
	const size_t ALIGNMENT = 16;
	
	PakFileHandle * OpenResource(const std::string & name, const std::string & resource_path);
	
	class ResourceHandle {
		
	public:
		
		ResourceHandle() {
			__count = 0;
		}
		virtual ~ResourceHandle() {};
		
		inline void Catch() {
			++__count;
		}
		inline void Release() {
			arx_assert(__count > 0);
			--__count;
		}
		inline s32 IsHandled() {
			return __count;
		}
		
	private:
		
		s32 __count;
		
	};
	
	template <class T>
	class ResourceList {
		
	public:
		
		typedef T * const * iterator;
		
		ResourceList();
		~ResourceList();
		
		inline bool IsValid(s32 index);
		inline T * operator[](s32 index);
		inline size_t Size() { return size; }
		inline s32 Add(T * element);
		inline void Delete(s32 index);
		inline void Clean();
		
		inline iterator begin() { return list; }
		inline iterator end() { return list + size; }
		inline iterator remove(iterator i) { Delete(i - begin()); return i + 1; }
		
	private:
		
		size_t size;
		T ** list;
		
	};
	
	template <class T>
	inline ResourceList<T>::ResourceList() : size(0), list(NULL) { }
	
	template <class T>
	inline ResourceList<T>::~ResourceList() {
		for (size_t i(0); i < size; i++) if (list[i]) delete list[i];
	}
	
	template <class T>
	inline bool ResourceList<T>::IsValid(s32 index) {
		return ((size_t)index < size && list[index]) ? true : false;
	}
	
	template <class T>
	inline T * ResourceList<T>::operator[](s32 index) {
		return list[index];
	}
	
	template <class T>
	inline s32 ResourceList<T>::Add(T * element) {
		
		void * ptr;
		size_t i(0);
		
		for(; i < size; i++) {
			if(!list[i]) {
				list[i] = element;
				return i;
			}
		}
		
		ptr = realloc(list, (size + ALIGNMENT) * sizeof(T *));
		if(!ptr) {
			return INVALID_ID;
		}
		
		list = (T **)ptr, size += ALIGNMENT;
		
		memset(&list[i], 0, ALIGNMENT * sizeof(T *));
		list[i] = element;
		
		return i;
	}
	
	template <class T>
	inline void ResourceList<T>::Delete(s32 index) {
		
		if(size_t(index) >= size  || !list[index]) return;
		
		delete list[index];
		list[index] = NULL;
		
		if(size <= ALIGNMENT) return;
		
		for(size_t j(size - ALIGNMENT); j < size; j++) {
			if(list[j])
				return;
		}
		
		list = (T **)realloc(list, (size -= ALIGNMENT) * sizeof(T *));
	}
	
	template <class T>
	inline void ResourceList<T>::Clean() {
		
		for(size_t i(0); i < size; i++) {
			if(list[i]) {
				delete list[i];
			}
		}
			
		free(list), list = NULL, size = 0;
	}
	
} // namespace audio

#endif // ARX_AUDIO_AUDIORESOURCE_H
