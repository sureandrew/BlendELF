
elfListPtr* elfCreateListPtr()
{
	elfListPtr* ptr;

	ptr = (elfListPtr*)malloc(sizeof(elfListPtr));
	memset(ptr, 0x0, sizeof(elfListPtr));

	elfIncObj(ELF_LIST_PTR);

	return ptr;
}

void elfDestroyListPtr(elfListPtr* ptr)
{
	if(ptr->obj) elfDecRef(ptr->obj);

	free(ptr);

	elfDecObj(ELF_LIST_PTR);
}

void elfDestroyListPtrs(elfListPtr* ptr)
{
	if(ptr->next) elfDestroyListPtrs(ptr->next);

	elfDestroyListPtr(ptr);
}

ELF_API elfList* ELF_APIENTRY elfCreateList()
{
	elfList* list;

	list = (elfList*)malloc(sizeof(elfList));
	memset(list, 0x0, sizeof(elfList));
	list->objType = ELF_LIST;
	list->objDestr = elfDestroyList;

	elfIncObj(ELF_LIST);

	return list;
}

void elfDestroyList(void* data)
{
	elfList* list = (elfList*)data;

	if(list->first) elfDestroyListPtrs(list->first);

	free(list);

	elfDecObj(ELF_LIST);
}

ELF_API int ELF_APIENTRY elfGetListLength(elfList* list)
{
	return list->length;
}

ELF_API void ELF_APIENTRY elfInsertListObject(elfList* list, int idx, elfObject* obj)
{
	elfListPtr* ptr;
	elfListPtr* tptr;
	int i;

	if(idx < 0 || idx > list->length) return;

	i = 0;
	ptr = list->first;

	if(!ptr)
	{
		list->first = elfCreateListPtr();
		list->first->obj = obj;
		list->last = list->first;
		list->length = 1;
	}
	else if(idx == 0)
	{
		ptr->prev = elfCreateListPtr();
		ptr->prev->obj = obj;
		ptr->prev->next = ptr;
		list->first = ptr->prev;
		list->length++;
	}
	else if(idx == list->length)
	{
		list->last->next = elfCreateListPtr();
		list->last->next->obj = obj;
		list->last->next->prev = list->last;
		list->last = list->last->next;
		list->length++;
	}
	else
	{
		while(i != idx)
		{
			ptr = ptr->next;
			i++;
		}
		tptr = ptr->prev;
		tptr->next = elfCreateListPtr();
		tptr->next->obj = obj;
		tptr->next->next = ptr;
		tptr->next->prev = tptr;
		ptr->prev = tptr->next;
		list->length++;
	}

	elfIncRef(obj);
}

ELF_API void ELF_APIENTRY elfAppendListObject(elfList* list, elfObject* obj)
{
	if(!obj) return;

	if(!list->first)
	{
		list->first = elfCreateListPtr();
		list->first->obj = obj;
		list->last = list->first;
		list->length = 1;
	}
	else
	{
		list->last->next = elfCreateListPtr();
		list->last->next->obj = obj;
		list->last->next->prev = list->last;
		list->last = list->last->next;
		list->length++;
	}

	elfIncRef(obj);
}

ELF_API elfObject* ELF_APIENTRY elfGetListObject(elfList* list, int idx)
{
	elfListPtr* ptr;
	int i;

	if(idx < 0 || idx > list->length-1) return NULL;

	ptr = list->first;
	i = 0;

	while(ptr)
	{
		if(i == idx)
		{
			return ptr->obj;
		}
		ptr = ptr->next;
		i++;
	}

	return NULL;
}

ELF_API unsigned char ELF_APIENTRY elfRemoveListObject(elfList* list, elfObject* obj)
{
	elfListPtr* ptr;
	elfListPtr* prevPtr;

	prevPtr = NULL;
	ptr = list->first;

	if(list->cur && obj == list->cur->obj)
	{
		prevPtr = list->cur->prev;
		ptr = list->cur;
		list->cur = list->cur->prev;
	}

	while(ptr)
	{
		if(ptr->obj == obj)
		{
			if(prevPtr)
			{
				prevPtr->next = ptr->next;
				if(ptr->next) ptr->next->prev = prevPtr;
			}
			else
			{
				list->first = ptr->next;
				if(list->first) list->first->prev = NULL;
			}
			if(ptr == list->last) list->last = ptr->prev;
			elfDestroyListPtr(ptr);
			list->length--;

			return ELF_TRUE;
		}
		prevPtr = ptr;
		ptr = ptr->next;
	}

	return ELF_FALSE;
}

ELF_API elfObject* ELF_APIENTRY elfBeginList(elfList* list)
{
	list->cur = list->first;
	if(list->cur)
	{
		list->next = list->cur->next;
		return list->cur->obj;
	}
	return NULL;
}

ELF_API elfObject* ELF_APIENTRY elfGetListNext(elfList* list)
{
	list->cur = list->next;
	if(list->cur)
	{
		list->next = list->cur->next;
		return list->cur->obj;
	}
	return NULL;
}

ELF_API elfObject* ELF_APIENTRY elfRBeginList(elfList* list)
{
	list->cur = list->last;
	if(list->cur)
	{
		list->next = list->cur->prev;
		return list->cur->obj;
	}
	return NULL;
}

ELF_API elfObject* ELF_APIENTRY elfGetListRNext(elfList* list)
{
	list->cur = list->next;
	if(list->cur)
	{
		list->next = list->cur->prev;
		return list->cur->obj;
	}
	return NULL;
}

void elfSetListCurPtr(elfList* list, elfObject* ptr)
{
	if(list->cur)
	{
		elfDecRef((elfObject*)list->cur->obj);
		list->cur->obj = ptr;
		elfIncRef((elfObject*)list->cur->obj);
	}
}

ELF_API void ELF_APIENTRY elfSeekList(elfList* list, elfObject* ptr)
{
	elfObject* obj;

	for(obj = elfBeginList(list); obj;
		obj = elfGetListNext(list))
	{
		if(obj == ptr) return;
	}
}

ELF_API void ELF_APIENTRY elfRSeekList(elfList* list, elfObject* ptr)
{
	elfObject* obj;

	for(obj = elfRBeginList(list); obj;
		obj = elfGetListRNext(list))
	{
		if(obj == ptr) return;
	}
}

