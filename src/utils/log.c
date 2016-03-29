/*
 * Copyright © 2016 S-Core Corporation
 * Copyright © 2016-2017 Samsung Electronics co., Ltd. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "utils.h"
#include <stdarg.h>
#include <stdio.h>

#define VK_LOG_FILE	stdout

void
vk_log(const char *domain, const char *file, int line, const char *format, ...)
{
	va_list args;

	fprintf(VK_LOG_FILE, "[VK] ");

	if (domain)
		fprintf(VK_LOG_FILE, "%s: ", domain);

	va_start(args, format);
	vfprintf(VK_LOG_FILE, format, args);
	va_end(args);

	fprintf(VK_LOG_FILE, " %s:%d\n", file, line);
}
