Summary: The backend for Repsheet
Name: repsheet_backend
Version: 1.4.0
Release: 1
License: ASL 2.0
Group: System Environment/Daemons
URL: https://github.com/repsheet/backend
Source0: http://getrepsheet.com/releases/backend/%{name}-%{version}.zip
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: hiredis
BuildRequires: pcre-devel hiredis-devel gcc libcurl-devel json-c-devel

%description 
This tool performs the heavy lifting for scoring and auto
blacklisting offenders on the Repsheet

%prep
%setup -q

%build
./autogen.sh
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
install -D -m755 src/repsheet $RPM_BUILD_ROOT/%{_bindir}/repsheet

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc LICENSE
%{_bindir}/repsheet

%changelog
* Fri Nov 08 2013 Aaron Bedra <aaron@aaronbedra.com> - 1.4.0-1
- Adds historical blacklisting.
* Mon Oct 28 2013 Aaron Bedra <aaron.bedra@braintreepayments.com> - 1.3.0-1
- Fixes OFDP Blacklisting. Adds long CLI options.
* Fri Oct 18 2013 Aaron Bedra <aaron.bedra@braintreepayments.com> - 1.2.0-1
- Moves OFDP to JSON API. Minor fixes.
* Wed Oct 09 2013 Aaron Bedra <aaron.bedra@braintreepayments.com> - 1.1.0-1
- Adds OFDP integration
* Tue Aug 22 2013 Aaron Bedra <aaron.bedra@braintreepayments.com> - 1.0.0-1
- Move to SEMVER. Blacklisting now matches current TTL
* Mon Aug 12 2013 Aaron Bedra <aaron.bedra@braintreepayments.com> - 0.12-1
- Introduces Cloudflare integration
* Sat Jul 20 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.11-1
- Support for new key management strategy
* Fri Jul 12 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.10-1
- Fixes for segfaults under heavy Redis load
* Sat Jul 06 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.9-1
- Do not score whitelisted actors
* Fri Jul 05 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.8-1
- Memory allocation fixes
* Thu Jun 27 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.3-1
- Initial build.