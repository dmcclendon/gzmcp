/*
 *  Guitar-ZyX(tm)::MasterControlProgram - portable guitar F/X controller
 *  Copyright (C) 2009  Douglas McClendon
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NDS_LOADER_ARM9_H
#define NDS_LOADER_ARM9_H


#ifdef __cplusplus
extern "C" {
#endif

#define LOAD_DEFAULT_NDS 0

int runNds(const void* loader, 
	   u32 loaderSize, 
	   u32 cluster, 
	   bool initDisc, 
	   bool dldiPatchNds, 
	   int argc, 
	   const char** argv);

int execz(const char* filename, int argc, const char** argv);

#ifdef __cplusplus
}
#endif

#endif // NDS_LOADER_ARM7_H
