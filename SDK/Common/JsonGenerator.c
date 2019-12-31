/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "JsonGenerator.h"

#define INITIAL_SIZE        4096
#define EXPAND_SIZE         2048

typedef enum {
	STATE_UNKNOWN           = 0,
	STATE_ROOT              = 1,
	STATE_OBJECT            = 2,
	STATE_ARRAY             = 3,
	STATE_FIELD             = 4,
} state_t;

static const char colon_symbol          = ':';
static const char comma_symbol          = ',';
static const char start_object_symbol   = '{';
static const char end_object_symbol     = '}';
static const char start_array_symbol    = '[';
static const char end_array_symbol      = ']';
static const char start_string_quote    = '"';
static const char end_string_quote      = '"';

static int ensure_capacity(JsonGenerator *generator, size_t len)
{
	char *buffer;

	assert(generator);
	assert(generator->buffer);

	if (generator->pos + len + 1 <= generator->capacity)
		return 0;

	buffer = (char *)realloc(generator->buffer, generator->capacity + EXPAND_SIZE);
	if (!buffer)
		return -1;

#ifndef NDEBUG
	// realloc does not guarantee that the additional memory is also zero-filled.
	memset(buffer + generator->capacity, 0, EXPAND_SIZE);
#endif

	generator->capacity = generator->capacity + EXPAND_SIZE;
	generator->buffer = buffer;
	return 0;
}

static inline void push_state(JsonGenerator *generator, state_t state)
{
	assert(generator);
	assert(generator->deep < sizeof(generator->state));

	generator->state[generator->deep++] = (uint8_t)state;
}

static inline state_t pop_state(JsonGenerator *generator)
{
	state_t state;

	assert(generator);
	assert(generator->deep > 0);

	if (generator->deep <= 0)
		return STATE_UNKNOWN;

	state = (state_t)(generator->state[--generator->deep] & 0x7F);
	return state;
}

static inline state_t get_state(JsonGenerator *generator)
{
	uint8_t state;

	assert(generator);
	assert(generator->deep > 0);

	if (generator->deep <= 0)
		return STATE_UNKNOWN;

	state = (state_t)(generator->state[generator->deep - 1] & 0x7F);
	return state;
}

static inline void set_state_sticky(JsonGenerator *generator)
{
	assert(generator);
	assert(generator->deep > 0);

	generator->state[generator->deep - 1] |= 0x80;
}

static inline int is_state_sticky(JsonGenerator *generator)
{
	assert(generator);
	assert(generator->deep > 0);

	if (generator->deep <= 0)
		return STATE_UNKNOWN;

	return (generator->state[generator->deep - 1] & 0x80) == 0x80;
}

JsonGenerator *JsonGenerator_Initialize(JsonGenerator *generator)
{
	if (!generator)
		return NULL;

	generator->buffer = (char *)malloc(INITIAL_SIZE);
	if (!generator->buffer)
		return NULL;

#ifndef NDEBUG
	// zero-filled just for debug.
	memset(generator->buffer, 0, INITIAL_SIZE);
	memset(generator->state, 0, sizeof(generator->state));
#endif

	generator->capacity = INITIAL_SIZE;
	generator->pos = 0;
	generator->deep = 0;
	generator->buffer[0] = 0;

	push_state(generator, STATE_ROOT);
	return generator;
}

int JsonGenerator_WriteStartObject(JsonGenerator *generator)
{
	int is_sticky;

	if (!generator || !generator->buffer)
		return -1;

	assert(get_state(generator) == STATE_ROOT
	       || get_state(generator) == STATE_ARRAY
	       || get_state(generator) == STATE_FIELD);

	is_sticky = is_state_sticky(generator);

	if (ensure_capacity(generator, 1 + is_sticky) == -1)
		return -1;

	if (is_sticky)
		generator->buffer[generator->pos++] = comma_symbol;

	generator->buffer[generator->pos++] = start_object_symbol;
	set_state_sticky(generator);
	push_state(generator, STATE_OBJECT);

	return 0;
}

