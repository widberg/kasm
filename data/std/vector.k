struct vector
{
	T : type;
	data : T*;
	size : u32;
	capacity : u32;
};

vector(T : type, capacity : u32 = 50) : struct vector*
{
	vec := alloc(struct vector, 1);
	vec->type = T;
	vec->size = 0;
	vec->capacity = capacity;
	vec->data = alloc(T, capacity);
	return vec;
}

xvector(vec : struct vector*) : void
{
	free(vec->data);
	free(vec);
}

vector_is_empty(vec : struct vector*) : u8
{
	return !vec->size;
}

vector_push_back(vec : struct vector*, elem : vec->T) : u8
{
	if (vec->size >= vec->capacity)
	{
		vector_reserve(vec, vec->capacity * 2);
	}

	vec->data[vec->size] = elem;
	vec->size = vec->size + 1;

	return 1;
}

vector_pop_back(vec : struct vector*) : u8
{
	if (vector_is_empty(vec)) return 0;

	vec->size = vec->size - 1;

	return 1;
}

vector_at(vec : struct vector*, i : u32) : vec->T*
{
	if (i >= vec->size) return nullptr;
	return vec->data + i;
}

vector_reserve(vec : struct vector*, capacity : u32) : u8
{
	if (capacity > vec->capacity)
	{
		vec->capacity = capacity;
		tmp := vec->data;
		vec->data = alloc(vec->T, vec->capacity);
		memcpy(tmp, vec->data, vec->size * sizeof(vec->T));
		free(tmp);
	}

	return true;
}

vector_resize(vec : struct vector*, size : u32) : u8
{
	if (size > vec->capacity)
	{
		vector_reserve(vec, size);
	}

	vec->size = size;

	return true;
}

vector_shrink_to_fit(vec : struct vector*) : u8
{
	if (vec->size < vec->capacity)
	{
		vec->capacity = size;
		tmp := vec->data;
		vec->data = alloc(vec->T, vec->capacity);
		memcpy(tmp, vec->data, vec->size * sizeof(vec->T));
		free(tmp);
	}

	return true;
}

vector_clear(vec : struct vector*) : u8
{
	vec->size = 0;

	return true;
}

vector_copy(src : struct vector*) : struct vector*
{
	vec := alloc(struct vector, 1);
	vec->type = src->T;
	vec->size = src->size;
	vec->capacity = src->capacity;
	vec->data = alloc(T, capacity);
	memcpy(src->data, vec->data, src->size * sizeof(src->T));
	return vec;
}

vector_get_size(vec : struct vector*) : u32
{
	return vec->size;
}

vector_get_capacity(vec : struct vector*) : u32
{
	return vec->capacity;
}

vector_get_data(vec : struct vector*) : vec->T*
{
	return vec->data;
}
