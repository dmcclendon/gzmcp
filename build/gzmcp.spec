Name:		gzmcp
Version:	0.2.2010_01_16
#Release:	1%{?dist}
Release:	1.zyx
Summary:	Wireless remote control for guitar effects system

# TODO: pick a better group
Group:		System Environment/Base
License:	GPLv3
URL:		http://viros.org/GuitarZyX
Source0:	%{name}-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	gcc, make
Requires:	perl, bash, squashfs-tools, httpd, pykickstart, vnc, vnc-server, qemu, wget

%description
Client-server system for wirelessly controlling the Rakarrack real-time guitar effects
system.

%prep
%setup -q


%build
#%NOTconfigure
#make %NOT{?_smp_mflags}
make


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr
make install PREFIX=${RPM_BUILD_ROOT}/usr


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc
/usr/bin
/usr/lib/gzmcp
/usr/share


%changelog
* Sat Jan 16 2009 Douglas McClendon <dmc@viros.org> - 0.2.2010_01_16
- update to sync with rakarrack-0.4.0.dmc (presets/midi map)

* Sun Nov 22 2009 Douglas McClendon <dmc@viros.org> - 0.2.2009_11_22
- client: doubled and undoubled font modes for menu, better filebrowsing
- client: aesthetic menu enhancements, alphasort, headers, ...
- client: flicker free fading goodness
- client: ssid scanning and entry now supported via main menu
- common: bugfix: fixed possible protocol data size mismatch

* Thu Nov 12 2009 Douglas McClendon <dmc@viros.org> - 0.2.2009_11_12
- client: main menu system instead of proof of concept dedicated mode
- client: filesytem browse with execz() thanks to external loader code 
  - using Chishm's GPLv2+ loader for now, maybe later akloader under <=LGPL
- common: client can now pull and execute updated code/firmware from server 
  - or any file, via new update request command packet type over wifi
- bugfix: server: broadcast presence on all interfaces 
  - 255.255.255.255 on linux apparently only goes to default interface
    - makes it easy to just dedicate a wifi dongle and AP to the task
      - remember to use iptables and opt-in port 24642
- client: better pc debug client, or at least supports new command type

* Sat Sep 05 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090905
- server: major rewrite, udp broadcast discovery
- client: write newline when writing user inputted ssid config
- client: new protocol, trying with 60 midi-syncs/minute, vs 30 b4
- client: now groks 'wfc' as a special ssid
- client: abstracted intermode reinit
- client: better backgrounds
- no more sprintf
- new memstats 
- client: compile time extra debug logging flag
- client: better fading, use of constants to tune timing of fades

* Fri Jul 31 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090731
- tools: g_ds_menu.dat vs _ds_menu.dat
- debug: new logfile mechanism
- note: use dswifi-0.3.9 for now

* Tue Jul 28 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090728
- client: no chmod, so change the way defaults file is written
- client: makefile: dlditool is part of current dkp

* Mon Jul 27 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090727
- client: code refactored into several modes of a state machine
- client: new GUI method for user to input ssid
- probably other bugfixes, cleanups, and refactorings

* Sat Mar 21 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090321
- server: handshake string comparison bug, don't know how it worked well enough before.
- client: splash screens, more eyecandy and aesthetic tweaks generally
- client: SuperCowbellShingleMode

* Sun Mar 09 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090309
- second rpm-ified release

* Fri Feb 27 2009 Douglas McClendon <dmc@viros.org> - 0.1.20090227
- initial rpm-ified release