int JsonGenerator_WriteEndObject(JsonGenerator *generator)
{
	if (!generator || !generator->buffer || ensure_capacity(generator, 1) == -1)
		return -1;

	assert(get_state(generator) == STATE_OBJECT);

	generator->buffer[generator->pos++] = end_object_symbol;
	pop_state(generator);
	if (get_state(generator) == STATE_FIELD)
		pop_state(generator); /* pop field state */

	return 0;
}

int JsonGenerator_WriteStartArray(JsonGenerator *generator)
{
	if (!generator || !generator->buffer || ensure_capacity(generator, 1) == -1)
		return -1;

	assert(get_state(generator) == STATE_FIELD);

	generator->buffer[generator->pos++] = start_array_symbol;
	push_state(generator, STATE_ARRAY);

	return 0;
}

int JsonGenerator_WriteEndArray(JsonGenerator *generator)
{
	if (!generator || !generator->buffer || ensure_capacity(generator, 1) == -1)
		return -1;

	assert(get_state(generator) == STATE_ARRAY);

	generator->buffer[generator->pos++] = end_array_symbol;
	pop_state(generator);
	if (get_state(generator) == STATE_FIELD)
		pop_state(generator); /* pop field state */

	return 0;
}

int JsonGenerator_WriteFieldName(JsonGenerator *generator, const char *name)
{
	size_t len;
	int is_sticky;

	if (!generator || !generator->buffer || !name || !*name)
		return -1;

	assert(get_state(generator) == STATE_OBJECT);

	is_sticky = is_state_sticky(generator);

	len = strlen(name);
	if (ensure_capacity(generator, len + 3 + is_sticky) == -1)
		return -1;

	if (is_sticky)
		generator->buffer[generator->pos++] = comma_symbol;

	generator->buffer[generator->pos++] = start_string_quote;
	strcpy(generator->buffer + generator->pos, name);
	generator->pos += len;
	generator->buffer[generator->pos++] = end_string_quote;
	generator->buffer[generator->pos++] = colon_symbol;

	set_state_sticky(generator);
	push_state(generator, STATE_FIELD);

	return 0;
}

int JsonGenerator_WriteString(JsonGenerator *generator, const char *value)
{
	size_t len;
	int is_sticky;

	if (!generator || !generator->buffer)
		return -1;

	assert(get_state(generator) == STATE_FIELD
	       || get_state(generator) == STATE_ARRAY);

	is_sticky = is_state_sticky(generator);

	len = value ? strlen(value) : 4; // null value
	if (ensure_capacity(generator, len + 2 + is_sticky) == -1)
		return -1;

	if (is_sticky)
		generator->buffer[generator->pos++] = comma_symbol;

	if (value) {
		generator->buffer[generator->pos++] = start_string_quote;
		strcpy(generator->buffer + generator->pos, value);
		generator->pos += len;
		generator->buffer[generator->pos++] = end_string_quote;
	} else {
		strcpy(generator->buffer + generator->pos, "null");
		generator->pos += 4;
	}

	if (get_state(generator) == STATE_FIELD)
		pop_state(generator);
	else
		set_state_sticky(generator);

	return 0;
}

int JsonGenerator_WriteStringField(JsonGenerator *generator,
                                   const char *name, const char *value)
{
	int rc;

	rc = JsonGenerator_WriteFieldName(generator, name);
	if (rc < 0)
		return rc;

	return JsonGenerator_WriteString(generator, value);
}

const char *JsonGenerator_Finish(JsonGenerator *generator)
{
	const char *buffer;

	if (!generator || !generator->buffer || get_state(generator) != STATE_ROOT)
		return NULL;

	if (generator->buffer[generator->pos] != 0)
		generator->buffer[generator->pos] = 0;

	buffer = (const char *)generator->buffer;
	generator->buffer = NULL;

	return buffer;
}