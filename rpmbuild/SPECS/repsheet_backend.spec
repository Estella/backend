Summary: The backend for Repsheet
Name: repsheet_backend
Version: 0.3
Release: 1
License: ASL 2.0
Group: System Environment/Daemons
URL: https://github.com/repsheet/backend
Source0: http://getrepsheet.com/releases/backend/%{name}-%{version}.zip
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: hiredis
BuildRequires: pcre-devel hiredis-devel gcc

%description 
This tool performs the heavy lifting for scoring and auto
blacklisting offenders on the Repsheet

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
install -D -m755 repsheet $RPM_BUILD_ROOT/%{_bindir}/repsheet

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc LICENSE
%{_bindir}/repsheet

%changelog
* Thu Jun 27 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.3-1
- Initial build.
