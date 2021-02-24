#ifndef _ASSERT_H
#define _ASSERT_H

#define assert(exp) if (!(exp)) panic("assertion '" #exp "'failed");

#endif