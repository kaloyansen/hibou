%global debug_package %{nil}

Name:           hibou
Version:        1.0
Release:        1%{?dist}
Summary:        a compact system resources monitoring tool using ncurses

License:        GPLv3
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, ncurses-devel
Requires:       ncurses

%description
hibou is a compact system resources monitoring tool using ncurses

%prep
%setup -q

%build
gcc src/hibou.c -o bin/hibou -lncurses

%install
mkdir -p %{buildroot}/usr/bin
cp bin/hibou %{buildroot}/usr/bin/hibou

%files
/usr/bin/hibou

%changelog
* Fri Feb 28 2025 Your Name <kaloyansen@gmail.com> - 1.0-1
- Initial RPM release
