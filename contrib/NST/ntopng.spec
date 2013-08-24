#
# Spec file: ntopng

#
# Tags: Data Definitions...
Summary: A next generation network packet traffic probe and collector providing visual network usage and statistics.
Name: ntopng
Version: 1.0.1
Release: 6695.7.nst18
License: GPLv3
Group: Applications/Internet
URL: http://www.ntop.org/
Packager: http://www.networksecuritytoolkit.org/
Vendor: NST Project

Source0: ntopng-1.0.1.tar.gz
Source1: nDPI-1.0.1.tar.gz
Source2: ntopng
Source3: ntopng.conf
Source4: ntopng.service

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: GeoIP, libgcc, nst-systemd-presets, redis, zlib
%{systemd_requires}

BuildRequires: gcc, gcc-c++, zlib-devel
BuildArch: x86_64

%description
ntopng is the next generation version of the original ntop.
It is a network packet traffic probe and collector that renders
network usage graphically, similar to what the popular top Unix
command does. It is based on libpcap and it has been written in a
portable way in order to virtually run on every Unix platform and on
Windows as well.

A web browser used to navigate through ntopng's rendered web pages
for viewing current traffic information and/or to get a data dump
of the collected network network status and statistics. In the
latter case, ntopng can be seen as a simple RMON-like agent with an
embedded web interface.

These ntopng features:

    * An intuitive web interface sporting numerous visual and graphs
    * Limited configuration and administration via the web interface
    * Reduced CPU and memory usage (this varies according to network
      size and traffic)
    * Collection of a large number of hosts and network statistic values

make ntopng easy to use and suitable for monitoring enterprise network
environments.

%prep
%setup -q -n %{name}-%{version}

%build

#
# Extract the nDPI source prior to build...
%{__tar} -xzf "%{SOURCE1}";

%{configure}

%install
%{__make};

#
# Install ntopng...
%{__install} -D --mode 755 "%{name}" "%{buildroot}%{_bindir}/%{name}";

#
# Install supporting files...
%{__mkdir_p} "%{buildroot}%{_datadir}/%{name}";
for f in "httpdocs" "scripts"; do
  %{__cp} -a "${f}" "%{buildroot}%{_datadir}/%{name}";
done

#
# Assume running ntopng for directory: "/usr/share/ntopng" then
# setup GeoIP database paths...
%{__mkdir_p} "%{buildroot}%{_datadir}/%{name}/httpdocs/geoip";
%{__ln_s} "%{_datadir}/GeoIP/GeoIPASNum.dat" "%{buildroot}%{_datadir}/%{name}/httpdocs/geoip";
%{__ln_s} "%{_datadir}/GeoIP/GeoIPASNumv6.dat" "%{buildroot}%{_datadir}/%{name}/httpdocs/geoip";
%{__ln_s} "%{_datadir}/GeoIP/GeoLiteCity.dat" "%{buildroot}%{_datadir}/%{name}/httpdocs/geoip";
%{__ln_s} "%{_datadir}/GeoIP/GeoLiteCityv6.dat" "%{buildroot}%{_datadir}/%{name}/httpdocs/geoip";

#
# Install the ntopng man page...
%{__install} -D --mode 644 "%{name}.8" "%{buildroot}%{_mandir}/man8/%{name}.8";

#
# Install an ntopng systemd environmental configuration file...
%{__install} -D --mode 644 %{SOURCE2} \
  "%{buildroot}%{_sysconfdir}/sysconfig/%{name}";

#
# Install a default ntopng configuration file...
%{__install} -D --mode 644 %{SOURCE3} \
  "%{buildroot}%{_sysconfdir}/%{name}/%{name}.conf";

#
# Install the ntop systemd service control file...
%{__install} -D --mode 644 %{SOURCE4} \
  "%{buildroot}%{_unitdir}/%{name}.service";

#
# Install a default ntopng working directory...
%{__mkdir_p} "%{buildroot}%{_localstatedir}/nst/%{name}";

%clean
%{__rm} -rf "%{buildroot}";

%pre

%post
#
# Add an 'ntopng' service and set its boot state disabled...
%systemd_post %{name}.service

%preun
#
# Stop a running the 'ntopng' service and remove start/stop links...
%systemd_preun %{name}.service

%postun
#
# ntopng package upgrade:
#
# If 'ntopng' was running, stop it and restart it with the upgraded version...
%systemd_postun_with_restart %{name}.service

%files
%defattr(-,root,root,-)
%doc COPYING README*
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_unitdir}/%{name}.service
%{_mandir}/man8/%{name}.8.gz

%config(noreplace) %{_sysconfdir}/%{name}/%{name}.conf
%config(noreplace) %{_sysconfdir}/sysconfig/%{name}

%defattr(-,nobody,nobody,-)
%dir %{_localstatedir}/nst/%{name}


%changelog
* Thu Aug 22 2013 Ronald W. Henderson <rwhalb@nycap.rr.com>
- Next development release: SVN: 6695

* Tue Aug 20 2013 Ronald W. Henderson <rwhalb@nycap.rr.com>
- Created initial version of template spec file.
