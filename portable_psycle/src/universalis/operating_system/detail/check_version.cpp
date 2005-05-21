// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\implementation universalis::operating_system::detail::check_version
#include <universalis/detail/project.private.hpp>
#include "check_version.hpp"
#include "../loggers.hpp"
#include "../exceptions/code_description.hpp"
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
namespace universalis
{
	namespace operating_system
	{
		namespace detail
		{
			void check_version() throw(exception)
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					class msdos {};
					try
					{
						bool static once(false);

						// extended version info

						if(!once)
						{
							once = true;
							// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/osversioninfoex_str.asp
							{
								::OSVERSIONINFOEX version_info = OSVERSIONINFOEX(); // default-initialization to ensure that if not all fields are set by the OS, we have them set to default values (zeros).
								version_info.dwOSVersionInfoSize = sizeof version_info;
								if(!::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info))) // yet another hard cast needed by the poorly designed winapi
								{
									// ignore the error, extended version information is only available since nt4.0sp6
									once = false;
								}
								else
								{
									if(loggers::information()())
									{
										std::ostringstream s;
										s
											<< "operating system: "
											<< "platform: " << (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT ? "ibm/ms nt" : "msdos") << ", "
											<< "version: "
											<< version_info.dwMajorVersion << '.'
											<< version_info.dwMinorVersion << '.'
											<< version_info.dwBuildNumber << '.'
											<< version_info.wServicePackMajor << '.'
											<< version_info.wServicePackMinor << '.'
											<< version_info.wSuiteMask << '.'
											<< version_info.wProductType << ' '
											<< version_info.szCSDVersion;
										loggers::information()(s.str());
									}
									if
									(
										version_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
										version_info.dwPlatformId == VER_PLATFORM_WIN32s
									)
									{
										throw msdos();
									}
								}
							}
						}

						// crippled version info

						if(!once)
						{
							once = true;
							// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/osversioninfo_str.asp
							{
								::OSVERSIONINFO version_info;
								version_info.dwOSVersionInfoSize = sizeof version_info;
								if(!::GetVersionEx(&version_info))
								{
									std::ostringstream s;
									s << "Your operating system did not want to report its version. Aborting.";
									loggers::exception()("error: " + s.str());
									::MessageBox(0, s.str().c_str(), "Operating system version error", MB_ICONERROR);
									throw exception(exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
								}
								if(loggers::information()())
								{
									std::ostringstream s;
									s
										<< "operating system: "
										<< "platform: " << (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT ? "ibm/ms nt" : "msdos") << ", "
										<< "version: "
										<< version_info.dwMajorVersion << '.'
										<< version_info.dwMinorVersion << '.'
										<< version_info.dwBuildNumber << ' '
										<< version_info.szCSDVersion;
									loggers::information()(s.str());
								}
								if
								(
									version_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
									version_info.dwPlatformId == VER_PLATFORM_WIN32s
								)
								{
									throw msdos();
								}
							}
						}
					}
					catch(msdos)
					{
						std::ostringstream s;
						s << "Your operating system reported itself as being a derivation of MSDOS. This program will only run on derivations of NT. Aborting.";
						loggers::exception()("error: " + s.str());
						::MessageBox(0, s.str().c_str(), "Operating system version error: MSDOS", MB_ICONERROR);
						throw exception(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
			}
		}
	}
}
