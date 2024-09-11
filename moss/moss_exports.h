#pragma once

#ifndef MOSS_EXPORTS_H_
#define MOSS_EXPORTS_H_


#ifdef _WIN32
#ifdef MOSS_EXPORTS
#define MOSS_EXPORT __declspec(dllexport)
#else
#define MOSS_EXPORT __declspec(dllimport)
#endif
#else
#define MOSS_EXPORT
#endif

#endif // MOSS_EXPORTS_H_

