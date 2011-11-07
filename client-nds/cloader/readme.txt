#############################################################################
#

This is Chishm's Loader, effectively providing the exec() function, which he
generously made available under GPLv2+.[1]

This edition has been lightly modified, primarily whitespace formatting,
and is licensed under GPLv3, as is the rest of MCP code here.

[1] 
http://forum.gbadev.org/viewtopic.php?t=12906&postdays=0&postorder=asc&start=45
chishm
Joined: 09 Jan 2005
Posts: 1391
Location: Canberra, Australia
PostPosted: Sun Sep 16, 2007 3:07 am    Post subject:	Reply with quote
The loader binary is distributed under the GNU GPL v2, so if you change that
in any way then you'll need to supply source code on request.
Although I forgot to explicitly license it in the source code (meaning it
automatically has full copyright to me), consider the loader stub
(nds_loader_*) to be distributed under 3-clause BSD. All that is required is
credit in the documentation.
This is all assuming that you load the binary from disc and don't statically
link it into your main application, at which point your application would have
to be licensed under GPL v2 or above too.
http://chishm.drunkencoders.com
http://dldi.drunkencoders.com

#
#############################################################################
