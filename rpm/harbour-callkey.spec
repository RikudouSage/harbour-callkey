Name:       harbour-callkey

Summary:    CallKey
Version:    1.1.2
Release:    1
License:    MIT
URL:        https://github.com/RikudouSage/harbour-callkey
Source0:    %{name}-%{version}.tar.bz2

%{!?harbour_store:%define harbour_store %(if [ -n "$HARBOUR_STORE" ]; then echo 1; elif echo "$PWD" | grep -q -- '-Store'; then echo 1; else echo 0; fi)}

%global __provides_exclude_from ^%{_datadir}/%{name}/lib/.*$
%global __requires_exclude_from ^%{_datadir}/%{name}/lib/.*$
%global __requires_exclude ^libvoipringer\\.so$|^libvoipringer\\.so\\(\\)\\(64bit\\)$

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   sailfishsecretsdaemon-secretsplugins-default
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishsecrets)
BuildRequires:  pkgconfig(sailfishcrypto)
BuildRequires:  desktop-file-utils

%description
Trigger gates, doors and other devices with a quick VoIP call.


%prep
%setup -q -n %{name}-%{version}

%build

%if 0%{?harbour_store}
%qmake5 CONFIG+=harbour_store
%else
%qmake5
%endif

%make_build


%install
%qmake5_install

%if 0%{?harbour_store}
sed -i '/^ExecDBus=/d' %{buildroot}%{_datadir}/applications/%{name}.desktop
%endif

desktop-file-install --delete-original         --dir %{buildroot}%{_datadir}/applications                %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
